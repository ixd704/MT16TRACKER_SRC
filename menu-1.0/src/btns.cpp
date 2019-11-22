#include "btns.h"

int btns::fd = 0; // initializer
unsigned btns::prevBtn = BTN_NONE;
long btns::_on_start_ms = 0; 
bool btns::_is_freeze = false;


