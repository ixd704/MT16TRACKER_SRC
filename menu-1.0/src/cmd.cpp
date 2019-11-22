/** ************************************************************************************
 *
 *          @file  cmd.cpp
 *
 *    	   @brief  
 *
 *       @version  1.0
 *          @date  09.01.2014 15:57:33
 *
 *        @author  Sudheer Divakaran 
 *
 *		 @details
 *
 *\if NODOC
 *       Revision History:
 *				09.01.2014 - Sudheer Divakaran - Created
 *\endif
 ***************************************************************************************/
#include <sys/wait.h>
#include <stdarg.h>
#include <cmd.h>

pid_t cmd::Fork(char *pCommand,...)
{

	pid_t pid = fork();
	if (-1 == pid) {
		return pid;
	}
	if (0 == pid) {
		va_list args;
		unsigned n,nArgs=0;
		char **pArgs;
		char **p;
		int fd;

		int maxfd=sysconf(_SC_OPEN_MAX);
		for(fd=3; fd < maxfd; fd++) {
			close(fd);
		}


		va_start(args, pCommand);
		do {
			nArgs++;
		} while(va_arg(args, char *));
		va_end(args);

		p = pArgs = (char**)malloc((nArgs+2) * sizeof(char *));

		va_start(args, pCommand);
		*p = basename(pCommand);
		p++;
		for (n=0; n < nArgs; n++) {
			*p++ = va_arg(args, char *);
		}
		va_end(args);
		*p=NULL;
		execvp(pCommand, pArgs);
		exit(EXIT_FAILURE);
	}

	return pid;
}

bool cmd::IsProcessAlive(pid_t pid)
{
	if (pid) {
		WaitPid();
		if(!kill(pid,0)) {
			return true;
		}
	}
	return false;
}

void cmd::KillProcess(pid_t pid)
{
	if (pid) {
		if(!kill(pid,SIGKILL)) {
			waitpid(pid,NULL,0);
		}
	}
}


void cmd::WaitPid()
{
	waitpid(-1,NULL,WNOHANG);
}
