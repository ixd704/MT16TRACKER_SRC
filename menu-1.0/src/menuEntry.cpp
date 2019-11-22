#include "menuEntry.h"
#include "lcd.h"
#include "cmd.h"

void menuEntry::print(){
	lcd::clear();
	lcd::write(this->getMenuText().c_str(), this->getMenuText().size(),0,0);
}

bool menuEntry::isVisible(){
	return true;
}
 

menuEntry*  menuEntry::on(unsigned btn){
		if (!(_btn_mask & btn)) {
			return this;
		}
		switch(btn){
			case BTN_NONE:
			break;

			case BTN_LEFT:
				return this->onLeft();
			break;
			case BTN_LEFT_SCROLL:
				return this->onLeftScroll();
			break;
			case BTN_LEFT_SCROLL_OFF:
				return this->onLeftScrollOff();
			break;

			case BTN_RIGHT:
				return this->onRight();
			break;
			case BTN_RIGHT_SCROLL:
				return this->onRightScroll();
			break;
			case BTN_RIGHT_SCROLL_OFF:
				return this->onRightScrollOff();
			break;

			case BTN_DOWN:
				return this->onDown();
			break;
			case BTN_DOWN_SCROLL:
				return this->onDownScroll();
			break;
			case BTN_DOWN_SCROLL_OFF:
				return this->onDownScrollOff();
			break;

			case BTN_UP:
				return this->onUp();
			break;

			case BTN_UP_SCROLL:
				return this->onUpScroll();
			break;

			case BTN_UP_SCROLL_OFF:
				return this->onUpScrollOff();
			break;
			
			default:
				syslog(LOG_ERR,"Unknown btn %u\n", btn );
		}
	return this;
		
	}



menuEntry* menuEntry::onUp(){
	menuEntry* prev = this->getPrev();
	if (NULL == prev) 
		return this;
	if (!prev->isVisible())
		return prev->onUp();

	prev->print();
	
	return prev;
}

menuEntry* menuEntry::onUpScroll(){
	return onUp();
}
menuEntry* menuEntry::onUpScrollOff(){
	return this;
}


menuEntry* menuEntry::onDown(){
	menuEntry* next = this->getNext();
	if (NULL == next) 
		return this;
	if (!next->isVisible()) 
		return next->onDown();

	next->print();			  
	return next;
}

menuEntry* menuEntry::onDownScroll(){
	return onDown();
}

menuEntry* menuEntry::onDownScrollOff(){
	return this;
}



menuEntry* menuEntry::onLeft(){
	menuEntry* parent = this->getParent();
	if (NULL == parent) 
		return this;
	if (!parent->isVisible())
		return parent->onLeft();

	parent->print();			  
	return parent;
}

menuEntry* menuEntry::onLeftScroll(){
	return onLeft();
}
menuEntry* menuEntry::onLeftScrollOff(){
	return this;
}

menuEntry* menuEntry::onRight(){

    if ( _menuText == "Reboot")	{
		syslog(LOG_INFO,"reboot");
        cmd::reboot();
	}

	menuEntry* children = this->getChildren();
	if (NULL == children) 
		return this;
	if (!children->isVisible())
		return children->getChildren();

	children->print();			  
	return children;
}

menuEntry* menuEntry::onRightScroll(){
	return onRight();
}
menuEntry* menuEntry::onRightScrollOff(){			  
	return this;
}


void menuEntry::SetButtonMask(unsigned nMask)
{
	_btn_mask = nMask;
}




/*
menuEntry::menuEntry( const std::string * menuText, void * userData, MENU_ACTION_CALLBACK_FUNC func)
{
  m_menuText = strdup(menuText);
  m_userData = userData;
  m_nextSibling = NULL;
  m_prevSibling = NULL;
  m_child = NULL;  
  m_parent = NULL;
  m_callback = func;
}
*/

//menuEntry::menuEntry(){
	//_childrens = new std::list<menuEntry>();
//}

//menuEntry::menuEntry(const std::string* menuText){
	
	//_childrens = new std::list<menuEntry>();
//}
/*
  menuEntry::menuEntry(const std::string* menuText, const menuEntry * pParent);


  //add a child menu item.  They will be kept it the order they are added, from top to bottom.
  bool menuEntry::addChild( menuEntry* child);
*/
/*
bool menuEntry::addChild( menuEntry* child){
	this->_childrens.push_back(child);
	return false;
}
*/

/*
void MenuEntry::ExecuteCallback()
{
  if( m_callback != NULL )
  {
    m_callback(m_menuText, m_userData);
  }
}

bool MenuEntry::addChild(MenuEntry* child)
{
  child->setParent( this );
  if(m_child != NULL)
  {
    m_child->addSibling( child );
  }
  else
  {
    m_child = child;
  }
  return true;
}

bool MenuEntry::addSibling( MenuEntry* sibling)
{
  sibling->setParent( m_parent );
  if( m_nextSibling != NULL )
  {
    m_nextSibling->addSibling(sibling);
  }
  else
  {
    m_nextSibling = sibling;
    sibling->setPrevSibling( this );
  }
}

void MenuEntry::setPrevSibling( MenuEntry * pPrevSibling)
{
  m_prevSibling = pPrevSibling;
}

char * MenuEntry::getMenuText()
{
  return m_menuText;
}

MenuEntry *MenuEntry::getNextSibling()
{
  return m_nextSibling;
}
MenuEntry *MenuEntry::getPrevSibling()
{
  return m_prevSibling;
}
MenuEntry *MenuEntry::getChild()
{
  return m_child;
}
MenuEntry *MenuEntry::getParent()
{
  return m_parent;
}
void MenuEntry::setParent( MenuEntry * parent)
{
  m_parent = parent;
}
*/

/*
void MenuEntry_BoolTrueCallbackFunc( char * pMenuText, void * pUserData )
{
  *((unsigned int *)pUserData) = true;
}

void MenuEntry_BoolFalseCallbackFunc( char * pMenuText, void * pUserData )
{
  *((unsigned int *)pUserData) = false;
}

void MenuEntry_BackCallbackFunc( char * pMenuText, void * pUserData )
{
  ((MenuManager *)pUserData)->DoMenuAction( MENU_ACTION_BACK );
}

*/
