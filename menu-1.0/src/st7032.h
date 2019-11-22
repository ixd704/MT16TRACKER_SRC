#ifndef __LCD_H
#define __LCD_H

#define I2C_DEV_NUM 0
//#define CMD
//#define LOCAL_AUDIO


int lcd_init(int file);

int i2c_lcd_dev_open();

/* write to lcd
file - fd of i2c device
buff - pointer to string to write
length - length of the string (without /n)
line_n - line number to write (0 or 1)
pos - position in the line to write (0 to 15)
*/
int lcd_write(int file, const char *buff, int length, int line_n, int pos);

int lcd_set_cursor(int file,int ena, int line_n, int pos);

int lcd_clear(int file);

#endif // __LCD_H
