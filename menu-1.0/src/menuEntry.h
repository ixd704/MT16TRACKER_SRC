#ifndef MenuEntry_h
#define MenuEntry_h 1

#include <string>
#include <list> 
#include "btns.h"
//#include "st7032.h"
//#include "lcd.h"
/*
typedef void (*MENU_ACTION_CALLBACK_FUNC)( char * pMenuText, void * pUserData );


//To use these functions, pass a function pointer as the argument to the MenuEntry constructor.
//pUserData should point to an unsigned int that will be set to true or false.
void MenuEntry_BoolTrueCallbackFunc( char * pMenuText, void * pUserData );
void MenuEntry_BoolFalseCallbackFunc( char * pMenuText, void * pUserData );

//Use this callback function for a "Back" menu item for hardware that doesn't include a back button
//pUserData should point to a MenfsuManager object.
void MenuEntry_BackCallbackFunc( char * pMenuText, void * pUserData );

//The MenuEntry class represents one menu item in the overall menu system, such as "Set Time" or "Back"
//The MenuEntry classes point to each other to create a tree of menu items.  You can navigate
// the classes using the get* calls.  MenuManager uses the get* calls to figure out what to draw to the LCD
*/


class menuEntry;
typedef void (*MENU_ENTRY_PRINT_HANDLER)(menuEntry* pMenu);

class menuEntry
{
	
  public:
  //Constructor to create each entry.
	  menuEntry(const std::string menuText){
		  _menuText = menuText;
		  _prev = NULL;
		  _next = NULL;
		  _parent = NULL;
		  _children = NULL;
		  _btn_mask = BTN_MASK_ALL;
	  }
	/*
	  menuEntry(const std::string menuText,  menuEntry* prev ){
			_menuText = menuText;
			_prev = prev;
	 	        _parent = NULL;
		        _children = NULL;
			
			prev->setNext(this);

			// actualisation prev for first element at menu loop
			for (menuEntry* p = prev; ; p = p->getPrev() ){
				if ((NULL == p->getPrev() ) || (prev == p->getPrev())){
					p->setPrev(this);
					_next = p;
					break;
				}
			}
	  }
	*/
	  menuEntry(const std::string menuText,  menuEntry* prev, menuEntry* parent ){
			_menuText = menuText;
			_prev = prev;
			_parent = parent;
			_children = NULL;
			_btn_mask = BTN_MASK_ALL;

			if (NULL == prev) 
				return;

			prev->setNext(this);

			// actualisation prev for first element at menu loop
			for (menuEntry* p = prev; ; p = p->getPrev() ){
				if ((NULL == p->getPrev() ) || (prev == p->getPrev())){
					p->setPrev(this);
					_next = p;
					break;
				}
			}
	  }


	std::string getMenuText(){
		return _menuText;
	}

	menuEntry* on(unsigned btn);
	virtual menuEntry* onUp();
	virtual menuEntry* onUpScroll();
	virtual menuEntry* onUpScrollOff();
	virtual menuEntry* onDown();
	virtual menuEntry* onDownScroll();
	virtual menuEntry* onDownScrollOff();
	virtual menuEntry* onLeft();
	virtual menuEntry* onLeftScroll();
	virtual menuEntry* onLeftScrollOff();
	virtual menuEntry* onRight();
	virtual menuEntry* onRightScroll();
	virtual menuEntry* onRightScrollOff();

	virtual bool isVisible();

	virtual void print();

/*
	  menuEntry(const std::string menuText,  menuEntry* prev, menuEntry* next ){
			_menuText = menuText;
			_prev = prev;
			_next = next;
			prev->setNext(this);
	  }

*/


//  menuEntry();

  //menuEntry(const std::string* menuText, const menuEntry * pParent);


  //add a child menu item.  They will be kept it the order they are added, from top to bottom.
  //bool addChild( menuEntry* child);
  void setNext(menuEntry* next){
	  _next = next;				
  }
  menuEntry* getNext(){
	  return _next;				
  }

  void setPrev(menuEntry* prev){
	  _prev =  prev;					
  }

  menuEntry* getPrev(){
	  return _prev;								
  }


  void setParent(menuEntry* parent){
	   _parent = parent;
  }

  menuEntry* getParent(){
	 return  _parent;								
  }

  void setChildren(menuEntry* children){
	   _children = children;
  }

  virtual menuEntry* getChildren(){
	  return _children;								
  }

  void SetButtonMask(unsigned nMask);


/*
  //Add a menu item as a sibling of this one, at the end of the sibling chain.
  bool addSibling( MenuEntry* sibling);
  //Sets the previous sibling, mostly used during menu creation to notify a new entry where it's
  //previous pointer needs to point.
  void setPrevSibling( MenuEntry* prevSibling);
  //Can set the action call back dynamically. Overrides what was passed to the constructor.
  bool addActionCallback( MENU_ACTION_CALLBACK_FUNC pCallback);
  
  char* getMenuText();
  //Sets the previous sibling, mostly used during menu creation to notify a new entry where it's
  //previous pointer needs to point.
  void setParent( MenuEntry* parent );
  
  MenuEntry *getNextSibling();
  MenuEntry *getPrevSibling();
  MenuEntry *getChild();
  MenuEntry *getParent();
  //This call will call the action callback for use when the menu item is selected.
  //if this menu entry has any children, the callback will not be executed.
  void ExecuteCallback();

  bool isBackEntry() { return (m_callback == MenuEntry_BackCallbackFunc); }
  
  */
  protected:
	std::string _menuText;

	menuEntry* _parent;
	menuEntry* _children;

//	std::list<menuEntry*> _childrens;

	menuEntry* _next;
	menuEntry* _prev;
	unsigned _btn_mask;

		
};



#endif
