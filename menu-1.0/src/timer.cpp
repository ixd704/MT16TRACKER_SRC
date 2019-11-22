/** ************************************************************************************
 *
 *          @file  timer.cpp
 *
 *    	   @brief  
 *
 *       @version  1.0
 *          @date  09.01.2014 14:08:24
 *
 *        @author  Sudheer Divakaran 
 *
 *		 @details
 *
 *\if NODOC
 *       Revision History:
 *				09.01.2014 - Sudheer Divakaran - Created
 *\endif
 ***************************************************************************************/

#include <timer.h>

unsigned Timer::m_nClientCount;
TimerClientVector Timer::m_Clients;
pthread_cond_t Timer::m_Condition = PTHREAD_COND_INITIALIZER;
pthread_mutex_t Timer::m_Mutex = PTHREAD_MUTEX_INITIALIZER;

void Timer::AddClient(TimerCallBack pFunction, void *pData, time_t nInterval)
{
	unsigned n;
	bool bAdd = true;
	pthread_mutex_lock(&m_Mutex);
	if (m_nClientCount) {
		for (n=0; n < m_nClientCount; n++) {
			TimerClient &refClient = m_Clients[n];
			if (refClient.pFunction == pFunction) {
				bAdd = false;
				break;
			}
		}
	}
	if (bAdd) {
		if (nInterval) {
			TimerClient oClient;
			oClient.pFunction = pFunction;
			oClient.pData = pData;
			oClient.nInterval = nInterval;
			oClient.nTimeout = time(NULL) + nInterval;
			m_Clients.push_back(oClient);
			m_nClientCount++;
			pthread_cond_signal(&m_Condition);
		} else {

		}
	}
	pthread_mutex_unlock(&m_Mutex);
}

void Timer::RemoveClient(TimerCallBack pFunction)
{
	
	pthread_mutex_lock(&m_Mutex);
	if (m_nClientCount) {
		TimerClientVector::iterator iter;
		for (iter=m_Clients.begin(); iter != m_Clients.end(); iter++) {
			TimerClient &refClient = *iter;
			if (refClient.pFunction == pFunction) {
				m_Clients.erase(iter);
				m_nClientCount--;
				break;
			}
		}
	}
	pthread_mutex_unlock(&m_Mutex);
}

void Timer::NotifyClients()
{
	bool bEnabled;
	pthread_mutex_lock(&m_Mutex);
	if (m_nClientCount) {
		TimerClientVector::iterator iter;
		iter = m_Clients.begin();

		while (iter != m_Clients.end()) {
			TimerClient &refClient = *iter;
			if (time(NULL) >= refClient.nTimeout) {
				bEnabled = refClient.pFunction(refClient.pData);
				if (bEnabled) {
					refClient.nTimeout = time(NULL) + refClient.nInterval;
				} else {
					TimerClientVector::iterator iter2 = iter;
					iter = m_Clients.erase(iter2);
					m_nClientCount--;
					continue;
				}
			}
			iter++;
		}
	}
	pthread_mutex_unlock(&m_Mutex);
}




void* Timer::ThreadFn(void *pArg)
{
	while (1) {
		pthread_mutex_lock(&m_Mutex);
		if (!m_nClientCount) {
			pthread_cond_wait(&m_Condition, &m_Mutex);
		}
		pthread_mutex_unlock(&m_Mutex);
		
		NotifyClients();
		sleep(1);
	}

	return NULL;
}

