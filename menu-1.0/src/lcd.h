#ifndef LCD_H
#define LCD_H 1

#include "st7032.h"
#include <stdio.h>
#include <string.h>
#include <string>

using namespace std;

class lcd {


public:

	static void init(){
#ifdef CMD
#else
		fd = i2c_lcd_dev_open();
		lcd_init(fd);	
#endif
	}


	static void tick(){
		if (_i1 > 0){
			lcd_write(fd,_str1.c_str()+_i1,_str1.size()-_i1,1,_pos1);
			if (_str1.size()-_i1 < 15)
				_i1 = 0;
		}
	}


	static int write(const char *buff, int length, int line_n, int pos){
#ifdef CMD
		return printf("lcd (%d, %d) = %s\n",line_n, pos, buff);
#else
		if (length > 15 && line_n == 1 ){
			_str1 = std::string(buff);
			_i1 = 0;
			_pos1 = pos;

		} else {
			_i1 = -1;
		}
		return lcd_write(fd, buff, length, line_n, pos);
#endif
	}

	static int write(const char c, int line_n, int pos){
#ifdef CMD
		return printf("lcd (%d, %d) = %C\n",line_n, pos, c);
#else
		return lcd_write(fd, &c, 1, line_n, pos);
#endif
	}

	static int set_cursor(int ena, int line_n, int pos){
#ifdef CMD
		return printf("lcd (%d) set cursor pos - %d\n", line_n, pos);
#else
		return lcd_set_cursor(fd, ena, line_n, pos);
#endif
	}

	static int clear(){
#ifdef CMD
		return printf("lcd clear \n");		
#else
		return lcd_clear(fd);
#endif
	}

		static int clear(int line_n){
#ifdef CMD
		return printf("lcd clear line %d\n", line_n);		
#else
		return lcd_write(fd,"                ",16,line_n,0);
#endif
	}



	private:
		static int fd;
		static string _str1;
		static int _i1;
		static int _pos1;
};


#endif
