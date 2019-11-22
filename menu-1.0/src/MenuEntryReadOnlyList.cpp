/** ************************************************************************************
 *
 *          @file  MenuEntryReadOnlyList.cpp
 *
 *    	   @brief  
 *
 *       @version  1.0
 *          @date  12.12.2013 11:30:41
 *
 *        @author  Sudheer Divakaran 
 *
 *		 @details
 *
 *\if NODOC
 *       Revision History:
 *				12.12.2013 - Sudheer Divakaran - Created
 *\endif
 ***************************************************************************************/

#include "menuEntryList.h"
#include "lcd.h"
#include "config.h"
#include <string>
#include <sstream>

MenuEntryReadOnlyList::MenuEntryReadOnlyList(const std::string menuText,
							menuEntry* prev,
							menuEntry* parent,
							std::string config_key,
							std::vector<std::string>& rList):
	menuEntry(menuText, prev, parent),
	_menu_active(false),
	_config_key(config_key),
	p_list(NULL),
	p_map(NULL),
	ListType(LIST)
{
	p_list= new std::vector<std::string>(rList);
	OnListModification();
}

MenuEntryReadOnlyList::MenuEntryReadOnlyList(const std::string menuText,
						menuEntry* prev,
						menuEntry* parent,
						std::string config_key,
						std::map<std::string, std::string>& rMap):
	menuEntry(menuText, prev, parent),
	_menu_active(false),
	_config_key(config_key),
	p_list(NULL),
	p_map(NULL),
	ListType(MAP)
{
	p_map= new std::map<std::string, std::string>(rMap);
	OnListModification();
}


MenuEntryReadOnlyList::~MenuEntryReadOnlyList()
{
	if (p_list) {
		delete p_list;
		p_list=NULL;
	}

	if (p_map) {
		delete p_map;
		p_map=NULL;
	}
}

void MenuEntryReadOnlyList::OnListModification()
{
	_selected_index = _display_index = 0;
	if (LIST == ListType) {
		_size = p_list->size();
	} else {
		_size = p_map->size();
	}
	SetIndexByKey(config::GetConfig(_config_key));
}

bool MenuEntryReadOnlyList::SetIndexByKey(std::string sKey)
{
	bool bResult = false;
	if (_size) {
		unsigned i;
		if (LIST == ListType) {
			for (i=0; i < _size; i++) {
				if ((*p_list)[_display_index] == sKey) {
					_selected_index = i;
					bResult = true;
					break;
				}
			}
		} else {
			std::map<std::string, std::string>::iterator iter;
			for (i=0, iter=p_map->begin(); iter!=p_map->end(); i++, iter++) {
				if (iter->first == sKey) {
					_selected_index = i;
					bResult = true;
					break;
				}
			}
		}
	}
	_display_index = _selected_index;
	return bResult;
}


void MenuEntryReadOnlyList::SetItems(std::vector<std::string>& rList)
{
	if (LIST == ListType) {
		*p_list	 = rList;
		OnListModification();
	}
}

void MenuEntryReadOnlyList::SetItems(std::map<std::string, std::string>& rMap)
{
	if (MAP == ListType) {
		*p_map = rMap;
		OnListModification();
	}
}

std::string MenuEntryReadOnlyList::GetSelectedText()
{
	std::string label;
	if (_size) {
		if (LIST == ListType) {
			label=(*p_list)[_display_index];
		} else {
			unsigned  i;
			std::map<std::string, std::string>::iterator iter;
			for (i=0, iter=p_map->begin(); iter!=p_map->end(); i++, iter++) {
				if (i == _display_index) {
					label = iter->second;
					break;
				}
			}
		}
	}
	return label;
}

std::string MenuEntryReadOnlyList::GetSelectedKey()
{
	std::string key;
	if (_size) {
		if (LIST == ListType) {
			key=(*p_list)[_selected_index];
		} else {
			unsigned  i;
			std::map<std::string, std::string>::iterator iter;
			for (i=0, iter=p_map->begin(); iter!=p_map->end(); i++, iter++) {
				if (i == _selected_index) {
					key = iter->first;
					break;
				}
			}
		}
	}
	return key;
}



void MenuEntryReadOnlyList::print()
{
	std::stringstream sstm;
	sstm << this->getMenuText() << ":" ;
	std::string label = sstm.str();
	
	lcd::clear();	
	lcd::write(label.c_str(), label.size(),0,0);
	
	sstm.str(std::string());
	sstm.clear();
	if (_menu_active) {
		sstm << "* ";
	} else {
		sstm << "  ";
	}
	sstm << GetSelectedText();
	label = sstm.str();
	lcd::write(label.c_str(), label.size(),1,0);
}

menuEntry* MenuEntryReadOnlyList::onUp()
{
	if (_menu_active) {
		if (_size) {
			_display_index--;
			if (_display_index >= _size) {
				_display_index = _size-1;
			}
			print();
		}

		return this;
	} else {
		return menuEntry::onUp();
	}
	
}

menuEntry* MenuEntryReadOnlyList::onDown()
{
	if (_menu_active) {
		if (_size) {
			_display_index++;
			if (_display_index >= _size) {
				_display_index = 0;
			}
			print();
		}

		return this;
	} else {
		return menuEntry::onDown();
	}
}
	
menuEntry* MenuEntryReadOnlyList::onRight()
{
	if (!_menu_active) {
		_menu_active = true;
		_display_index = _selected_index;
		print();
	}
	return this;
}
	
menuEntry* MenuEntryReadOnlyList::onRightScrollOff()
{
	_menu_active = false;
	_selected_index = _display_index;
	config::SetConfig(_config_key,GetSelectedKey());
	return this;
}
	
menuEntry* MenuEntryReadOnlyList::onLeft()
{
	if (_menu_active) {
		_menu_active = false;
		_display_index = _selected_index;
		print();
		return this;
	} else {
		return menuEntry::onLeft();
	}
}

