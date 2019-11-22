#ifndef ETH_H
#define ETH_H 1

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h> 
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/sockios.h>

#include <netpacket/packet.h>
#include <net/ethernet.h>

#include "cmd.h"

using namespace std;

#define  ETH_NAME "eth0"
#define  WIFI_NAME "wlan0"

class eth {

private: 
	static std::string getIoctl(int request){
		int fd;
		struct ifreq ifr;

		fd = socket(AF_INET, SOCK_DGRAM, 0);
		ifr.ifr_addr.sa_family = AF_INET;
		strncpy(ifr.ifr_name, ETH_NAME, IFNAMSIZ-1);
 		ioctl(fd, request, &ifr);
		close(fd);
		
		return std::string(inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));

	}

/*
	static std::string getCmdOut(std::string cmd){
		std::string out;
		syslog(LOG_ERR, cmd.c_str());

  		FILE* fp 	= popen(cmd.c_str(), "r");
  		char line[1024]={0x0};
		memset( line, '\0', sizeof(line) );

		while(fgets(line, sizeof(line), fp) != NULL){    
			if (out.size() == 0)
				out = std::string(line);

			syslog(LOG_ERR, line);
			memset( line, '\0', sizeof(line) );
		}

	  	pclose(fp);
	  	return out;
	}

*/
public:

	static string initByDhcp(){
		return cmd::execS1("udhcpc -b");

	}

	static string setIp(string ip, string mask){   
        syslog(LOG_INFO, "set static ip [%s] mask[%s]",ip.c_str(), mask.c_str());
		char cmd[1024]={0x0};
		memset( cmd, '\0', sizeof(cmd) );
		snprintf(cmd, sizeof(cmd), "ifconfig %s %s netmask %s", ETH_NAME, ip.c_str(), mask.c_str());
		return cmd::execS1(cmd);

	}
	static string setGateway(std::string dg){
        syslog(LOG_INFO, "set static default gateway %s",dg.c_str());

		char cmd[1024]={0x0};
		memset( cmd, '\0', sizeof(cmd) );
		snprintf(cmd, sizeof(cmd), "route add default gw  %s %s 2>&1", dg.c_str(), ETH_NAME);
		return cmd::execS1(cmd);

	}
	static string setDns(std::string dns){
        syslog(LOG_INFO, "set static dns %s",dns.c_str());
		char cmd[1024]={0x0};
		memset( cmd, '\0', sizeof(cmd) );
		snprintf(cmd, sizeof(cmd), "echo \"nameserver %s\" > /etc/resolv.conf", dns.c_str());
		return cmd::execS1(cmd);

	}

	
	static string setEth(std::string ip, std::string mask, std::string dg, std::string dns)
	{
		string out = setIp(ip, mask);		
		if (out.size() > 0) {
			syslog(LOG_ERR, "%s", out.c_str());
			return out;
		}

		out = setGateway(dg);
		if (out.size() > 0){
			syslog(LOG_ERR, "%s", out.c_str());
			return out;
		}

		out = setDns(dns);
		if (out.size() > 0){
			syslog(LOG_ERR, "%s", out.c_str());
			return out;
		}

		return "";
	}

	static string getEthMac(){
		char cmd[1024]={0x0};
		memset( cmd, '\0', sizeof(cmd) );
		snprintf(cmd, sizeof(cmd), "ifconfig | grep %s | awk '$0 ~ /HWaddr/ { print $5 }'", ETH_NAME);

		return cmd::execS1(cmd);
	} 

	static string getWiFiMac(){
		char cmd[1024]={0x0};
		memset( cmd, '\0', sizeof(cmd) );
		snprintf(cmd, sizeof(cmd), "ifconfig | grep %s | awk '$0 ~ /HWaddr/ { print $5 }'", WIFI_NAME);

		return cmd::execS1(cmd);
	} 

	static string getIp(){
		return getIoctl(SIOCGIFADDR);
	} 

	static string getMask(){
		return getIoctl(SIOCGIFNETMASK);
	} 
	

	static string getDafaultGateway(){
		return cmd::execS1("netstat -r | grep default | awk '{print $2}'");
	}


	static string getDns(){
		return cmd::execS1("cat /etc/resolv.conf | grep -v '^#' | grep nameserver | awk '{print $2}' ") ;
	}


	static string up(){
		char cmd[1024]={0x0};
		memset( cmd, '\0', sizeof(cmd) );
		snprintf(cmd, sizeof(cmd), "ifconfig %s up'", ETH_NAME);

		return cmd::execS1(cmd);
	} 

	static string down(){
		char cmd[1024]={0x0};
		memset( cmd, '\0', sizeof(cmd) );
		snprintf(cmd, sizeof(cmd), "ifconfig %s down'", ETH_NAME);

		return cmd::execS1(cmd);
	} 


};


#endif
