#include <sstream>
#include "menuEntryString.h"
//#include "menuEntryStringEdit.h"
#include "lcd.h"
#include "config.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void menuEntryString::print(){
	std::stringstream sstm;
	sstm << this->getMenuText() << " : " ;
	std::string out = sstm.str();
	
	lcd::clear();	
	lcd::write(out.c_str(), out.size(),0,0);
	lcd::write(config::GetConfig(this->_config_key).c_str(), config::GetConfig(this->_config_key).size(), 1, 0);
}

menuEntryString::menuEntryString(const std::string menuText,  menuEntry* prev, menuEntry* parent, std::string config_key, bool isEditor, iAlphabet* pAlphabet)
		:menuEntry(menuText, prev, parent)
		,_config_key(config_key)
		,_isEditor(isEditor)
		,_alphabet(pAlphabet)
	{
	}



 menuEntry* menuEntryString::getChildren(){
	 if (!_isEditor)
		 return this;
	 
		if (NULL == this->_children )
					_children = new menuEntryStringEdit(_menuText, NULL, this, _config_key,_alphabet);
	  return _children;								
	  
}


 bool menuEntryString::isVisible(){
	 return config::isVisible(this->_config_key);
 }



 //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 menuEntryStringEdit::menuEntryStringEdit(const std::string menuText,  menuEntry* prev, menuEntry* parent, std::string config_key,iAlphabet* pAlphabet)
		:menuEntryString(menuText, prev, parent, config_key, false, pAlphabet)
	{
	}

menuEntryStringEdit::~menuEntryStringEdit(void)
{
}


void menuEntryStringEdit::print(){
	_str = config::GetConfig(this->_config_key);
	_pos = 0;
	
	lcd::clear();	
	lcd::write(getMenuText().c_str(), getMenuText().size(),0,0);
	lcd::write(" : ", 3, 0, getMenuText().size());
	lcd::write(_str.c_str(), _str.size(),1,0);
	lcd::set_cursor(true, 1, _pos);

}

menuEntry* menuEntryStringEdit::onUp(){
	lcd::set_cursor(false, 1, _pos);
	unsigned aIndex = 0;
	for (unsigned i = 0; i < _alphabet->getAlphabet().size(); i++){
		if (_str[_pos] == _alphabet->getAlphabet()[i]){
			aIndex = i;
			break;
		}
	}

	if (_alphabet->getAlphabet().size() - 1 == aIndex)
		aIndex = 0;
	else 
		aIndex++;

	_str[_pos] = _alphabet->getAlphabet()[aIndex];
	lcd::write(_str[_pos], 1, _pos);

	lcd::set_cursor(true, 1, _pos);
	return this;
}

menuEntry* menuEntryStringEdit::onDown(){
	lcd::set_cursor(false, 1, _pos);

	int aIndex = 0;
	for (unsigned i = 0; i < _alphabet->getAlphabet().size(); i++){
		if (_str[_pos] == _alphabet->getAlphabet()[i]){
			aIndex = i;
			break;
		}
	}

	if (0 == aIndex)
		aIndex = _alphabet->getAlphabet().size()-1 ;
	else 
		aIndex--;

	_str[_pos] = _alphabet->getAlphabet()[aIndex];
	lcd::write(_str[_pos], 1, _pos);

	lcd::set_cursor(true, 1, _pos);

	return this;
}



menuEntry* menuEntryStringEdit::onRight(){
	lcd::set_cursor(false, 1, _pos);

	if (_pos == 15)
		_pos = 0;
	else
		_pos++;
		
	lcd::set_cursor(true, 1, _pos);
	return this;
}


menuEntry* menuEntryStringEdit::onRightScroll(){
	return this;
}

menuEntry* menuEntryStringEdit::onRightScrollOff(){
	lcd::set_cursor(false, 1, _pos);

	config::SetConfig(this->_config_key, _str);

	this->getParent()->print();			  
	return this->getParent();
}


menuEntry* menuEntryStringEdit::onLeft(){
	lcd::set_cursor(false, 1, _pos);

	if (_pos == 0)
		_pos = 15;
	else
		_pos--;
		
	lcd::set_cursor(true, 1, _pos);
	return this;
}


menuEntry* menuEntryStringEdit::onLeftScroll(){
	return this;
}

menuEntry* menuEntryStringEdit::onLeftScrollOff(){
	lcd::set_cursor(false, 1, _pos);
	this->getParent()->print();			  
	return this->getParent();
}
