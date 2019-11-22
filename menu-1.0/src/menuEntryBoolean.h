#ifndef MenuEntryBoolean_h
#define MenuEntryBoolean_h 1

#include <string>
#include "menuEntry.h"


class menuEntryBoolean :
	public menuEntry
{
public:
	menuEntryBoolean(const std::string menuText,  menuEntry* prev, menuEntry* parent, std::string config_key, bool isEditor);
	~menuEntryBoolean(void);

	void print();
	virtual menuEntry* getChildren();

protected:
	std::string _config_key;
	bool _isEditor;
};



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class menuEntryBooleanEdit :
	public menuEntryBoolean
{
public:
	menuEntryBooleanEdit(const std::string menuText,  menuEntry* prev, menuEntry* parent, std::string config_key);
	~menuEntryBooleanEdit(void);

	void print();
	void print_star();


	virtual menuEntry* onUp();
	virtual menuEntry* onDown();
	virtual menuEntry* onRight();
	virtual menuEntry* onLeft();



private:
	bool _is_on;
	bool _model_ui;
};



#endif
