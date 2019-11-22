/** ************************************************************************************
 *
 *          @file  MenuEntryListAndOptions.h
 *
 *    	   @brief  
 *
 *       @version  1.0
 *          @date  27.12.2013 14:51:31
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

#ifndef __MENU_ENTRY_LIST_AND_OPTIONS_H__
#define __MENU_ENTRY_LIST_AND_OPTIONS_H__

#define SET_NEXT_VALUE(var,max_val) \
	if (max_val) {					\
		var++;						\
		if (max_val == var) {		\
			var = 0;				\
		}							\
	}

#define SET_PREVIOUS_VALUE(var,max_val) \
	if (max_val) {						\
		if (var) {						\
			var--;						\
		} else {						\
			var = max_val -1;			\
		}								\
	}




typedef void (*MENU_ACTION_HANDLER)(menuEntry* pMenu);
typedef void (*MENU_OPTION_HANDLER)(menuEntry* pMenu, const std::string& sListEntry, const std::string& sOption);

typedef std::vector<std::string> StringList;

struct MenuEntryListOptionHandler
{
public:

	MENU_ACTION_HANDLER OnDisplay;
	MENU_ACTION_HANDLER OnHide;
	MENU_OPTION_HANDLER OnOptionSelection;
	MENU_ACTION_HANDLER OnOptionHide;
};


class MenuEntryListAndOptions: public menuEntry
{

public:
	MenuEntryListAndOptions(const std::string MenuText,
							menuEntry* prev,
							menuEntry* parent,
							StringList __list,
							StringList __options,
							MenuEntryListOptionHandler _Handler);

	MenuEntryListAndOptions(const std::string MenuText,
							menuEntry* prev,
							menuEntry* parent);
	
	void SetList(StringList& rList);
	StringList& GetList();
	void SetOptions(StringList& rOptions);
	bool SetIndexByKey(std::string& sKey);

	unsigned GetOptionIndex();

public:
	void Reset();
	virtual void print();

	virtual menuEntry* onUp();
	virtual menuEntry* onDown();
	virtual menuEntry* onLeft();
	virtual menuEntry* onRight();



public:
	MenuEntryListOptionHandler m_Handler;

protected:
	bool _menu_displayed;

	StringList _list;
	unsigned _list_index;
	unsigned _list_count;

	bool _options_displayed;
	StringList _options;
	unsigned _option_index;
	unsigned _option_count;

};


#endif

