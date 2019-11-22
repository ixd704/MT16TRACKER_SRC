#ifndef MenuEntryString_h
#define MenuEntryString_h 1

#include <string>
#include "menuEntry.h"
#include <vector>


class iAlphabet {
public:
	virtual std::vector<char> getAlphabet() =0 ;
};

class alphabetIp: public iAlphabet{
public:
	virtual std::vector<char> getAlphabet(){
		std::vector<char> out;
		out.push_back(' ');
		for (int i=0; i < 10; i++) {
			out.push_back((char)('0' + i));
   		}
		out.push_back('.');
		return out;
	}
};

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



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class menuEntryString :
	public menuEntry
{
public:
	menuEntryString(const std::string menuText,  menuEntry* prev, menuEntry* parent, std::string config_key,bool isEditor, iAlphabet* pAlphabet);

	void print();
	virtual menuEntry* getChildren();
	virtual bool isVisible();

protected:
	std::string _config_key;
	bool _isEditor;
	iAlphabet* _alphabet;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class menuEntryStringEdit :
	public menuEntryString
{
public:
	menuEntryStringEdit(const std::string menuText,  menuEntry* prev, menuEntry* parent, std::string config_key, iAlphabet* pAlphabet);
	~menuEntryStringEdit(void);

	void print();

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
	int _pos;
	iAlphabet* pAlphabet;
};



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class menuEntryPassword:
	public menuEntryString
{
public:
	menuEntryPassword(const std::string menuText,  menuEntry* prev, menuEntry* parent, std::string config_key, bool isEditor)
		:menuEntryString(menuText, prev, parent, config_key, isEditor, new alphabetPassword())
	{}

};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class menuEntryIp:
	public menuEntryString
{
public:
	menuEntryIp(const std::string menuText,  menuEntry* prev, menuEntry* parent, std::string config_key, bool isEditor)
		:menuEntryString(menuText, prev, parent, config_key, isEditor, new alphabetIp())
	{}

};






#endif
