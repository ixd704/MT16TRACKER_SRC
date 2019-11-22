/** ************************************************************************************
 *
 *          @file  MenuRpcService.cpp
 *
 *    	   @brief  
 *
 *       @version  1.0
 *          @date  20.12.2013 13:47:32
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
#include <json/reader.h>
#include <json/value.h>

#include <MenuRpcService.h>
#include <config.h>
#include <menu.h>

#define RPC_SOCKET_MENU "/var/run/menu.socket"

static Tracker__Setting SettingsList[MAX_MENU_SETTINGS];
static Tracker__Setting* pSettings[MAX_MENU_SETTINGS];


static void rpc_menu_get_settings(Tracker__Menu_Service *service,
                       const Tracker__RequestNone *input,
                       Tracker__SettingsMap_Closure closure,
                       void *closure_data)
{
	int i=0;
	Tracker__SettingsMap result = TRACKER__SETTINGS_MAP__INIT;	
	std::map<std::string, std::string> SettingsMap = config::GetSettings();
   	
	result.n_settings = 0;
    result.settings = &pSettings[0];

	for(std::map<std::string, std::string>::iterator it = SettingsMap.begin(); it != SettingsMap.end(); it++) {
		tracker__setting__init(&SettingsList[i]);
		SettingsList[i].key = (char*)it->first.c_str();
		SettingsList[i].value = (char*)it->second.c_str();
		result.n_settings++;
		i++;
		if (MAX_MENU_SETTINGS == i) {
			break;
		}
	}

	closure(&result, closure_data);
}

static void rpc_menu_update_settings(Tracker__Menu_Service *service,
                          const Tracker__SettingsMap *input,
                          Tracker__Status_Closure closure,
                          void *closure_data)
{
	if (NULL != input) {
	  	if (NULL != input->settings) {
			if (input->n_settings) {
				for (unsigned i = 0; i< input->n_settings; i++) {
					config::SetConfig(input->settings[i]->key, input->settings[i]->value, false);
				}
			}
		}
	}
	
	closure(NULL, closure_data);
}

static void rpc_menu_network_service_status_changed(Tracker__Menu_Service *service,
                                         const Tracker__NetworkServiceStatus *input,
                                         Tracker__ResponseNone_Closure closure,
                                         void *closure_data)
{
	Menu::OnNetworkServiceStatusChanged(input);
	closure(NULL, closure_data);
}

static void rpc_menu_files_uploaded_to_cloud_notification(Tracker__Menu_Service *service,
                                               const Tracker__StringList *input,
                                               Tracker__ResponseNone_Closure closure,
                                               void *closure_data)
{

	closure(NULL, closure_data);
}


static Tracker__Menu_Service service_menu = TRACKER__MENU__INIT(rpc_menu_);

void* menu_rpc_thread_function(void *pArg)
{
	for (int i=0; i < MAX_MENU_SETTINGS; i++) {
		pSettings[i] = &SettingsList[i];
	}
	ProtobufCDispatch *pMenuRPCDispatch = protobuf_c_dispatch_new(&protobuf_c_default_allocator);

	ProtobufC_RPC_Server *server = protobuf_c_rpc_server_new(PROTOBUF_C_RPC_ADDRESS_LOCAL, RPC_SOCKET_MENU,
									(ProtobufCService *)&service_menu, pMenuRPCDispatch);
	while (1) {
		protobuf_c_dispatch_run(pMenuRPCDispatch);
	}

	return NULL;	
}


