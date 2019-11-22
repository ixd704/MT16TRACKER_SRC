/** ************************************************************************************
 *
 *          @file  NetworkServiceRpc.cpp
 *
 *    	   @brief  
 *
 *       @version  1.0
 *          @date  20.12.2013 21:55:09
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

#ifdef __cplusplus
#define PROTOBUF_C_BEGIN_DECLS	extern "C" {
#define PROTOBUF_C_END_DECLS	}
#else
#define PROTOBUF_C_BEGIN_DECLS
#define PROTOBUF_C_END_DECLS
#endif

PROTOBUF_C_BEGIN_DECLS
#include <google/protobuf-c/protobuf-c-rpc.h>
PROTOBUF_C_END_DECLS
#include <tracker.pb-c.h>

#include <string>
#include <map>

#include <config.h>
#include <NetworkServiceRpc.h>


#define RPC_SOCKET_NETWORK_SERVICE "/var/run/networkservice.socket"

static Tracker__Setting SettingsList[MAX_MENU_SETTINGS];
static Tracker__Setting* pSettings[MAX_MENU_SETTINGS];


static ProtobufCService *pNetworkService;
static ProtobufC_RPC_Client *pNetworkServiceClient;
static ProtobufCDispatch *pNetworkServiceDispatch;

static bool bUpdateSettingsResult;
static bool bUploadToCloudResult;

static bool ConnectToNetworkService()
{
	if (protobuf_c_rpc_client_is_connected(pNetworkServiceClient)) {
		return true;
	} else {
		protobuf_c_dispatch_run(pNetworkServiceDispatch);
		return protobuf_c_rpc_client_is_connected(pNetworkServiceClient);
	}
}


bool InitializeNetworkServiceRpc()
{

	for (int i=0; i < MAX_MENU_SETTINGS; i++) {
		pSettings[i] = &SettingsList[i];
	}
	
	pNetworkServiceDispatch = protobuf_c_dispatch_new(&protobuf_c_default_allocator);
	pNetworkService = protobuf_c_rpc_client_new (PROTOBUF_C_RPC_ADDRESS_LOCAL,
									RPC_SOCKET_NETWORK_SERVICE,
									&tracker__network_service__descriptor,
									pNetworkServiceDispatch);
	pNetworkServiceClient = (ProtobufC_RPC_Client *)pNetworkService;

	return ConnectToNetworkService();
}

static void UpdateSettingsHandler(const Tracker__Status *message, void *closure_data)
{
	bUpdateSettingsResult = false;
	if (message) {
		bUpdateSettingsResult = TRACKER__STATUS_ENUM__SUCCESS==message->status;
	}

	*(protobuf_c_boolean *)closure_data = 1;
}


bool NotifySettingsToNetworkService(std::map<std::string, std::string>& refMapSettings)
{

	protobuf_c_boolean is_done=0;

	Tracker__SettingsMap result = TRACKER__SETTINGS_MAP__INIT;	
   	int i=0;
	result.n_settings = 0;
    result.settings = &pSettings[0];

	for(std::map<std::string, std::string>::iterator it = refMapSettings.begin(); it != refMapSettings.end(); it++) {
		tracker__setting__init(&SettingsList[i]);
		SettingsList[i].key = (char*)it->first.c_str();
		SettingsList[i].value = (char*)it->second.c_str();
		result.n_settings++;
		i++;
		if (MAX_MENU_SETTINGS == i) {
			break;
		}
	}

	if (!ConnectToNetworkService()) {
		return false;
	}

	tracker__network_service__update_settings(pNetworkService, &result,
                                              UpdateSettingsHandler, &is_done);
	while (!is_done) {
		protobuf_c_dispatch_run(pNetworkServiceDispatch);
	}

	return bUpdateSettingsResult;
}

static void UploadToCloudHandler(const Tracker__Status *message, void *closure_data)
{
	bUploadToCloudResult = false;
	if (message) {
		bUploadToCloudResult = TRACKER__STATUS_ENUM__SUCCESS==message->status;
	}

	*(protobuf_c_boolean *)closure_data = 1;
}


bool UploadToCloud(std::string &refFile)
{
	protobuf_c_boolean is_done=0;

	Tracker__StringList FileList = TRACKER__STRING_LIST__INIT;
	char *pFile = (char *)refFile.c_str();
	
	FileList.n_value = 1;
	FileList.value = &pFile;

	if (!ConnectToNetworkService()) {
		return false;
	}

	tracker__network_service__upload_to_cloud(pNetworkService, &FileList,
												UploadToCloudHandler, &is_done);


	while (!is_done) {
		protobuf_c_dispatch_run(pNetworkServiceDispatch);
	}

	return bUploadToCloudResult;
}
