/** ************************************************************************************
 *
 *          @file  AudioFiles.cpp
 *
 *    	   @brief  
 *
 *       @version  1.0
 *          @date  27.12.2013 21:54:14
 *
 *        @author  Sudheer Divakaran 
 *
 *		 @details
 *
 *\if NODOC
 *       Revision History:
 *				27.12.2013 - Sudheer Divakaran - Created
 *\endif
 ***************************************************************************************/
#include <sys/types.h>
#include <dirent.h>

#include <string>
#include <vector>
#include <config.h>
#include <utils.h>
#include <menuEntry.h>
#include <MenuEntryListAndOptions.h>
#include <AudioFiles.h>
#include <NetworkServiceRpc.h>
#include <cmd.h>
#include <timer.h>
#include <lcd.h>

#define CMD_PLAY			(1)
#define CMD_UPLOAD_TO_CLOUD (2)
#define CMD_COPY_TO_USB 	(3)
#define CMD_DELETE			(4)

extern pthread_mutex_t MenuMutex;
static pid_t pid_player;
static time_t PlayBackStartTime;
static std::string sPlaybackFile;

static void ReloadFiles(MenuEntryListAndOptions *pMenu)
{
	DIR *dir;
	struct dirent *ent;
	std::string sAudioFolder = config::GetConfig(KEY_AUDIO_FOLDER);
	if ((dir = opendir (sAudioFolder.c_str())) != NULL) {
		StringList lstFiles;
		while ((ent = readdir (dir)) != NULL) {
			if (strlen(ent->d_name) > 3) {
				lstFiles.push_back (ent->d_name);
			}
		}
		closedir (dir);
		pMenu->SetList(lstFiles);
	} else {
		syslog(LOG_ERR,"could not open AudioFolder");
	}

}


void OnDisplay(menuEntry* pMenuEntry)
{
	MenuEntryListAndOptions *pMenu = (MenuEntryListAndOptions *) pMenuEntry;

	ReloadFiles(pMenu);
}

void OnHide(menuEntry* pMenuEntry)
{
	MenuEntryListAndOptions *pMenu = (MenuEntryListAndOptions *) pMenuEntry;
	pMenu->GetList().clear();
	pMenu->Reset();
}

static void StopPlayback(MenuEntryListAndOptions *pMenu)
{
	if (pid_player) {
		cmd::KillProcess(pid_player);
		pid_player = 0;
		pMenu->SetButtonMask(BTN_MASK_ALL);
		pMenu->print();
	}
}

static void DisplayPlayBackInfo()
{
	char sBuffer[32];
	time_t nElapsedTime = time(NULL) - PlayBackStartTime;
	time_t nMinutes = nElapsedTime / 60;
	snprintf(sBuffer,32,"%003d:%02d Playing..",nMinutes, nElapsedTime % 60);
	lcd::clear();
	lcd::write(sBuffer, strlen(sBuffer), 0, 0);
	lcd::write(sPlaybackFile.c_str(), sPlaybackFile.size(), 1, 0);
}

bool PlayerTimerCallBack(void *pArg)
{
	bool bRetval=true;
	MenuEntryListAndOptions *pMenu = pArg;

	pthread_mutex_lock(&MenuMutex);
	if (!cmd::IsProcessAlive(pid_player)) {
		StopPlayback(pMenu);
		bRetval = false;
	} else {
		DisplayPlayBackInfo();
	}
	pthread_mutex_unlock(&MenuMutex);

	return bRetval;
}

void OnOptionSelection(menuEntry* pMenuEntry, const std::string& sListEntry, const std::string& sOption)
{
	MenuEntryListAndOptions *pMenu = (MenuEntryListAndOptions *) pMenuEntry;
	unsigned nCommand = pMenu->GetOptionIndex();
	std::string cmd;
	switch (nCommand) {
		case CMD_PLAY:
			if (!pid_player) {
				cmd = config::GetConfig(KEY_AUDIO_FOLDER);
				AddDirSeperator(cmd);
				cmd += sListEntry;
				pid_player = cmd::Fork("mplayer", (char*)cmd.c_str(), NULL);
				if (-1 != pid_player) {
					PlayBackStartTime=time(NULL);
					sPlaybackFile = sListEntry;
					pMenuEntry->SetButtonMask(BTN_RIGHT);
					DisplayPlayBackInfo();
					Timer::AddClient(PlayerTimerCallBack, pMenu,1);
				} else {
					pid_player = 0;
				}
			} else {
				pthread_mutex_lock(&MenuMutex);
				StopPlayback(pMenu);
				pthread_mutex_unlock(&MenuMutex);
			}
			break;
		case CMD_UPLOAD_TO_CLOUD:
			cmd = config::GetConfig(KEY_AUDIO_FOLDER);
			AddDirSeperator(cmd);
			cmd += sListEntry;
			if (!UploadToCloud(cmd)) {
				syslog(LOG_ERR, "UploadToCloud failed");
			}
			break;
		case CMD_COPY_TO_USB:
			cmd = "cp \""+config::GetConfig(KEY_AUDIO_FOLDER)+"/"+sListEntry+"\" \""+config::GetConfig(KEY_USB_FOLDER)+"\"";
			syslog(LOG_INFO, "loadToUSB [%s]", cmd.c_str());
			cmd::exec(cmd);
			break;
		case CMD_DELETE:
			cmd = "rm \""+config::GetConfig(KEY_AUDIO_FOLDER)+"/"+sListEntry+"\"";
			syslog(LOG_INFO, "remove file [%s]", cmd.c_str());
			cmd::exec(cmd);
			ReloadFiles(pMenu);
			pMenu->Reset();
			pMenu->print();
			break;
		default:
			break;
	}
}

void SetAudioFileListParameters(menuEntry* pMenuEntry)
{
	MenuEntryListAndOptions *pMenu = (MenuEntryListAndOptions *) pMenuEntry;
	MenuEntryListOptionHandler &rOptionHandler = pMenu->m_Handler;
	rOptionHandler.OnDisplay = OnDisplay;
	rOptionHandler.OnHide = OnHide;
	rOptionHandler.OnOptionSelection = OnOptionSelection;

	StringList sList;
	sList.push_back("*Play");
	sList.push_back("*Upload To Cloud");
	sList.push_back("*Copy To USB");
	sList.push_back("*Delete");

	pMenu->SetOptions(sList);
}
