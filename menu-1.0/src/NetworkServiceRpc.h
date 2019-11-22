/** ************************************************************************************
 *
 *          @file  NetworkServiceRpc.h
 *
 *    	   @brief  
 *
 *       @version  1.0
 *          @date  20.12.2013 21:55:02
 *
 *        @author  Sudheer Divakaran 
 *
 *		 @details
 *
 *\if NODOC
 *       Revision History:
 *				20.12.2013 - Sudheer Divakaran - Created
 *\endif
 ***************************************************************************************/

#ifndef __NETWORK_SERVICE_RPC_H__
#define __NETWORK_SERVICE_RPC_H__

extern bool InitializeNetworkServiceRpc();
extern bool NotifySettingsToNetworkService(std::map<std::string, std::string>& refMapSettings);
extern bool UploadToCloud(std::string &refFile);


#endif
