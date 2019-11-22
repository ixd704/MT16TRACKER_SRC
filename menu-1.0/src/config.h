#ifndef CONFIG_H
#define CONFIG_H 1

#include <stdio.h>
#include <string.h>
#include <map>
#include <string>
#include <vector>
#include "json/reader.h"
#include "json/writer.h"
#include "eth.h"
#include "wifi.h"

#include "cmd.h"
#include "audio.h"
#include "pthread.h"


extern const char* KEY_AUDIO_FOLDER;
extern const char* KEY_USB_FOLDER;
extern const char* KEY_CHANNELS;
extern const char* KEY_RECORD_MODE;
extern const char* KEY_SAMPLING_RATE;
extern const char* KEY_AUDIO_FORMAT;
extern const char* KEY_DHCP;
extern const char* KEY_STATIC_DNS;
extern const char* KEY_STATIC_GATEWAY;
extern const char* KEY_STATIC_IP;
extern const char* KEY_STATIC_IP_MASK;
extern const char* KEY_WIFI;
extern const char* KEY_WIFI_PASSWORD;
extern const char* KEY_WIFI_SSID;


extern const char* KEY_CODE_REV;
extern const char* KEY_PIN;
extern const char* KEY_SERVICE_STATUS;
extern const char* KEY_AUDIO_RECORD;
extern const char* KEY_SUPPORT_EMAIL;
extern const char* KEY_SUPPORT_PHONE;



extern const char* KEY_AUDIO_CHANNELS;
extern const char* KEY_IP;
extern const char* KEY_ETH_MAC;
extern const char* KEY_WIFI_MAC;
extern const char* KEY_DNS;
extern const char* KEY_GATEWAY;
extern const char* KEY_MASK;
extern const char* KEY_DHCP_IP;
extern const char* KEY_DHCP_DNS;
extern const char* KEY_DHCP_IP_MASK;
extern const char* KEY_DHCP_GATEWAY;
extern const char* KEY_WIFI_IP;
extern const char* KEY_WIFI_DNS;
extern const char* KEY_WIFI_IP_MASK;
extern const char* KEY_WIFI_GATEWAY;

#define DEFAULT_SAMPLING_RATE "44100"
#define RECORD_MODE_SPLITTED "0"
#define DEFAULT_AUDIO_FORMAT "wav"

#define MAX_MENU_SETTINGS (100)

#define CHANNEL_INVALID		(9999)

class config{

	static void initByConfig(std::string key, std::string defVal){
		menu_cfg[key] = _config.get(key, defVal).asString();
		syslog(LOG_INFO, " fw_menu_config[%s] => [%s]", key.c_str(), menu_cfg[key].c_str());				
	}

	static std::string getFwEnv(std::string key){
		std::string cmd = "fw_printenv | grep "+key+" | tr  '=' ' ' | awk '{print $2}'";
		return cmd::execS1(cmd);
	}

	static void RestoreAudioChannels();

public:
	static void load();
	static void save();
	static void init();

	static bool IsEssentialKey(std::string sKey);

	static std::string getChannelsAsString();

	static void setChannels(std::map<std::string,std::string>);

	static std::string GetConfig(std::string key){
		if (key == KEY_AUDIO_CHANNELS){
			return getChannelsAsString();
		} else if (key == KEY_IP){
			return eth::getIp();
		} else if (key == KEY_ETH_MAC){
			return eth::getEthMac();
		} else if (key == KEY_WIFI_MAC){
			return eth::getWiFiMac();
		} else if (key == KEY_DNS){
			return eth::getDns();
		} else if (key == KEY_GATEWAY){
			return eth::getDafaultGateway();
		} else if (key == KEY_MASK){
			return eth::getMask();
		}

		return menu_cfg[key];
	}

	static void SetConfig(std::string key,
								std::string value,
								bool bNotifyClients=true);

	static void SetConfig(std::string key, bool value){
		std::string sValue=value?"on":"off";
		SetConfig(key, sValue);
	}
	static bool GetConfigOnOffAsBool(std::string key){
		if (menu_cfg[key] == "on")
			return true;
		return false;
	}

	static bool isVisible(std::string key){
		if (key == KEY_STATIC_IP
			|| key == KEY_STATIC_DNS
			|| key == KEY_STATIC_IP_MASK
			|| key == KEY_STATIC_GATEWAY)
			return !GetConfigOnOffAsBool(KEY_DHCP);

		if (key == KEY_DHCP_IP
			|| key == KEY_DHCP_DNS
			|| key == KEY_DHCP_IP_MASK
			|| key == KEY_DHCP_GATEWAY)
			return GetConfigOnOffAsBool(KEY_DHCP);


		if (key == KEY_WIFI_SSID
			|| key == KEY_WIFI_PASSWORD
			|| key == KEY_WIFI_IP
			|| key == KEY_WIFI_DNS
			|| key == KEY_WIFI_IP_MASK
			|| key == KEY_WIFI_GATEWAY)
			return GetConfigOnOffAsBool(KEY_WIFI);


		return true;
	}

	static void OnSettingsChange(std::string& refKey, std::string& refValue);
	static void LoadDefaults();
	static std::map<std::string, std::string> GetSettings();
	static bool KeyExists(std::string &refKey);

private:
	static std::map<std::string, std::string> menu_cfg;
	static std::map<std::string, bool> _keys;
	static Json::Value  _config; 
    static bool _need_sync;
public:
	static unsigned HighestEnabledChannel;
};


#endif
