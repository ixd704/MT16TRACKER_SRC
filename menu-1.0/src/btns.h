#ifndef BTNS_H
#define BTNS_H 1

#include <stdio.h>
#include <string.h>
#include "config.h"

#include <stdio.h>
#include <string.h>

#include <sys/stat.h>
#include <fcntl.h>
#ifndef _MSC_VER
	#include <unistd.h>
	#include <sys/time.h>
#endif

#ifdef USE_CURSES_LCD
#include <ncurses.h>
#endif


#define NAMEDPIPE_NAME "/dev/buttons_lcd"
#define BUFSIZE        10*1024

#define BTN_NONE				0x0
#define BTN_LEFT				0x01
#define BTN_LEFT_SCROLL			0x10
#define BTN_LEFT_SCROLL_OFF		0x20
#define BTN_RIGHT				0x02
#define BTN_RIGHT_SCROLL		0x40
#define BTN_RIGHT_SCROLL_OFF	0x80
#define BTN_UP					0x04
#define BTN_UP_SCROLL		    0x100
#define BTN_UP_SCROLL_OFF		0x200
#define BTN_DOWN				0x08
#define BTN_DOWN_SCROLL			0x400
#define BTN_DOWN_SCROLL_OFF		0x800

#define BTN_MASK_ALL			(0xFFF)

#ifdef _MSC_VER 
#define syslog(priority, ...)    printf(__VA_ARGS__)
#else
#include <syslog.h>
#endif

class btns {
	public:


	static void freeze(bool is_freeze){
		_is_freeze = is_freeze;		
	}

	static bool init(){
#ifndef _MSC_VER 		
	    if ( (fd = open(NAMEDPIPE_NAME, O_RDONLY)) <= 0 ) {
        	syslog(LOG_ERR,"Can't open named pipe for btns %s\n", NAMEDPIPE_NAME);
	        return false;
	    }
	    syslog(LOG_DEBUG,"%s is opened\n", NAMEDPIPE_NAME);

		//while(BTN_NONE != get_btn()){}
#endif
	    syslog (LOG_DEBUG,"Btns init from %s\n", "input");		
		return true;
	}
	
#ifndef _MSC_VER 		
	static long clock(){
		timeval time;
		gettimeofday(&time, NULL);
		return (time.tv_sec * 1000) + (time.tv_usec / 1000);
	}
#endif

	static void reset(){
#ifndef _MSC_VER 		
		// TODO:: очистка в случае переполнения буфера... но это нужнно еще и натыкать столько ;)
		char buf[10*1024];
		int len;

		memset(buf, '\0', 10*1024);
	        if ( (len = read(fd, buf, 10*1024-1)) <= 0 ) {
        	    syslog(LOG_ERR,"Can't read from btns named pipe\n");
	            close(fd);
		    fd = 0;
        	    remove(NAMEDPIPE_NAME);
	        }
#endif
	}
	
	


	static unsigned get_btn_by_cmd(){
		unsigned crntBtn = BTN_NONE;

		char str [3];
		int l  = scanf ("%s",str);   		
		if (0 != l) {
		switch(str[0]){
			case 'w':
				crntBtn = BTN_UP;	
			        break;
			case 's':
				crntBtn = BTN_DOWN;	
			        break;
			case 'a':
				crntBtn = BTN_LEFT;	
				if (str[1] == 'a')
					crntBtn = BTN_LEFT_SCROLL_OFF;	
			        break;
			case 'd':
				crntBtn = BTN_RIGHT;	
				if (str[1] == 'd')
					crntBtn = BTN_RIGHT_SCROLL_OFF;
			        break;

			}
		}
		syslog(LOG_INFO, "crnt_btn %hd",crntBtn);
		return  crntBtn;

	}


	static unsigned get_btn(){


		unsigned crntBtn = BTN_NONE;
#ifdef CMD
		return get_btn_by_cmd();
#else
		char buf[3];
		int len;

		memset(buf, '\0', 3);
		if ( (len = read(fd, buf, 2)) <= 0 ) {
			syslog(LOG_ERR,"Can't read from btns named pipe\n");
			close(fd);
			fd = 0;
			remove(NAMEDPIPE_NAME);
		}

		for (int i=0; i < len; i++){
			switch(buf[i]){
				case BTN_LEFT:
					if (prevBtn == BTN_LEFT || prevBtn == BTN_LEFT_SCROLL){
						syslog(LOG_DEBUG,"LEFT SCROLL - ON\n");
						prevBtn = BTN_LEFT_SCROLL;
						crntBtn = BTN_LEFT_SCROLL;	
					} else {
						syslog(LOG_DEBUG,"LEFT - ON\n");
						prevBtn = BTN_LEFT;
					}
					break;
				case BTN_RIGHT:
					if (prevBtn == BTN_RIGHT || prevBtn == BTN_RIGHT_SCROLL){
						syslog(LOG_DEBUG,"RIGHT SCROLL - ON\n");
						prevBtn = BTN_RIGHT_SCROLL;
						crntBtn = BTN_RIGHT_SCROLL;
					}
					else{
						syslog(LOG_DEBUG,"RIGHT - ON\n");
						prevBtn = BTN_RIGHT;
					}
					break;

				case BTN_UP:
					if (prevBtn == BTN_UP || prevBtn == BTN_UP_SCROLL){
						syslog(LOG_DEBUG,"UP SCROLL - ON\n");
						prevBtn = BTN_UP_SCROLL;
						crntBtn = BTN_UP_SCROLL;
					}
					else{
						syslog(LOG_DEBUG,"UP - ON\n");
						prevBtn = BTN_UP;
					}
					break;

				case BTN_DOWN:
					if (prevBtn == BTN_DOWN || prevBtn == BTN_DOWN_SCROLL){
						syslog(LOG_DEBUG,"DOWN SCROLL - ON\n");
						prevBtn = BTN_DOWN_SCROLL;
						crntBtn = BTN_DOWN_SCROLL;
					}
					else{
						syslog(LOG_DEBUG,"DOWN - ON\n");
						prevBtn = BTN_DOWN;
					}
					break;

				case BTN_NONE:
					switch (prevBtn){
						case BTN_LEFT:
							syslog(LOG_DEBUG,"LEFT - OFF\n");
							crntBtn = BTN_LEFT;
							break;
						case BTN_LEFT_SCROLL:
							syslog(LOG_DEBUG,"LEFT SCROLL - OFF\n");
							crntBtn = BTN_LEFT_SCROLL_OFF;
							break;

						case BTN_RIGHT:
							syslog(LOG_DEBUG,"RIGHT - OFF\n");
							crntBtn = BTN_RIGHT;
							break;
						case BTN_RIGHT_SCROLL:
							syslog(LOG_DEBUG,"RIGHT SCROLL - OFF\n");
							crntBtn = BTN_RIGHT_SCROLL_OFF;
							break;

						case BTN_UP:
							syslog(LOG_DEBUG,"UP - OFF\n");
							crntBtn = BTN_UP;
							break;
						case BTN_UP_SCROLL:
							syslog(LOG_DEBUG, "UP SCROLL - OFF\n");
							crntBtn = BTN_UP_SCROLL_OFF;
							break;
						case BTN_DOWN:
							syslog(LOG_DEBUG,"DOWN btn off\n");
							crntBtn = BTN_DOWN;
							break;
						case BTN_DOWN_SCROLL:
							syslog(LOG_DEBUG,"BTN_DOWN_SCROLL - OFF\n");
							crntBtn = BTN_DOWN_SCROLL_OFF;
							break;

						case BTN_NONE:
							break;
						default:
							syslog(LOG_ERR,"unknown btn %x\n", prevBtn);
							break;

					}
					prevBtn = BTN_NONE;
					break;
				default:
					syslog(LOG_ERR,"unknown btn %x\n", buf[i]);
					break;

			}
		}
#endif
		if (_is_freeze)
			return BTN_NONE;

		return crntBtn;
	}

#ifdef USE_CURSES_LCD
	static void queue_keyboard_button_events()
	{
		int ch;
		uint8_t button;
		int fd_queue;

		if ( (fd_queue = open(NAMEDPIPE_NAME, O_WRONLY)) <= 0 ) {
			syslog(LOG_ERR,"Can't open named pipe %s for writing\n", NAMEDPIPE_NAME);
			return;
		}
		for(;;) {
			ch = getch();
			button = 0;
			switch (ch) {
				case KEY_LEFT:
					button = BTN_LEFT;
					break;
				case KEY_RIGHT:
					button = BTN_RIGHT;
					break;
				case KEY_UP:
					button = BTN_UP;
					break;
				case KEY_DOWN:
					button = BTN_DOWN;
					break;
				default:
					break;
			}

			if (button) {
				int unused __attribute__((unused));
				unused = write(fd_queue, &button, 1);
			}

		}

		close(fd_queue);
	}
#endif

	private:
		static int fd;
		static unsigned prevBtn;
		static long _on_start_ms; 
		static bool _is_freeze;
	
};


#endif
