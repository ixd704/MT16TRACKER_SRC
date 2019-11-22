/** ************************************************************************************
 *
 *          @file  ToggleMenuEntry.cpp
 *
 *    	   @brief
 *
 *       @version  1.0
 *          @date  28.12.2013 13:27:53
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
#include <lcd.h>
#include <menuEntry.h>
#include <ToggleMenuEntry.h>

ToggleMenuEntry::ToggleMenuEntry(const std::string MenuText,
					menuEntry* prev,
					menuEntry* parent,
					bool __toggled):
	menuEntry(MenuText, prev, parent),
	m_toggled(__toggled),
	m_bModelMenu(false)
{
	memset(&m_Handler, 0, sizeof(m_Handler));
}

ToggleEntryHandler& ToggleMenuEntry::GetHandler()
{
	return m_Handler;
}

bool ToggleMenuEntry::IsToggled()
{
	return m_toggled;
}

void ToggleMenuEntry::SetModelMenu(bool bModel)
{
	m_bModelMenu = bModel;
}

menuEntry* ToggleMenuEntry::onLeft()
{
	if (m_bModelMenu) {
		return this;
	} else {
		return menuEntry::onLeft();
	}
}

menuEntry* ToggleMenuEntry::onRight()
{
	m_toggled = !m_toggled;
	if (m_Handler.OnToggleStateChange) {
		m_Handler.OnToggleStateChange(this, m_toggled);
	}
	return this;
}

void ToggleMenuEntry::print()
{
	if (m_Handler.OnPrint) {
		m_Handler.OnPrint(this);
	} else {
		menuEntry::print();
	}
}
