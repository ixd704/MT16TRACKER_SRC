
#include <stdlib.h>

#include <iostream>
#include <string>
#include <string.h>
#include <syslog.h>

#ifdef __cplusplus
#define PROTOBUF_C_BEGIN_DECLS	extern "C" {
#define PROTOBUF_C_END_DECLS	}
#else
#define PROTOBUF_C_BEGIN_DECLS
#define PROTOBUF_C_END_DECLS
#endif
#include <tracker.pb-c.h>

PROTOBUF_C_BEGIN_DECLS
#include "google/protobuf-c/protobuf-c-rpc.h"
PROTOBUF_C_END_DECLS

#include "audio.h"

#include "json/reader.h"
#include "json/value.h"

using namespace std;
#include "config.h"
#include <menuEntry.h>
#include <ToggleMenuEntry.h>
#include <menu.h>
#include <lcd.h>

static string g_str;
static unsigned int g_status;
static Tracker__StatusEnum  g_rpc_result;



ProtobufCService *service_audio = NULL;
ProtobufC_RPC_Client *client_audio = NULL;
ProtobufCDispatch *dispatch_audio = NULL;

static void handle_response_none(const Tracker__ResponseNone *message, void *closure_data)
{
	* (protobuf_c_boolean *) closure_data = 1;
}


static void handle_status_response (const Tracker__Status *result, void *closure_data)
{
	g_rpc_result = TRACKER__STATUS_ENUM__FAILED;
	if (result) {
		g_rpc_result = result->status;
	}
	* (protobuf_c_boolean *) closure_data = 1;
}

static void handle_get_audio_params (const Tracker__String *result, void *closure_data)
{
	g_str.clear();
	if(result) {
		g_str = result->value;
	}
	* (protobuf_c_boolean *) closure_data = 1;
}

static void handle_encoding_status_response (const Tracker__MessageInt *result, void *closure_data)
{
	if (result) {
		g_status = result->value;
	} else {
		g_status = 0;
	}
	* (protobuf_c_boolean *) closure_data = 1;
}

int rpc_init()
{
	int attempt = 0;

	syslog(LOG_INFO, "rpc_init...");

	ProtobufC_RPC_AddressType address_type = PROTOBUF_C_RPC_ADDRESS_LOCAL;
	dispatch_audio = protobuf_c_dispatch_new(&protobuf_c_default_allocator);

	service_audio = protobuf_c_rpc_client_new (address_type, RPC_SOCKET_AUDIO, &tracker__audio__descriptor, dispatch_audio);
	client_audio = (ProtobufC_RPC_Client *) service_audio;

	syslog(LOG_INFO, "rpc_init continue...");

	while (!protobuf_c_rpc_client_is_connected(client_audio)) {
		protobuf_c_dispatch_run(dispatch_audio);
		++attempt;
		if (attempt > 3)
			return -1;
	}

	syslog (LOG_INFO, "rpc_init done!");

	return 0;
}

int rpc_de_init()
{
	protobuf_c_service_destroy(service_audio);
	protobuf_c_dispatch_free(dispatch_audio);
	return 0;
}


void set_audio_params(string str)
{
	syslog(LOG_INFO, "set_audio_params [%s]", str.c_str());

	protobuf_c_boolean is_done = 0;
	Tracker__String _params = TRACKER__STRING__INIT;

	_params.value = (char *)str.c_str();
	tracker__audio__set_params(service_audio, &_params , handle_response_none, &is_done);

	while (!is_done)
		protobuf_c_dispatch_run (dispatch_audio);

	syslog(LOG_INFO, "set_audio_params done!");
}

string get_audio_params()
{
	syslog (LOG_INFO, "get_audio_params");

	Tracker__RequestNone request = TRACKER__REQUEST_NONE__INIT;
	protobuf_c_boolean is_done = 0;
	string result;

	tracker__audio__get_params(service_audio, &request, handle_get_audio_params, &is_done);

	while (!is_done)
		protobuf_c_dispatch_run (dispatch_audio);

	result = g_str;

	syslog (LOG_INFO, "get_audio_params done [%s]!", result.c_str());

	return result;
}

map<string,string> get_channels_map(){
	std::map<std::string,std::string> out;

	Json::Value root;   // will contains the root value after parsing.
	Json::Reader reader;
	string channels = config::GetConfig(KEY_CHANNELS);
	bool parsingSuccessful = reader.parse(channels, root );

	if ( parsingSuccessful ){
		Json::Value chnls = root.get(KEY_CHANNELS, Json::Value(Json::arrayValue));
		for (unsigned i = 0; i < chnls.size(); i++){
			out[chnls[i].get("num",CHANNEL_INVALID).asString()] = "on";
		}
	} else {
		syslog(LOG_ERR,"ERROR: can't parse channels state [%s]", channels.c_str());
	}

	return out;
}





bool start_capture()
{
	syslog (LOG_INFO, "start_capture");

	Tracker__RequestNone request = TRACKER__REQUEST_NONE__INIT;
	protobuf_c_boolean is_done = 0;

	tracker__audio__start_capture(service_audio, &request, handle_status_response, &is_done);

	while (!is_done)
		protobuf_c_dispatch_run (dispatch_audio);

	syslog (LOG_INFO, "start_capture done!");

	return TRACKER__STATUS_ENUM__SUCCESS == g_rpc_result;
}

bool stop_capture()
{
	syslog (LOG_INFO, "stop_capture");

	Tracker__RequestNone request = TRACKER__REQUEST_NONE__INIT;
	protobuf_c_boolean is_done = 0;

	tracker__audio__stop_capture(service_audio, &request, handle_status_response, &is_done);

	while (!is_done)
		protobuf_c_dispatch_run (dispatch_audio);

	syslog (LOG_INFO, "stop_capture done!");

	return TRACKER__STATUS_ENUM__SUCCESS == g_rpc_result;
}

bool toggle_encoding()
{
	syslog (LOG_INFO, "toggle_encoding");

	Tracker__RequestNone request = TRACKER__REQUEST_NONE__INIT;
	protobuf_c_boolean is_done = 0;

	tracker__audio__toggle_encoding (service_audio, &request, handle_status_response, &is_done);

	while (!is_done)
		protobuf_c_dispatch_run (dispatch_audio);

	syslog (LOG_INFO, "toggle_encoding done!");

	return TRACKER__STATUS_ENUM__SUCCESS == g_rpc_result;
}

int get_encoding_status()
{
	syslog (LOG_INFO, "get_encoding_status");

	Tracker__RequestNone request = TRACKER__REQUEST_NONE__INIT;
	protobuf_c_boolean is_done = 0;

	tracker__audio__get_encoding_status (service_audio, &request, handle_encoding_status_response, &is_done);

	while (!is_done)
		protobuf_c_dispatch_run (dispatch_audio);

	syslog (LOG_INFO, "get_encoding_status done!");

	return g_status;
}

void NotifyAudioParameters()
{
	Json::Value root;
	Json::Reader reader;

	std::string channels =  config::GetConfig(KEY_CHANNELS);

	if (reader.parse(channels, root))
	{
		root[KEY_RECORD_MODE] = Json::Value(config::GetConfig(KEY_RECORD_MODE));
		root[KEY_SAMPLING_RATE] = Json::Value(config::GetConfig(KEY_SAMPLING_RATE));
		root[KEY_AUDIO_FOLDER] = Json::Value(config::GetConfig(KEY_AUDIO_FOLDER));
		root[KEY_AUDIO_FORMAT] = Json::Value(config::GetConfig(KEY_AUDIO_FORMAT));

		set_audio_params(root.toCompactString());
	}
	else
	{
		syslog(LOG_ERR,"ERROR: can't parse channels state [%s]", channels.c_str());
	}
}

void OnRecordMenuPrint(menuEntry* pMenu)
{
	lcd::clear();
	if (Menu::IsRecording()) {
		char sBuffer[32];
		float fSamplingRate = atoi(config::GetConfig(KEY_SAMPLING_RATE).c_str());
		fSamplingRate /=1000;
		sprintf(sBuffer,"Rec:%s Chnls:%u",config::GetConfig(KEY_AUDIO_FORMAT).c_str(), config::HighestEnabledChannel);
		lcd::write(sBuffer, strlen(sBuffer),0,0);
		sprintf(sBuffer,"Rate:%.1f KHz",fSamplingRate);
		lcd::write(sBuffer, strlen(sBuffer),1,0);
	} else {
		std::string sMenuText;
		sMenuText = pMenu->getMenuText();
		lcd::write(sMenuText.c_str(), sMenuText.size(),0,0);
	}
}

void OnRecordMenuStateChange(ToggleMenuEntry* pMenu, bool bEnabled)
{
	if (bEnabled) {
		if (!Menu::IsRecording()) {
			if(start_capture()) {
				pMenu->SetModelMenu(true);
				Menu::SetRecordingStatus(true);
				OnRecordMenuPrint(pMenu);
			}
		}
	} else {
		if (Menu::IsRecording()) {
			pMenu->SetModelMenu(false);
			stop_capture();
			Menu::SetRecordingStatus(false);
			lcd::clear();
			lcd::write("Encoding",8,0,0);
			stop_capture();
			toggle_encoding();

			while (get_encoding_status() == 1){
				lcd::clear();
				std::string out = "Encoding";
				for(int i=0; i <4; i++) {
					lcd::write(out.c_str(),out.length(),0,0);
					sleep(1);
					out += ".";
				}
			}
			pMenu->print();
		}

	}
	syslog(LOG_INFO,"Record State Changed:%d", bEnabled);
}

void SetupAudioRecordHandler(ToggleMenuEntry *pMenu)
{
	ToggleEntryHandler &rHandler = pMenu->GetHandler();
	rHandler.OnToggleStateChange = OnRecordMenuStateChange;
	rHandler.OnPrint = OnRecordMenuPrint;
}
