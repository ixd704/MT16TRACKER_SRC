#include <sstream>
#include "menuEntryBoolean.h"
//#include "menuEntryBooleanEdit.h"
#include "lcd.h"
#include "config.h"
#include "audio.h"

menuEntryBoolean::~menuEntryBoolean(void)
{
}



void menuEntryBoolean::print(){
	std::stringstream sstm;
	sstm << this->getMenuText() << " : " << config::GetConfig(this->_config_key);
	std::string out = sstm.str();
	
	lcd::clear();	
	lcd::write(out.c_str(), out.size(),0,0);
}

menuEntryBoolean::menuEntryBoolean(const std::string menuText,  menuEntry* prev, menuEntry* parent, std::string config_key, bool isEditor)
		:menuEntry(menuText, prev, parent)
		,_config_key(config_key)
		,_isEditor(isEditor)
	{
	}

 menuEntry* menuEntryBoolean::getChildren(){
	 if (!_isEditor)
		 return this;

	 if (NULL == this->_children )
					_children = new menuEntryBooleanEdit(_menuText, NULL, this, _config_key);
	 return _children;								
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 menuEntryBooleanEdit::menuEntryBooleanEdit(const std::string menuText,  menuEntry* prev, menuEntry* parent, std::string config_key)
		:menuEntryBoolean(menuText, prev, parent,config_key,false)
	{
		_is_on = config::GetConfigOnOffAsBool(this->_config_key);
	}

menuEntryBooleanEdit::~menuEntryBooleanEdit(void)
{
}



void menuEntryBooleanEdit::print(){
	std::stringstream sstm;
	sstm << this->getMenuText() << " : " << config::GetConfig(this->_config_key);
	std::string out = sstm.str();
	

	_is_on = config::GetConfigOnOffAsBool(this->_config_key);
	print_star();

}


void menuEntryBooleanEdit::print_star(){
	lcd::clear();	
	lcd::write(this->getMenuText().c_str(), this->getMenuText().size(),0,0);
	lcd::write("on", 2,0, 13);
	lcd::write("off", 3,1, 13);
	
	lcd::write(_is_on?"*":" ", 1, 0,12);
	lcd::write(_is_on?" ":"*", 1, 1, 12);
}

menuEntry* menuEntryBooleanEdit::onUp(){
		  _is_on = !_is_on;
		  print_star();
		  return this;
	  }

menuEntry* menuEntryBooleanEdit::onDown(){
		  _is_on = !_is_on;
		  print_star();
		  return this;
	  }



menuEntry* menuEntryBooleanEdit::onRight(){
	if ((this->_config_key == KEY_WIFI) &&
		(config::GetConfigOnOffAsBool(this->_config_key) != _is_on)) {
		if (_is_on){ 
            syslog(LOG_INFO, "wifi enable");
            wifi::on(config::GetConfig(KEY_WIFI_SSID),config::GetConfig(KEY_WIFI_PASSWORD));
        } else {
            syslog(LOG_INFO, "wifi disable");
            wifi::off();
        }
    } else 	if ((this->_config_key == KEY_DHCP) &&
		(config::GetConfigOnOffAsBool(this->_config_key) != _is_on)) {
		if (_is_on){ 
            syslog(LOG_INFO, "dhcp enable");
            eth::initByDhcp();
        } else {
            syslog(LOG_INFO, "dhcp disable");
        }
    }

	config::SetConfig(this->_config_key, _is_on);

	this->getParent()->print();
	btns::reset();
	return this->getParent();
}

menuEntry* menuEntryBooleanEdit::onLeft()
{
	return menuEntry::onLeft();
}
