#ifndef __MENU_H
#define __MENU_H

#define         LCD_ROWS        2
#define         LCD_COLS        16

class menuEntry;

class Menu
{
private:
	static void Initialize();
	static void EnableRemoteControl(bool bRemoteControl);

public:
	static void StartService();
	static void OnNetworkServiceStatusChanged(const Tracker__NetworkServiceStatus *pStatus);
	static void SetRecordingStatus(bool bStatus);
	static bool IsRecording();


private:
	static menuEntry* m_pActiveMenuEntry;
	static bool m_bRemoteControlled;
	static bool m_bRecordingStatus;

public:
	static int argc;
	static char **argv;
};



#endif // __MENU_H
