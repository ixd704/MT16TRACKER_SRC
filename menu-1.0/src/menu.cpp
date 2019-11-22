#include <stdio.h>
#include <string.h>

#include <sys/stat.h>
#include <fcntl.h>

#include <errno.h>
#include "lcd.h"


#ifndef _MSC_VER
	#include <unistd.h>
	#include <syslog.h>
	#include <pthread.h>
#endif

#ifdef _MSC_VER 
#ifndef usleep
#  include <stdlib.h>
#  define usleep(t) _sleep((t) / 1000)	
#endif
#endif




#include "menuEntry.h"
#include "menuEntryBoolean.h"
#include "menuEntryString.h"
#include "menuEntryList.h"
#include <MenuEntryListAndOptions.h>
#include <AudioFiles.h>
#include <ToggleMenuEntry.h>
#include <timer.h>

#include "lcd.h"
#include "btns.h"
#include "config.h"

#include "json/reader.h"

#ifndef _MSC_VER 
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h> 
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/sockios.h>

#include <netpacket/packet.h>
#include <net/ethernet.h>


#endif


#include "cmd.h"
#include "eth.h"
#include "wifi.h"
#include "audio.h"
#include <MenuRpcService.h>
#include <NetworkServiceRpc.h>


#include <vector>

#ifdef USE_CURSES_LCD
#include <ncurses.h>
#endif

#include <tracker.pb-c.h>
#include "menu.h"

#define MENU_VERSION "1.12"
#define MENU_COMMIT "4a7fee0f-a1d9-4e53-a0b8-3390c756a871"

pthread_mutex_t mutex_fw = PTHREAD_MUTEX_INITIALIZER;


using namespace std;

menuEntry* Menu::m_pActiveMenuEntry;
int Menu::argc;
char **Menu::argv;
bool Menu::m_bRemoteControlled;
bool Menu::m_bRecordingStatus;

pthread_mutex_t MenuMutex = PTHREAD_MUTEX_INITIALIZER;


void * thread_func_fw(void *arg)	{

	do{
        sleep(10);
        config::save();
	}  while(1);
	
	return NULL;
}

#ifdef USE_CURSES_LCD
	int keyboard_buttons_emulation;

	void* keyboard_thread_func(void *arg)
	{
		btns::queue_keyboard_button_events();
		return NULL;
	}

	void start_keyboard_thread()
	{
		pthread_t th_keyboard;
		if (pthread_create(&th_keyboard, NULL, keyboard_thread_func, NULL)) {
			syslog(LOG_ERR, "Creating keyboard thread failed");
		}
	}

#endif





int main (int argc, char *argv[])
{
	Menu::argc = argc;
	Menu::argv = argv;

	Menu::StartService();

	return 0;
}



void Menu::Initialize()
{
    syslog(LOG_INFO,"menu rel:%s.%s is running", MENU_VERSION, MENU_COMMIT);
	int result;
	pthread_t fwThread;
	int id2 = 2;
	 result = pthread_create(&fwThread, NULL, thread_func_fw, &id2);
	if (result != 0) {
		syslog(LOG_ERR, "Creating the fw thread failed");
		exit(1);
	}

	pthread_t thRpcServer;
	if (pthread_create(&thRpcServer, NULL, menu_rpc_thread_function, NULL))
	{
		syslog(LOG_ERR, "Creating the RPC thread failed");
		exit(1);
	}

	pthread_t thTimer;
	if(pthread_create(&thTimer, NULL, Timer::ThreadFn, NULL))
	{
		syslog(LOG_ERR, "Creating Timer thread failed");
		exit(1);
	}

	InitializeNetworkServiceRpc();


#ifdef USE_CURSES_LCD
	int c;
	while ((c = getopt (argc, argv, "k")) != -1) {
		switch (c)
		{
			case 'k':
				keyboard_buttons_emulation = 1;
				syslog(LOG_INFO, "Keyboard button emulation enabled");
				break;
			default:
				break;
		}
	}
#endif



#ifdef CMD
	syslog(LOG_INFO, "WARNING: menu run in cmd mode");
#endif

	rpc_init();
#ifdef CMD
	string str = "{\"channels\":[{\"num\":5,\"codec\":\"mp3\"}, {\"num\":7,\"codec\":\"alac\"}]}";
	set_channels(str);
#endif

	lcd::init();
	lcd::clear();
	btns::init();
	config::init();
}


void Menu::StartService()
{

	Initialize();

	menuEntry* mConfig 			= new menuEntry("Config", NULL, NULL);
	menuEntry* mCloud 			= new menuEntry("Cloud", mConfig, NULL);

	menuEntry* mReboot 			= new menuEntry("Reboot", mCloud, NULL);
	menuEntry* mFactoryDefaults = new menuEntry("Factory Defaults", mReboot, NULL);
	menuEntry* mStatus 			= new menuEntry("Status", mFactoryDefaults, NULL);


	//Service
	menuEntry* mCloudAuth 		= new menuEntryWifiSsid("AuthToken", NULL, mCloud,	KEY_PIN, false);
	menuEntry* mCloudStatus 	= new menuEntryWifiSsid("Connect Status", mCloudAuth,	mCloud,	KEY_SERVICE_STATUS, false);
	mCloud->setChildren(mCloudAuth);

	// Config
	menuEntry* mWiFi 		= new menuEntry("WiFi", NULL, mConfig);
	menuEntry* mEthernet 	= new menuEntry("Ethernet",	mWiFi, mConfig);
	menuEntry* mAudio 		= new menuEntry("Audio", mEthernet,	mConfig);
	mConfig->setChildren(mWiFi);



	// Ethernet
	menuEntry* mDHCP				= new menuEntryBoolean("DHCP", NULL, mEthernet,	KEY_DHCP, true);
	menuEntry* mStaticIp			= new menuEntryIp("Static IP", mDHCP, mEthernet, KEY_STATIC_IP, true);
	menuEntry* mStaticMask			= new menuEntryIp("Static IP mask", mStaticIp, mEthernet, KEY_STATIC_IP_MASK, true);
	menuEntry* mStaticGateway		= new menuEntryIp("Static Gateway", mStaticMask, mEthernet,	KEY_STATIC_GATEWAY, true);
	menuEntry* mStaticDNS			= new menuEntryIp("Static DNS", mStaticGateway, mEthernet, KEY_STATIC_DNS, true);
	mEthernet->setChildren(mDHCP);

	// Wifi
	menuEntry* mWiFiOnOff			= new menuEntryBoolean("WiFi", NULL, mWiFi,	KEY_WIFI, true);
	menuEntry* mWifiSsid			= new menuEntryWifiSsid("WiFi SSID", mWiFiOnOff, mWiFi,	KEY_WIFI_SSID, true);
	menuEntry* mWifiPassword		= new menuEntryPassword("WiFi password", mWifiSsid, mWiFi, KEY_WIFI_PASSWORD, true);
	mWiFi->setChildren(mWiFiOnOff);

	//Audio
	menuEntry* mAudioChannels		= new menuEntryChannels("Channels",	NULL, mAudio,	KEY_AUDIO_CHANNELS, true);
	mAudio->setChildren(mAudioChannels);

	std::map<std::string,std::string> mapOptions;

	mapOptions["0"] = "Splitted";
	mapOptions["1"] = "Combined";
	menuEntry* mRecordMode = new MenuEntryReadOnlyList("Record Mode", mAudioChannels, mAudio, KEY_RECORD_MODE, mapOptions);

	mapOptions.clear();
	mapOptions["44100"] = "44.1 KHz";
	mapOptions["48000"] = "48 KHz";
	mapOptions["96000"] = "96 KHz";

	menuEntry* mSamplingRates = new MenuEntryReadOnlyList("Sampling Rate", mRecordMode, mAudio,KEY_SAMPLING_RATE, mapOptions);

	mapOptions.clear();
	mapOptions["wav"] = "WAV";
	mapOptions["mp3"] = "MP3";
	mapOptions["alac"] = "ALAC";
	menuEntry* mAudioFormat = new MenuEntryReadOnlyList("Audio Format", mSamplingRates, mAudio, KEY_AUDIO_FORMAT, mapOptions);

	menuEntry* mAudioRecord	= new ToggleMenuEntry("Record:off", mAudioFormat, mAudio);
	SetupAudioRecordHandler((ToggleMenuEntry*)mAudioRecord);

	menuEntry* mFiles				= new MenuEntryListAndOptions("Files", mAudioRecord, mAudio);
	SetAudioFileListParameters(mFiles);


	//Status
	menuEntry* mStatusEthernet 	= new menuEntry("-= Network =-", NULL, mStatus);
	mStatus->setChildren(mStatusEthernet);

	menuEntry* mStatusDHCP			= new menuEntryBoolean("DHCP", mStatusEthernet, mStatus, KEY_DHCP, false);
	menuEntry* mStatusWiFiOnOff		= new menuEntryBoolean("WiFi", mStatusDHCP, mStatus, KEY_WIFI, false);

	menuEntry* mStatusStaticIp		= new menuEntryIp("IP", mStatusWiFiOnOff, mStatus, KEY_IP, false);
	menuEntry* mStatusStaticMask	= new menuEntryIp("IP Mask", mStatusStaticIp, mStatus, KEY_MASK, false);
	menuEntry* mStatusStaticGateway	= new menuEntryIp("Default Gateway", mStatusStaticMask, mStatus, KEY_GATEWAY, false);
	menuEntry* mStatusStaticDNS		= new menuEntryIp("DNS", mStatusStaticGateway, mStatus,	KEY_DNS, false);

	menuEntry* mStatusStaticMac		= new menuEntryIp("Ethernet Mac Address", mStatusStaticDNS,	mStatus, KEY_ETH_MAC, false);
	menuEntry* mStatusWiFiMac		= new menuEntryIp("WiFi Mac Address", mStatusStaticMac, mStatus, KEY_WIFI_MAC, false);



	menuEntry* mStatusAudio 		= new menuEntry("-= Audio =-", mStatusWiFiMac,	mStatus);

	menuEntry* mStatusAudioChannels		= new menuEntryChannels("Channels",	mStatusAudio, mStatus, KEY_AUDIO_CHANNELS, false);
	menuEntry* mStatusAudioRecordOnOff	= new menuEntryBoolean("Record", mStatusAudioChannels, mStatus, KEY_AUDIO_RECORD, false);

	menuEntry* mStatusSystem 		= new menuEntry("-= System =-", mStatusAudioRecordOnOff, mStatus);
	menuEntry* mStatusCodeRev		= new menuEntryString("Code rev", mStatusSystem, mStatus, KEY_CODE_REV,false, NULL);
	menuEntry* mStatusSupportPhone	= new menuEntryString("Support Phone", mStatusCodeRev, mStatus,	KEY_SUPPORT_PHONE, false, NULL);
	menuEntry* mStatusSupportEmail	= new menuEntryString("Support Email", mStatusSupportPhone, mStatus, KEY_SUPPORT_EMAIL, false, NULL);


	m_pActiveMenuEntry = mConfig;
	m_pActiveMenuEntry->print();

///Btn loop

	do {
		m_pActiveMenuEntry = m_pActiveMenuEntry->on(btns::get_btn());
		lcd::tick();
		usleep(100*1000);
	} while (1);

}

void Menu::OnNetworkServiceStatusChanged(const Tracker__NetworkServiceStatus *pStatus)
{
	if (!pStatus)
		return;

	if (TRACKER__NETWORK_STATUS_ENUM__REMOTE_CONTROLLED != pStatus->servicestatus) {
		if (m_bRemoteControlled) {
			EnableRemoteControl(false);
			m_bRemoteControlled = false;
		}
	}

	switch (pStatus->servicestatus) {
		case TRACKER__NETWORK_STATUS_ENUM__OFFLINE:
			config::SetConfig(KEY_SERVICE_STATUS, "offline", false);
			break;
		case TRACKER__NETWORK_STATUS_ENUM__BINDING_TO_ACCOUNT:
			if (pStatus->pincode) {
				config::SetConfig(KEY_PIN, pStatus->pincode, false);
			}
			break;
		case TRACKER__NETWORK_STATUS_ENUM__ONLINE:
			config::SetConfig(KEY_SERVICE_STATUS, "online", false);
			break;
		case TRACKER__NETWORK_STATUS_ENUM__REMOTE_CONTROLLED:
			m_bRemoteControlled = true;
			EnableRemoteControl(true);
			break;
		default:
			break;
	}

}


void  Menu::EnableRemoteControl(bool bRemoteControl)
{
	if (bRemoteControl) {
		const char *sMsg = "RemoteControlled";
		btns::freeze(true);
		lcd::clear();
		lcd::write(sMsg, strlen(sMsg), 0, 0);

	} else {
		m_pActiveMenuEntry->print();
		btns::freeze(false);
	}
}

void Menu::SetRecordingStatus(bool bStatus)
{
	m_bRecordingStatus = bStatus;
}

bool Menu::IsRecording()
{
	return true==m_bRecordingStatus;
}
