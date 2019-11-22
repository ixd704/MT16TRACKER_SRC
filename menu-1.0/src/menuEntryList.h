#ifndef MenuEntryList_h
#define MenuEntryList_h 1

#include <string>
#include "lcd.h"
#include "menuEntry.h"
#include <vector>
#include <set>
#include "wifi.h"

#ifdef _MSC_VER 
#ifndef usleep
#  include <stdlib.h>
#  define usleep(t) _sleep((t) / 1000)	
#endif
#endif


class iList {
public:
	virtual std::vector<std::string> getList() =0 ;
};

class listCompress: public iList{
public:
	virtual std::vector<std::string> getList(){

		std::vector<std::string> out;
		out.push_back("FLAC");
		out.push_back("ALAC");
		out.push_back("MP3");
		out.push_back("OFF");

		return out;
	}
};


class listWifiSsid: public iList{
	std::vector<std::string> _list;
public:
	listWifiSsid(){
		wifi::up();
		_list = wifi::scan();

	}
	virtual std::vector<std::string> getList(){
		return _list;
	}
};

#ifndef _MSC_VER
#include <dirent.h>
#endif

class listFiles: public iList{
public:
	virtual std::vector<std::string> getList(){
	
		std::vector<std::string> out;
		//out.push_back("all+");
		//out.push_back("all-");
		
#ifdef _MSC_VER
		out.push_back("D13-03-20_T17-22-59_Red.wav");
		out.push_back("D05-01-17_T07-12-42_Blue.wav");
		out.push_back("D22-05-18_T02-12-23_Yellow.wav");
		out.push_back("D17-03-11_T05-13-14_Red.wav");
		out.push_back("D01-06-12_T09-14-23_Green.wav");
		out.push_back("D03-01-17_T10-15-43_Yellow.wav");
#else

		DIR *dir;
		struct dirent *ent;
		if ((dir = opendir ("/mnt/floppy/audio")) != NULL) {
		/* print all the files and directories within directory */
		while ((ent = readdir (dir)) != NULL) {
			if (strlen(ent->d_name) > 3) {
				out.push_back (ent->d_name);
			}
		}
		closedir (dir);
		} else {
		syslog(LOG_ERR,"could not open directory");
		}

#endif


		return out;
	}
};

class listChannels: public iList{
public:
	virtual std::vector<std::string> getList(){

		std::vector<std::string> out;
		out.push_back("1");
		out.push_back("2");
		out.push_back("3");
		out.push_back("4");
		out.push_back("5");
		out.push_back("6");
		out.push_back("7");
		out.push_back("8");
		out.push_back("9");
		out.push_back("10");
		out.push_back("11");
		out.push_back("12");
		out.push_back("13");
		out.push_back("14");
		out.push_back("15");
		out.push_back("16");

		return out;
	}
};

/*
class alphabetPassword: public iAlphabet{
public:
	virtual std::vector<char> getAlphabet(){
		std::vector<char> out;
		out.push_back(' ');
		for (int i=0; i < 26; i++) {
			out.push_back((char)('A' + i));
			out.push_back((char)('a' + i));
		}
		for (int i=0; i < 10; i++) {
			out.push_back((char)('0' + i));
		}
		for (int i=0; i < 14; i++) {
			out.push_back((char)('!' + i));
		}
		for (int i=0; i < 7; i++) {
			out.push_back((char)(':' + i));
		}
		return out;
	}
};

*/

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class menuEntryList :
	public menuEntry
{
public:
	menuEntryList(const std::string menuText,  menuEntry* prev, menuEntry* parent, std::string config_key,bool isEditor, iList* pList);

	virtual void print();
	virtual menuEntry* getChildren();
	virtual bool isVisible();

protected:
	std::string _config_key;
	bool		_isEditor;
	iList*		_list;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class menuEntryListEdit :
	public menuEntryList
{
public:
	menuEntryListEdit(const std::string menuText,  menuEntry* prev, menuEntry* parent, std::string config_key, iList* pList);

	virtual void print();

	virtual menuEntry* onUp();
	virtual menuEntry* onDown();
	virtual menuEntry* onRight();
	virtual menuEntry* onRightScroll();
	virtual menuEntry* onRightScrollOff();
	virtual menuEntry* onLeft();
	virtual menuEntry* onLeftScroll();
	virtual menuEntry* onLeftScrollOff();


protected:
	std::string _str;
	//int _pos;
	//iAlphabet* pAlphabet;
};



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class menuEntryCompress:
	public menuEntryList
{
public:
	menuEntryCompress(const std::string menuText,  menuEntry* prev, menuEntry* parent, std::string config_key, bool isEditor)
		:menuEntryList(menuText, prev, parent, config_key, isEditor, new listCompress())
	{}

};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///class menuEntryChannelsEdit;

class menuEntryChannels:
	public menuEntryList
{
public:
	menuEntryChannels(const std::string menuText,  menuEntry* prev, menuEntry* parent, std::string config_key, bool isEditor)
		:menuEntryList(menuText, prev, parent, config_key, isEditor, new listChannels())
	{}

private:


	virtual menuEntry* getChildren();
};


class menuEntryChannelsEdit :
	public menuEntryListEdit
{
public:
	menuEntryChannelsEdit(const std::string menuText,  menuEntry* prev, menuEntry* parent, std::string config_key, iList* pList);

	virtual void print();
	virtual void printState();

	virtual menuEntry* onUp();
	virtual menuEntry* onDown();
	virtual menuEntry* onRight();
	virtual menuEntry* onRightScroll();
	virtual menuEntry* onRightScrollOff();
	virtual menuEntry* onLeft();
	virtual menuEntry* onLeftScroll();
	virtual menuEntry* onLeftScrollOff();


protected:
	std::string				_state;
	unsigned				_i;
	std::set<std::string>	_activeChannels;
    std::map<std::string, std::string> _channels;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class menuEntryWifiSsid:
	public menuEntryList
{
public:
	menuEntryWifiSsid(const std::string menuText,  menuEntry* prev, menuEntry* parent, std::string config_key, bool isEditor)
		:menuEntryList(menuText, prev, parent, config_key, isEditor, NULL)
	{}

	 virtual menuEntry* getChildren(){
		if (!_isEditor)
			return this;
	 
		btns::freeze(true);
		lcd::clear();
		lcd::write("Scaning WiFi...", 12,0,0);
		if (NULL == this->_children )
			_children = new menuEntryListEdit(_menuText, NULL, this, _config_key, new listWifiSsid());
				
		lcd::clear();
		btns::freeze(false);
		
		lcd::clear();
		_children->print();

	  return _children;								
	 }
	  
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class MenuEntryReadOnlyList :
	public menuEntry
{
private:
	enum _ListType
	{
		LIST=0,
		MAP=1
	};

private:
	bool _menu_active;
protected:
	std::string _config_key;
	std::vector<std::string> *p_list;
	std::map<std::string, std::string> *p_map;
	unsigned _selected_index,_display_index;
	unsigned _size;
	_ListType ListType;

protected:
	void OnListModification();
	std::string GetSelectedText();
	std::string GetSelectedKey();

public:
	MenuEntryReadOnlyList(const std::string menuText,
							menuEntry* prev,
							menuEntry* parent,
							std::string config_key,
							std::vector<std::string>& rList);

	MenuEntryReadOnlyList(const std::string menuText,
						menuEntry* prev,
						menuEntry* parent,
						std::string config_key,
						std::map<std::string, std::string>& rMap);

	~MenuEntryReadOnlyList();
	void SetItems(std::vector<std::string>& rList);
	void SetItems(std::map<std::string, std::string>& rMap);

	bool SetIndexByKey(std::string sKey);

	virtual void print();

	virtual menuEntry* onUp();
	virtual menuEntry* onDown();
	virtual menuEntry* onRight();
	virtual menuEntry* onRightScrollOff();
	virtual menuEntry* onLeft();

};
#endif
