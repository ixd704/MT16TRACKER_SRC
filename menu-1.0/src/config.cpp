#include "config.h"
#include <sstream>
#include "audio.h"
#include <syslog.h>

#include <NetworkServiceRpc.h>



std::map<std::string, std::string> config::menu_cfg;
std::map<std::string, bool> config::_keys;

Json::Value config::_config;
bool config::_need_sync =false;
unsigned config::HighestEnabledChannel;

const char* KEY_AUDIO_FOLDER="audioFolder";
const char* KEY_USB_FOLDER="usbFolder";
const char* KEY_CHANNELS="channels";
const char* KEY_RECORD_MODE="recordMode";
const char* KEY_SAMPLING_RATE="samplingRate";
const char* KEY_AUDIO_FORMAT="audioFormat";
const char* KEY_DHCP="dhcp";
const char* KEY_STATIC_DNS="staticDns";
const char* KEY_STATIC_GATEWAY="staticGateway";
const char* KEY_STATIC_IP="staticIp";
const char* KEY_STATIC_IP_MASK="staticIpMask";
const char* KEY_WIFI="wifi";
const char* KEY_WIFI_PASSWORD="wifiPassword";
const char* KEY_WIFI_SSID="wifiSsid";


const char* KEY_CODE_REV="codeRev";
const char* KEY_PIN="pin";
const char* KEY_SERVICE_STATUS="serviceStatus";
const char* KEY_AUDIO_RECORD="audioRecord";
const char* KEY_SUPPORT_EMAIL="supportEmail";
const char* KEY_SUPPORT_PHONE="supportPhone";



const char* KEY_AUDIO_CHANNELS="audioChannels";
const char* KEY_IP="ip";
const char* KEY_ETH_MAC="ethMac";
const char* KEY_WIFI_MAC="wifiMac";
const char* KEY_DNS="dns";
const char* KEY_GATEWAY="gateway";
const char* KEY_MASK="mask";
const char* KEY_DHCP_IP="dhcpIP";
const char* KEY_DHCP_DNS="dhcpDns";
const char* KEY_DHCP_IP_MASK="dhcpMask";
const char* KEY_DHCP_GATEWAY="dhcpGateway";
const char* KEY_WIFI_IP="wifiIp";
const char* KEY_WIFI_DNS="wifiDns";
const char* KEY_WIFI_IP_MASK="wifiMask";
const char* KEY_WIFI_GATEWAY="wifiGateway";


static pthread_mutex_t mutex_ns = PTHREAD_MUTEX_INITIALIZER;


std::string config::getChannelsAsString(){
	syslog(LOG_INFO,"getChannelsAsString");

	std::string out;
	Json::Value  root;   // will contains the root value after parsing.
	Json::Reader reader;
	std::string  audio_channels = config::GetConfig(KEY_CHANNELS);
	syslog(LOG_INFO,"audio channels %s", audio_channels.c_str());
	bool parsingSuccessful = reader.parse(audio_channels, root );

	if ( !parsingSuccessful ){
		syslog(LOG_ERR,"ERROR: can't parse channels state");
		return out;
	}

	Json::Value chnls = root.get(KEY_CHANNELS, Json::Value(Json::arrayValue));
	for (unsigned i = 0; i < chnls.size(); i++){
		out += chnls[i].get("num",CHANNEL_INVALID).asString();
		out += "; ";
	}

	return out;
}



void config::setChannels(std::map<std::string,std::string> in)
{
	Json::Value root;
	Json::Value channels;

	HighestEnabledChannel = 0;

	for(std::map<std::string,std::string>::iterator iter = in.begin(); iter != in.end(); ++iter){
		if (iter->second == "on") {
			Json::Value channel;
			unsigned nChannel = atoi(iter->first.c_str());
			if (nChannel > HighestEnabledChannel) {
				HighestEnabledChannel = nChannel;
			}
			channel["num"]=Json::Value(Json::Int(nChannel));
			channels.append(channel);
		}
	}	
	root[KEY_CHANNELS] = channels;
	config::SetConfig(KEY_CHANNELS, root.toCompactString());
	syslog(LOG_INFO,"set_channels: %s",menu_cfg[KEY_CHANNELS].c_str());
}

void config::OnSettingsChange(std::string& refKey, std::string& refValue)
{
	if (refKey == KEY_STATIC_IP){
		eth::setIp(menu_cfg[KEY_STATIC_IP], menu_cfg[KEY_STATIC_IP_MASK]);
	} else if (refKey == KEY_STATIC_IP_MASK){
		eth::setIp(menu_cfg[KEY_STATIC_IP], menu_cfg[KEY_STATIC_IP_MASK]);
	} else if (refKey == KEY_STATIC_GATEWAY){
		eth::setGateway(menu_cfg[KEY_STATIC_GATEWAY]);
	} else if (refKey == KEY_STATIC_DNS){
		eth::setDns(menu_cfg[KEY_STATIC_DNS]);
	} else if (refKey == KEY_RECORD_MODE || refKey == KEY_SAMPLING_RATE ||
			refKey == KEY_CHANNELS || refKey == KEY_AUDIO_FORMAT) {
		NotifyAudioParameters();
	}
}

void config::RestoreAudioChannels()
{
	Json::Value root;

	Json::Value oChannels =  _config.get(KEY_CHANNELS, Json::Value(Json::arrayValue));
	HighestEnabledChannel = 0;
	for (unsigned i = 0; i < oChannels.size(); i++){
		Json::Value channel = oChannels[i].get("num",CHANNEL_INVALID);
		if (Json::intValue == channel.type()){
			unsigned nChannel = channel.asInt();
			if (CHANNEL_INVALID != nChannel && nChannel > HighestEnabledChannel) {
				HighestEnabledChannel = nChannel;
			}
		}
	}

	root[KEY_CHANNELS]=oChannels;

	menu_cfg[KEY_CHANNELS] = root.toCompactString();

	NotifyAudioParameters();
}

void config::LoadDefaults()
{

	initByConfig(KEY_USB_FOLDER, "/mnt/usb");
	initByConfig(KEY_AUDIO_FOLDER, "/mnt/floppy/audio");
	initByConfig(KEY_WIFI, "off");
	initByConfig(KEY_WIFI_SSID, "");
	initByConfig(KEY_WIFI_PASSWORD, "");
	initByConfig(KEY_DHCP, "on");
	initByConfig(KEY_WIFI, "off");

	initByConfig(KEY_SAMPLING_RATE, DEFAULT_SAMPLING_RATE);
	initByConfig(KEY_RECORD_MODE, RECORD_MODE_SPLITTED);
	initByConfig(KEY_AUDIO_FORMAT, DEFAULT_AUDIO_FORMAT);


	initByConfig(KEY_STATIC_IP,"192.168.0.222");
	initByConfig(KEY_STATIC_IP_MASK,"255.255.255.0");
	initByConfig(KEY_STATIC_DNS,"192.168.0.1");
	initByConfig(KEY_STATIC_GATEWAY,"192.168.0.1");


	RestoreAudioChannels();

	//init status
	initByConfig(KEY_CODE_REV,"0.0000");
	initByConfig(KEY_SUPPORT_PHONE,"+7-916-505-4887");
	initByConfig(KEY_SUPPORT_EMAIL,"support@ya.ru");

	menu_cfg[KEY_SERVICE_STATUS] 	= "offline";


	menu_cfg[KEY_PIN] = "";
	menu_cfg[KEY_AUDIO_RECORD] = "off";

}



void config::init()
{
	_keys[KEY_AUDIO_FOLDER] = true;
	_keys[KEY_USB_FOLDER] = true;
	_keys[KEY_CHANNELS] = true;
	_keys[KEY_RECORD_MODE] = true;
	_keys[KEY_SAMPLING_RATE] = true;
	_keys[KEY_AUDIO_FORMAT] = true;
	_keys[KEY_DHCP] = true;
	_keys[KEY_STATIC_DNS] = true;
	_keys[KEY_STATIC_GATEWAY] = true;
	_keys[KEY_STATIC_IP] = true;
	_keys[KEY_STATIC_IP_MASK] = true;
	_keys[KEY_WIFI] = true;
	_keys[KEY_WIFI_PASSWORD] = true;
	_keys[KEY_WIFI_SSID] = true;

	_keys[KEY_CODE_REV] = false;
	_keys[KEY_PIN] = false;
	_keys[KEY_SERVICE_STATUS] = false;
	_keys[KEY_AUDIO_RECORD] = false;
	_keys[KEY_SUPPORT_EMAIL] = false;
	_keys[KEY_SUPPORT_PHONE] = false;



	_keys[KEY_AUDIO_CHANNELS] = false;
	_keys[KEY_IP] = false;
	_keys[KEY_ETH_MAC] = false;
	_keys[KEY_WIFI_MAC] = false;
	_keys[KEY_DNS] = false;
	_keys[KEY_GATEWAY] = false;
	_keys[KEY_MASK] = false;
	_keys[KEY_DHCP_IP] = false;
	_keys[KEY_DHCP_DNS] = false;
	_keys[KEY_DHCP_IP_MASK] = false;
	_keys[KEY_DHCP_GATEWAY] = false;
	_keys[KEY_WIFI_IP] = false;
	_keys[KEY_WIFI_DNS] = false;
	_keys[KEY_WIFI_IP_MASK] = false;
	_keys[KEY_WIFI_GATEWAY] = false;




	load();

	LoadDefaults();


	for (std::map<std::string, std::string>::iterator it = menu_cfg.begin(); it != menu_cfg.end(); it++){
		syslog(LOG_INFO,"cfg [%s]=>[%s]", it->first.c_str(), it->second.c_str());
	}


	syslog(LOG_INFO,"init audio channels by saved configuration [%s]", menu_cfg[KEY_CHANNELS].c_str());

	syslog(LOG_INFO,"init network by saved configuration:");
	if (menu_cfg[KEY_DHCP] == "on"){
		syslog(LOG_INFO,"\tdhcp - on");
		eth::initByDhcp();
	}
	else if (menu_cfg[KEY_WIFI] == "on"){
		syslog(LOG_INFO,"\twifi - on");
		wifi::on(menu_cfg[KEY_WIFI_SSID], menu_cfg[KEY_WIFI_PASSWORD]);
	}
	else {
		syslog(LOG_INFO,"\tstatic eth - on");
		eth::setEth(menu_cfg[KEY_STATIC_IP], menu_cfg[KEY_STATIC_IP_MASK], menu_cfg[KEY_STATIC_GATEWAY], menu_cfg[KEY_STATIC_DNS]);
	}
}

bool config::IsEssentialKey(std::string sKey)
{
	if (KeyExists(sKey))
		return _keys[sKey];
	else
		return false;
}

void config::load()
{
	std::string cfg = getFwEnv("fw_menu_config");

	Json::Reader reader;

	if (!reader.parse(cfg, _config))
		syslog(LOG_ERR, "Can't parse config from fw_env !%s!, and use default values:", cfg.c_str());
	syslog(LOG_INFO, "fw_env  %s", cfg.c_str());
}


void config::save(){
	if (!_need_sync){
		return;
	}

	pthread_mutex_lock(&mutex_ns);
	_need_sync = false;

	Json::Value root;
	Json::Value _channel_root;
	for(std::map<std::string, std::string>::iterator it = menu_cfg.begin(); it != menu_cfg.end(); it++){
		if (!IsEssentialKey(it->first))
			continue;

		if (it->first != KEY_CHANNELS) {
			root[it->first] = Json::Value(it->second);
		} else {
			Json::Reader reader;
			if (reader.parse(menu_cfg[KEY_CHANNELS].c_str(), _channel_root)) {
				root[it->first] = _channel_root.get(KEY_CHANNELS, Json::Value(Json::arrayValue));
			}
		}

	}
	pthread_mutex_unlock(&mutex_ns);

	cmd::exec("fw_setenv fw_menu_config '" + root.toCompactString() + "'");
}

std::map<std::string, std::string> config::GetSettings()
{
	return menu_cfg;
}

void config::SetConfig(std::string key,
		std::string value,
		bool bNotifyClients)
{
	if (!KeyExists(key)) {
		syslog(LOG_ERR, "Ignoring unknown key '%s'",key.c_str());
		return;
	}
	syslog(LOG_INFO, "set(%s)=%s",key.c_str(),value.c_str());
	menu_cfg[key] = value;
	bool bEssentialKey = IsEssentialKey(key);

	OnSettingsChange(key, value);

	pthread_mutex_lock(&mutex_ns);
	if (bEssentialKey) {
		_need_sync = true;
	}
	pthread_mutex_unlock(&mutex_ns);

	NotifySettingsToNetworkService(menu_cfg);

}

bool config::KeyExists(std::string &refKey)
{
	return _keys.find(refKey) != _keys.end();
}
