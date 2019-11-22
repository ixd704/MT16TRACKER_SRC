
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/slab.h>

//#include <mach/clock.h>
//#include <asm/mach-types.h>

//#include <mach/hardware.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/timer.h>
#include <linux/kfifo.h>
#include <linux/poll.h>
#include <linux/semaphore.h>
#include <linux/sched.h>

#define POLL_IDLE 100
#define REPEAT_START 15
#define REPEAT_PERIOD 3

static DECLARE_MUTEX(dev_lock);
static int reader_present;
static int writer_present;
static int open_count;
static uint8_t previous_button;
static uint8_t button_value=0xF;

#define MAX_OPEN_COUNT (2)
#define MODE_READER (1)
#define MODE_WRITER (2)
#define TRUE (1)
#define FALSE (0)



#define BTN_NONE		(0x00)

#define BTN_RIGHT		(0x0e)
#define BTN_LEFT		(0x0d)
#define BTN_UP			(0x0b)
#define BTN_DOWN		(0x07)





static void on_button_press(uint8_t button);




static dev_t first; // Global variable for the first device number
static struct cdev c_dev; // Global variable for the character device structure
static struct class *cl; // Global variable for the device class

static struct timer_list button_poll_timer;

struct kfifo *rx_fifo;

int debounce_count = 1;
uint8_t gpio_value[4]={1,1,1,1};
uint8_t gpio_stable[4]={0,0,0,0};
int last_press_time;

wait_queue_head_t inq,outq;

static int buttons_lcd_open(struct inode *i, struct file *f)
{
	int retval = 0;
	f->private_data = 0;
	down(&dev_lock);
	if (MAX_OPEN_COUNT == open_count) {
		retval = -EBUSY;
	} else {
		if ((f->f_flags & O_ACCMODE) == O_RDONLY) {
			if (reader_present) {
				retval = -EBUSY;
			} else {
				reader_present = TRUE;
				f->private_data = MODE_READER;
				open_count++;
			}
		} else if((f->f_flags & O_ACCMODE) == O_WRONLY) {
			if (writer_present) {
				retval = -EBUSY;
			} else {
				writer_present = TRUE;
				f->private_data = MODE_WRITER;
				open_count++;
			}

		} else {
			retval = -EINVAL;
		}
	}

	up(&dev_lock);
	return retval;
}
static int buttons_lcd_close(struct inode *i, struct file *f)
{
	down(&dev_lock);
	
	if (MODE_READER == (int) f->private_data) {
		reader_present = FALSE;
		open_count--;
	}

	if (MODE_WRITER == (int) f->private_data) {
		writer_present = FALSE;
		open_count--;
	}
	f->private_data=0;
	up(&dev_lock);
	return 0;
}
static unsigned int buttons_lcd_poll(struct file *f, poll_table *wait)
{
	//struct scull_pipe *dev = filp->private_data;
	unsigned int mask = 0;
	//printk(KERN_INFO "Driver: poll()\n");

	//down(&dev->sem);
	poll_wait(f, &inq, wait);
	poll_wait(f, &outq, wait);
	mask |= POLLOUT | POLLWRNORM; /* writable */
	if (__kfifo_len(rx_fifo)>0)
		mask |= POLLIN | POLLRDNORM; /* readable */
	//up(&dev->sem);
	return mask;
}
static ssize_t buttons_lcd_read(struct file *f, char __user *buf, size_t len, loff_t *off)
{
	char *rbuf;

	//printk(KERN_INFO "Driver: read()\n");

	if (MODE_READER != ((int)f->private_data)) {
		return -EPERM;
	}
	


	if (f->f_flags & O_NONBLOCK)
	{
		return -EAGAIN;
	}

	if (wait_event_interruptible(outq, __kfifo_len(rx_fifo)>0))
	{
		// signal interrupted the wait, return
		return -ERESTARTSYS;
	}

	rbuf = kmalloc(len, GFP_KERNEL);
	if (IS_ERR(rbuf))
	{
		return -1;
	}
	len = __kfifo_get(rx_fifo, rbuf, len);
	len = len - copy_to_user(buf, rbuf, len);
	kfree(rbuf);

	return len;
}

static ssize_t buttons_lcd_write(struct file *f, const char __user *buf, size_t len, loff_t *off)
{
	uint8_t button=0;

	if (MODE_WRITER != (int) f->private_data) {
		return -EPERM;
	}

	if (1 != len) {
		return -EINVAL;
	}
	
	if (copy_from_user(&button, buf, len)) {
		return -EFAULT;
	}

	button_value = button;
	//button_value = (~button) & 0x0F;
	
	//on_button_press(button);

	return len;
}

static struct file_operations buttons_lcd_fops = {
	.owner = THIS_MODULE,
	.open = buttons_lcd_open,
	.release = buttons_lcd_close,
	.read = buttons_lcd_read,
	.write = buttons_lcd_write,
	.poll = buttons_lcd_poll
};


static void on_button_press(uint8_t button)
{
	if (previous_button && button != previous_button) {
		uint8_t btn_none=0;
		__kfifo_put(rx_fifo,&btn_none,1);
	}
	__kfifo_put(rx_fifo,&button,1);
	previous_button=button;
	wake_up(&outq);
}




void button_poll_timer_callback( unsigned long data )
{
	int value,i,ret;
	int is_changed = 0;
	int poll_period;
	unsigned char buff;

	for (i=0;i<4;i++)
	{
		//value = gpio_get_value(gpioLD[i]);
		value = (button_value & (1 << i))?1:0;
		if (value != gpio_value[i])
		{
			gpio_stable[i]++;
		}
		if ((value == gpio_value[i])&&(gpio_stable[i]>0))
		{
			gpio_stable[i]--;
		}

		if (gpio_stable[i]>=debounce_count)
		{
			gpio_stable[i] = 0;
			gpio_value[i] = value;
			last_press_time = 0;
			//debug
			//if (value == 0) printk ("button %d pressed\n", i);
			//else printk ("button %d released\n", i);
			buff = (gpio_value[3]<<3)|(gpio_value[2]<<2)|(gpio_value[1]<<1)|gpio_value[0];
			
			//printk ("Buttons state %d%d%d%d buff=0x%X\n",gpio_value[3],gpio_value[2],gpio_value[1],gpio_value[0],buff);
			__kfifo_put(rx_fifo,&buff,1);
			wake_up(&outq);
		}
		is_changed += gpio_stable[i];
	}
	button_value = 0xF;
	if (is_changed>0)
	{
		poll_period = 20;
	}else{
		poll_period = POLL_IDLE;
		//repetition check
		for (i=0;i<4;i++)
		{
			if ((gpio_value[i]==0) && (last_press_time>REPEAT_START) && !(last_press_time%REPEAT_PERIOD))
			{
				buff=(gpio_value[3]<<3)|(gpio_value[2]<<2)|(gpio_value[1]<<1)|gpio_value[0];
				//printk ("Buttons repeat %d%d%d%d buff=0x%X\n",gpio_value[3],gpio_value[2],gpio_value[1],gpio_value[0],buff);
				__kfifo_put(rx_fifo,&buff,1);
				wake_up(&outq);
				break;
			}
		}
		last_press_time++;
	}
	ret = mod_timer( &button_poll_timer, jiffies + msecs_to_jiffies(poll_period) );
	if (ret) printk(KERN_ERR "Error in mod_timer\n");
}


static int __init buttons_lcd_init(void)
{
	int ret;

	// allocate and register device
	if (alloc_chrdev_region(&first, 0, 1, "buttons_lcd") < 0)
	{
		printk(KERN_ERR "Can't alloc chardev region\n");
		goto init_error;
	}
	if ((cl = class_create(THIS_MODULE, "chardrv")) == NULL)
	{
		printk(KERN_ERR "Can't create class\n");
		unregister_chrdev_region(first, 1);
		goto init_error;
	}
	if (device_create(cl, NULL, MKDEV(MAJOR(first), MINOR(first)), NULL, "buttons_lcd") == NULL)
	{
		printk(KERN_ERR "Can't create device\n");
		class_destroy(cl);
		unregister_chrdev_region(first, 11);
		goto init_error;
	}

	cdev_init(&c_dev, &buttons_lcd_fops);
	c_dev.owner=THIS_MODULE;
	if (cdev_add(&c_dev, first, 1) < 0)
	{
		printk(KERN_ERR "Can't add cdev\n");
		device_destroy(cl, MKDEV(MAJOR(first), MINOR(first)));
		class_destroy(cl);
		unregister_chrdev_region(first, 1);
		goto init_error;
	}
	//allocate fifo
	rx_fifo = kfifo_alloc(16,GFP_KERNEL, NULL);
	if (IS_ERR(rx_fifo))
	{
		printk(KERN_ERR "Can't alloc buffer\n");
		goto init_error1;
	}

	//init wait queues
	init_waitqueue_head(&inq);
	init_waitqueue_head(&outq);

	// set timer for buttons polling
	setup_timer( &button_poll_timer, button_poll_timer_callback, 0 );
	ret = mod_timer( &button_poll_timer, jiffies + msecs_to_jiffies(POLL_IDLE) );
	if (ret)
	{
		printk(KERN_ERR "Error in mod_timer\n");
		goto init_error2;
	}

	printk(KERN_INFO "tracker buttons emulator driver init done\n");
	return 0;

init_error2:
	kfifo_free(rx_fifo);
init_error1:
	cdev_del(&c_dev);
	device_destroy(cl, MKDEV(MAJOR(first), MINOR(first)));
	class_destroy(cl);
	unregister_chrdev_region(first, 1);
init_error:
	return -1;
}

static void __exit buttons_lcd_exit(void)
{
	del_timer(&button_poll_timer);
	cdev_del(&c_dev);
	device_destroy(cl, MKDEV(MAJOR(first), MINOR(first)));
	class_destroy(cl);
	unregister_chrdev_region(first, 1);
}

module_init(buttons_lcd_init);
module_exit(buttons_lcd_exit);

MODULE_LICENSE("GPL");
