#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>


extern void mxc_mmc_force_detect(int id);

static int slot=0;
module_param(slot, int, S_IRUGO);


static __init int on_load(void)
{

	if (0==slot || 1==slot) {
		printk(KERN_INFO "Probing SD slot %d", slot);
		mxc_mmc_force_detect(slot);
	}
	return -1;
}


module_init(on_load);
MODULE_LICENSE("GPL");
