#ifndef WIFI_H
#define WIFI_H 1

#include <string>
#include <vector>
#include "cmd.h"
#include "eth.h"
#define  WIFI_NAME "wlan0"

class wifi {

public:

	static string up(){
		char cmd[1024]={0x0};
		memset( cmd, '\0', sizeof(cmd) );
		snprintf(cmd, sizeof(cmd), "ifconfig %s up'", WIFI_NAME);

		return cmd::execS1(cmd);
	} 

	static string down(){
		char cmd[1024]={0x0};
		memset( cmd, '\0', sizeof(cmd) );
		snprintf(cmd, sizeof(cmd), "ifconfig %s down'", WIFI_NAME);

		return cmd::execS1(cmd);
	} 
	

	static string getMac(){
		char cmd[1024]={0x0};
		memset( cmd, '\0', sizeof(cmd) );
		snprintf(cmd, sizeof(cmd), "ifconfig | grep %s | awk '$0 ~ /HWaddr/ { print $5 }'", WIFI_NAME);

		return cmd::execS1(cmd);
	} 

	static bool isOn(){
	    return !getMac().empty();		
	}

	static vector<string> scan(){
        char cmd[1024]={0x0};
		memset( cmd, '\0', sizeof(cmd) );
		snprintf(cmd, sizeof(cmd), "iwlist %s scanning | grep ESSID | sed 's/ESSID:\"//' | sed 's/\"//' |  tr -d ' '", WIFI_NAME);
		return cmd::exec(cmd);
	}

    static vector<string> con(std::string essid, std::string key){
        char cmd[1024]={0x0};
		memset( cmd, '\0', sizeof(cmd) );
		snprintf(cmd, sizeof(cmd), "iwconfig wlan0 essid %s key %s", essid.c_str(), key.c_str());
        return  cmd::exec(cmd);
	}
	static vector<string> dhcp_init(){
        char cmd[1024]={0x0};
		memset( cmd, '\0', sizeof(cmd) );
		snprintf(cmd, sizeof(cmd), "dhclient %s", WIFI_NAME);
		return cmd::exec(cmd);
	}
    
    static void on(std::string essid, std::string key){
        wifi::up();
        wifi::scan();
        wifi::con(essid,key);    
        wifi::dhcp_init();
        eth::down();                
    }
            
    static void off(){
        wifi::down();
    }

};


#endif
