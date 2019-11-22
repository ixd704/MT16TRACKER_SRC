/** ************************************************************************************
 *
 *          @file  MenuEntryListAndOptions.cpp
 *
 *    	   @brief
 *
 *       @version  1.0
 *          @date  27.12.2013 14:54:57
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

#include <string>
#include <vector>
#include <lcd.h>
#include <menuEntry.h>
#include <MenuEntryListAndOptions.h>

MenuEntryListAndOptions::MenuEntryListAndOptions(const std::string MenuText,
							menuEntry* prev,
							menuEntry* parent,
							StringList __list,
							StringList __options,
							MenuEntryListOptionHandler _Handler):
					menuEntry(MenuText, prev, parent),
					m_Handler(_Handler),
					_menu_displayed(false),
					_list(__list),
					_options(__options)
{
	Reset();
}

MenuEntryListAndOptions::MenuEntryListAndOptions(const std::string MenuText,
							menuEntry* prev,
							menuEntry* parent):
					menuEntry(MenuText, prev, parent),
					_menu_displayed(false)
{
	memset(&m_Handler, 0, sizeof(m_Handler));
	Reset();
}

void MenuEntryListAndOptions::Reset()
{
	_list_index=0;
	_options_displayed=false;
	_option_index=0;
	_list_count = _list.size();
	_option_count = _options.size();

}

void MenuEntryListAndOptions::SetList(StringList& rList)
{
	_list = rList;
	Reset();
}

StringList& MenuEntryListAndOptions::GetList()
{
	return _list;
}


void MenuEntryListAndOptions::SetOptions(StringList& rOptions)
{
	_options = rOptions;
	Reset();
}

bool MenuEntryListAndOptions::SetIndexByKey(std::string& sKey)
{
	return false;
}

unsigned MenuEntryListAndOptions::GetOptionIndex()
{
	if (_options_displayed)
		return _option_index + 1;
	else
		return _option_index;
}

menuEntry* MenuEntryListAndOptions::onUp()
{
	if (_menu_displayed) {
		if (_options_displayed) {
			SET_PREVIOUS_VALUE(_option_index, _option_count)
		} else {
			SET_PREVIOUS_VALUE(_list_index, _list_count)
		}
		print();
		return this;
	} else {
		return menuEntry::onUp();
	}
}

menuEntry* MenuEntryListAndOptions::onDown()
{
	if (_menu_displayed) {
		if (_options_displayed) {
			SET_NEXT_VALUE(_option_index, _option_count)
		} else {
			SET_NEXT_VALUE(_list_index, _list_count)
		}
		print();
		return this;

	} else {
		return menuEntry::onDown();
	}

}

menuEntry* MenuEntryListAndOptions::onLeft()
{
	if (_menu_displayed) {
		if (_options_displayed) {
			_options_displayed = false;
			if (m_Handler.OnOptionHide) {
				m_Handler.OnOptionHide(this);
			}
		} else {
			_menu_displayed = false;
			if (m_Handler.OnHide) {
				m_Handler.OnHide(this);
			}
		}
		print();
		return this;
	} else {
		return menuEntry::onLeft();
	}

}

menuEntry* MenuEntryListAndOptions::onRight()
{
	if (_menu_displayed) {
		if (_options_displayed) {
			if (m_Handler.OnOptionSelection) {
				m_Handler.OnOptionSelection(this, _list[_list_index], _options[_option_index]);
			}
		} else if (_list_count && _option_count) {
			_options_displayed = true;
			print();
		}
	} else {
		_menu_displayed = true;
		if (m_Handler.OnDisplay) {
			m_Handler.OnDisplay(this);
		}
		print();
	}
	return this;
}

void MenuEntryListAndOptions::print()
{
	if (_menu_displayed) {
		lcd::clear();
		std::string sText;

		if (_options_displayed) {
			sText = _list[_list_index];
			lcd::write(sText.c_str(), sText.size(),0,0);
			sText = _options[_option_index];
		} else {
			sText = this->getMenuText() + ":";
			lcd::write(sText.c_str(), sText.size(),0,0);
			if (_list_count) {
				sText = _list[_list_index];
			} else {
				sText = "<empty>";
			}
		}
		lcd::write(sText.c_str(), sText.size(),1,0);
	} else {
		menuEntry::print();
	}
}

