/** ************************************************************************************
 *
 *          @file  ToggleMenuEntry.h
 *
 *    	   @brief
 *
 *       @version  1.0
 *          @date  28.12.2013 13:19:20
 *
 *        @author  Sudheer Divakaran
 *
 *		 @details
 *
 *\if NODOC
 *       Revision History:
 *				28.12.2013 - Sudheer Divakaran - Created
 *\endif
 ***************************************************************************************/

#ifndef __TOGGLE_MENU_ENTRY_H__
#define __TOGGLE_MENU_ENTRY_H__

class ToggleMenuEntry;

typedef void (*TOGGLE_ACTION_HANDLER)(ToggleMenuEntry* pMenu, bool bEnabled);

struct ToggleEntryHandler
{
public:

	TOGGLE_ACTION_HANDLER OnToggleStateChange;
	MENU_ENTRY_PRINT_HANDLER OnPrint;
};



class ToggleMenuEntry: public menuEntry
{
public:
	ToggleMenuEntry(const std::string MenuText,
					menuEntry* prev,
					menuEntry* parent,
					bool __toggled=false);

	ToggleEntryHandler& GetHandler();
	bool IsToggled();
	void SetModelMenu(bool bModel);

	virtual menuEntry* onLeft();
	virtual menuEntry* onRight();

	virtual void print();

protected:
	ToggleEntryHandler m_Handler;
	bool m_toggled;
	bool m_bModelMenu;
};


#endif
