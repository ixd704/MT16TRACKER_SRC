#ifndef CMD_H
#define CMD_H 1

#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <syslog.h>

#include <string>
#include <vector>

using namespace std;

class cmd {

public: 
        
    static void reboot(){
        exec("reboot -f");
    }
    
	static  vector<string> exec(string cmd) {
		vector<string> out;
		string run = cmd;
		run = run + " 2>&1";
		
		syslog(LOG_DEBUG, "Exec [%s]\n", cmd.c_str());
  		FILE* fp = popen(run.c_str(), "r");
    		if (!fp) {
			syslog(LOG_ERR, "Cannot execute command [%s]\n", run.c_str());
			return out;
		}

  		char line[10240]={0x0};
		memset( line, '\0', sizeof(line) );

		while(fgets(line, sizeof(line), fp) != NULL){    
			syslog(LOG_DEBUG, "%s", line);
		        out.push_back(string(line));
			memset( line, '\0', sizeof(line) );
		}


    		if (pclose(fp) != 0) {
        		syslog(LOG_ERR, "Cannot close stream 4 command [%s]\n", cmd.c_str());
		}

		return out;
    	}


	static string execS1(string cmd){
		vector<string> out = exec(cmd);
		return (out.size()>0)?out[0]:string();
	}

	static pid_t Fork(char *pCommand,...);

	static bool IsProcessAlive(pid_t pid);

	static void KillProcess(pid_t pid);
	static void WaitPid();
};





#endif
