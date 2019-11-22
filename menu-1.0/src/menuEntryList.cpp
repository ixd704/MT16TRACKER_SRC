#include <sstream>
#include "menuEntryList.h"
#include "lcd.h"
#include "config.h"
#include <string>

#include <NetworkServiceRpc.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void menuEntryList::print(){
	std::stringstream sstm;
	sstm << this->getMenuText() << " : " ;
	std::string out = sstm.str();
	
	lcd::clear();	
	lcd::write(out.c_str(), out.size(),0,0);
	lcd::write(config::GetConfig(this->_config_key).c_str(), config::GetConfig(this->_config_key).size(), 1, 0);
}

menuEntryList::menuEntryList(const std::string menuText,  menuEntry* prev, menuEntry* parent, std::string config_key, bool isEditor, iList* pList)
		:menuEntry(menuText, prev, parent)
		,_config_key(config_key)
		,_isEditor(isEditor)
		,_list(pList)
	{
	}



 menuEntry* menuEntryList::getChildren(){
	 if (!_isEditor)
		 return this;
	 
		if (NULL == this->_children )
					_children = new menuEntryListEdit(_menuText, NULL, this, _config_key,_list);
	  return _children;								
	  
}


 bool menuEntryList::isVisible(){
	 return config::isVisible(this->_config_key);
 }



 //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 menuEntryListEdit::menuEntryListEdit(const std::string menuText,  menuEntry* prev, menuEntry* parent, std::string config_key,iList* pList)
		:menuEntryList(menuText, prev, parent, config_key, false, pList)
	{
	}



void menuEntryListEdit::print(){
	_str	= config::GetConfig(this->_config_key);
	
	lcd::clear();	
	lcd::write(getMenuText().c_str(), getMenuText().size(),0,0);
	lcd::write(" : ", 3, 0, getMenuText().size());
	lcd::write(" * ", 3, 1, 0);
	lcd::write(_str.c_str(), _str.size(),1,3);

}




menuEntry* menuEntryListEdit::onUp(){
	int nListSize;
	unsigned aIndex = 0;
	for (unsigned i = 0; i < _list->getList().size(); i++){
		if (_str == _list->getList()[i]){
			aIndex = i;
			break;
		}
	}

	nListSize = _list->getList().size();

	if (nListSize) {
	if (_list->getList().size() - 1 == aIndex)
		aIndex = 0;
	else 
		aIndex++;

	_str = _list->getList()[aIndex];
	}

	if (_str == config::GetConfig(this->_config_key))
			lcd::write(" * ", 3, 1, 0);
		else
			lcd::write("   ", 3, 1, 0);

	
	lcd::write(_str.c_str(), _str.size(),1,3);
	return this;
}

menuEntry* menuEntryListEdit::onDown(){
	int aIndex = 0;	
	for (unsigned i = 0; i < _list->getList().size(); i++){
		if (_str == _list->getList()[i]){
			aIndex = i;
			break;
		}
	}

	if ( 0 == aIndex)
		aIndex = _list->getList().size() - 1;
	else 
		aIndex--;

	_str = _list->getList()[aIndex];

	if (_str == config::GetConfig(this->_config_key))
			lcd::write(" * ", 3, 1, 0);
		else
			lcd::write("   ", 3, 1, 0);

	
	lcd::write(_str.c_str(), _str.size(),1,3);
	return this;
}



menuEntry* menuEntryListEdit::onRight(){
	config::SetConfig(this->_config_key, _str);

	this->getParent()->print();			  
	return this->getParent();

}


menuEntry* menuEntryListEdit::onRightScroll(){
	return this;
}

menuEntry* menuEntryListEdit::onRightScrollOff(){
	config::SetConfig(this->_config_key, _str);

	this->getParent()->print();			  
	return this->getParent();
}


menuEntry* menuEntryListEdit::onLeft(){
	this->getParent()->print();			  
	return this->getParent();
}


menuEntry* menuEntryListEdit::onLeftScroll(){
	return this;
}

menuEntry* menuEntryListEdit::onLeftScrollOff(){

	this->getParent()->print();			  
	return this->getParent();
}




//menuEntryChannels
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	menuEntry* menuEntryChannels::getChildren(){
		if (!_isEditor)
			return this;

		if (NULL == this->_children )
			_children = new menuEntryChannelsEdit(_menuText, NULL, this, _config_key, _list);

	  return _children;								
	}




	menuEntryChannelsEdit::menuEntryChannelsEdit(const std::string menuText,  menuEntry* prev, menuEntry* parent, std::string config_key,iList* pList)
		:menuEntryListEdit(menuText, prev, parent, config_key, pList)
	{
	}




/////Edit

void menuEntryChannelsEdit::print(){
	
	_i		       = 0;
	_str	       = _list->getList()[_i];
    _channels      = get_channels_map();
	


	lcd::clear();	
	lcd::write(getMenuText().c_str(), getMenuText().size(),0,0);
	lcd::write(" : ", 3, 0, getMenuText().size());
	printState();

}

void menuEntryChannelsEdit::printState(){
	lcd::clear(1);

	_state 	       =  _channels[_str].empty()?"off":_channels[_str];

	lcd::write(_str.c_str(), _str.size(),1,1);
	lcd::write(":", 1,1,3);
	lcd::write(_state.c_str(), _state.size(), 1, 5);

}



menuEntry* menuEntryChannelsEdit::onUp(){

	if (_list->getList().size() - 1 == _i)
		_i = 0;
	else 
		_i++;

	_str	= _list->getList()[_i];

	printState();
	return this;
}

menuEntry* menuEntryChannelsEdit::onDown(){
	if (0 == _i)
		_i = _list->getList().size() - 1;
	else 
		_i--;

	_str = _list->getList()[_i];

	printState();
	return this;
}



menuEntry* menuEntryChannelsEdit::onRight(){

	if (_state == "off"){
		_channels[_str] = "on";
	} else if (_state == "on"){
		_channels[_str] = "off";
	}

	printState();
    return this;
}


menuEntry* menuEntryChannelsEdit::onRightScroll(){
	return this;
}

menuEntry* menuEntryChannelsEdit::onRightScrollOff(){
	config::setChannels(_channels);
	this->getParent()->print();			  
	return this->getParent();
}


menuEntry* menuEntryChannelsEdit::onLeft(){
	if (_state == "off"){
		_channels[_str] = "on";
	} else if (_state == "on"){
		_channels[_str] = "off";
	}
	printState();

	return this;
}


menuEntry* menuEntryChannelsEdit::onLeftScroll(){
	return this;
}

menuEntry* menuEntryChannelsEdit::onLeftScrollOff(){

	this->getParent()->print();			  
	return this->getParent();
}


//////////////////////////////////////////////////////////////////////////////
//menuFiles

//menuEntryChannels
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

