/** ************************************************************************************
 *
 *          @file  timer.h
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
#ifndef __TIMER_H__
#define __TIMER_H__

#include <unistd.h>
#include <pthread.h>

#include <vector>


typedef bool (*TimerCallBack)(void *pArg);

struct TimerClient
{
	TimerCallBack pFunction;
	void * pData;
	time_t nInterval;
	time_t nTimeout;
};

typedef std::vector<TimerClient> TimerClientVector;

class Timer
{
private:
	static TimerClientVector m_Clients;
	static unsigned m_nClientCount;
	static pthread_cond_t m_Condition;
	static pthread_mutex_t m_Mutex;

public:
	static void AddClient(TimerCallBack pFunction, void *pData, time_t nInterval);
	static void RemoveClient(TimerCallBack pFunction);
	static void* ThreadFn(void *pArg);

private:
	static void NotifyClients();
};


#endif
