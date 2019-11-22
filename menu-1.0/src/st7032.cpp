#include <stdio.h>
#include <unistd.h>

#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include "st7032.h"

#ifdef USE_CURSES_LCD
#include <ncurses.h>

WINDOW *p_win_lcd;

int curses_lcd_init(int file);

int curses_lcd_write(int file, const char *buff, int length, int line_n, int pos);

int curses_lcd_set_cursor(int file,int ena, int line_n, int pos);

int curses_lcd_clear(int file);

#endif


// lcd initialization
int lcd_init(int file)
{
	unsigned char reg[2];

#ifdef USE_CURSES_LCD
	curses_lcd_init(file);
#endif

	reg[0]=0x00;
	reg[1]=0x38;
	if (write(file, &reg, 2) <0) {
		return -1;
		printf("write error\n");
	}
	usleep(30);

	reg[1]=0x39;
	if (write(file, &reg, 2) <0) {
		return -1;
		printf("write error\n");
	}
	usleep(30);

	reg[1]=0x14;
	if (write(file, &reg, 2) <0) {
		return -1;
		printf("write error\n");
	}
	usleep(30);

	reg[1]=0x78;
	if (write(file, &reg, 2) <0) {
		return -1;
		printf("write error\n");
	}
	usleep(30);

	reg[1]=0x5E;
	if (write(file, &reg, 2) <0) {
		return -1;
		printf("write error\n");
	}
	usleep(30);

	reg[1]=0x6A;
	if (write(file, &reg, 2) <0) {
		return -1;
		printf("write error\n");
	}
	usleep(200000);
//set display on and cursor off
	reg[1]=0x0C;
	if (write(file, &reg, 2) <0) {
		return -1;
		printf("write error\n");
	}
	usleep(30);

	reg[1]=0x01;
	if (write(file, &reg, 2) <0) {
		return -1;
		printf("write error\n");
	}
	usleep(2000);

	reg[1]=0x06;
	if (write(file, &reg, 2) <0) {
		return -1;
		printf("write error\n");
	}
	usleep(30);

	return 0;
}

int i2c_lcd_dev_open()
{
	int file, addr;
	char filename[20];

	snprintf(filename, 19, "/dev/i2c-%d", I2C_DEV_NUM);
	file = open(filename, O_RDWR);
	if (file < 0) {
		//printf ("open error\n");
		return -1;
	}
	addr = 0x3E; // The I2C address
	if (ioctl(file, I2C_SLAVE, addr) < 0) {
		//printf ("slave addr set error\n");
		close(file);
		return -1;
	}
	return file;
}

/* write to lcd
file - fd of i2c device
buff - pointer to string to write
length - length of the string (without /n)
line_n - line number to write (0 or 1)
pos - position in the line to write (0 to 15)
*/
int lcd_write(int file, const char *buff, int length, int line_n, int pos)
{
	int i;
	unsigned char reg[2];

#ifdef USE_CURSES_LCD
	curses_lcd_write(file, buff, length, line_n, pos);
#endif
	if (length<=0 || line_n<0 || line_n>1 || pos<0 || pos>15)
	{
		printf ("Bad parameter\n");
		return -1;
	}
	//set ram addr
	reg[0]=0x00;
	reg[1]=0x80+0x40*line_n+pos;
	if (write(file, &reg, 2)<0) return -1;
	usleep(30);
	reg[0]=0x40;
	for(i=0; i<=15-pos; i++)
	{
		if (i>=length) break; //end of string reached
		reg[1]=buff[i];
		if (write(file, &reg, 2)<0) return -1;
		usleep(30);
	}
	return 0;
}

int lcd_set_cursor(int file,int ena, int line_n, int pos)
{
	unsigned char reg[2];

#ifdef USE_CURSES_LCD
	curses_lcd_set_cursor(file, ena, line_n, pos);
#endif
	reg[0]=0x00;
	//toggle cursor on/off
	if (ena) {
		reg[1]=0x0F;
	} else reg[1]=0x0C;
	if (write(file, &reg, 2)<0) return -1;
	usleep(30);
	if (!ena) return 0;

	if (line_n<0 || line_n>1 || pos<0 || pos>15)
	{
		printf ("Bad parameter\n");
		return -1;
	}
	//set ram addr
	reg[0]=0x00;
	reg[1]=0x80+0x40*line_n+pos;
	if (write(file, &reg, 2)<0) return -1;
	usleep(30);
	return 0;
}

int lcd_clear(int file)
{
	unsigned char reg[2];

#ifdef USE_CURSES_LCD
	curses_lcd_clear(file);
#endif
	reg[0]=0x00;
	reg[1]=0x01;
	if (write(file, &reg, 2)<0) return -1;
	usleep(2000);
	return 0;
}

#ifdef USE_CURSES_LCD
extern int keyboard_buttons_emulation;
extern void start_keyboard_thread();

int curses_lcd_init(int file)
{
	initscr();

	cbreak();
	noecho();
	nodelay(stdscr, FALSE);
	keypad(stdscr, TRUE);
	if(keyboard_buttons_emulation) {
		printw("Button Press Emulator [Use LEFT,RIGHT,UP,DOWN arrow keys for button press emulation]");
		start_keyboard_thread();
	}
	
	refresh();
	p_win_lcd = newwin(4,20,1,0);
	box(p_win_lcd,0,0);
	wrefresh(p_win_lcd);

	return 0;
}


int curses_lcd_write(int file, const char *buff, int length, int line_n, int pos)
{
	if (line_n<0 || line_n>1 || pos<0 || pos>15 || length < 1)
	{
		return -1;
	}

	wmove(p_win_lcd, line_n+1, pos+1);
	for(int i=0; i<=15-pos; i++)
	{
		if (i>=length) break;
		waddch(p_win_lcd,buff[i]);
	}


	wrefresh(p_win_lcd);
	return 0;
}

int curses_lcd_set_cursor(int file,int ena, int line_n, int pos)
{	
	if (line_n<0 || line_n>1 || pos<0 || pos>15)
	{
		return -1;
	}

	if (ena)
		curs_set(1);
	else
		curs_set(0);
	
	wmove(p_win_lcd,line_n+1, pos+1);
	wrefresh(p_win_lcd);
	return 0;
}

int curses_lcd_clear(int file)
{
	wclear(p_win_lcd);
	box(p_win_lcd,0,0);
	wrefresh(p_win_lcd);
	return 0;
}
#endif
