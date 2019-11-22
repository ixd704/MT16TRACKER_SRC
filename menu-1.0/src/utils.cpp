/** ************************************************************************************
 *
 *          @file  utils.cpp
 *
 *    	   @brief  
 *
 *       @version  1.0
 *          @date  07.01.2014 03:05:04
 *
 *        @author  Sudheer Divakaran 
 *
 *		 @details
 *
 *\if NODOC
 *       Revision History:
 *				07.01.2014 - Sudheer Divakaran - Created
 *\endif
 ***************************************************************************************/

#include <string>
#include <utils.h>

void AddDirSeperator(std::string& path)
{
	if (path.empty())
		return;
	if (path.at(path.length() -1) != '/') {
		path.append("/");
	}
}

