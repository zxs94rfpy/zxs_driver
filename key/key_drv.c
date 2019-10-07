#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/poll.h>
#include <linux/sched.h>
#include <linux/irq.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <linux/interrupt.h>
#include <asm/uaccess.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/miscdevice.h>
#include <linux/gpio.h>


#include <cfg_type.h>
#define DEVICE_NAME "buttons"
struct button_desc {
	int gpio;
	int number;
	char *name;
	struct timer_list timer;
};
static struct button_desc buttons[] = {
	{ (PAD_GPIO_B + 9 ), 0, "KEY0" },//B9
	{ (PAD_GPIO_A + 28), 1, "KEY1" },//A28
	{ (PAD_GPIO_B + 30), 2, "KEY2" },//volup//B30
	{ (PAD_GPIO_B + 31), 3, "KEY3" },//voidn//B31
};
static volatile char key_values[] = {
	'0', '0','0','0'
};
static DECLARE_WAIT_QUEUE_HEAD(button_waitq);

static volatile int ev_press = 0;
static void gec6818_buttons_timer(unsigned long _data)
{
	struct button_desc *bdata = (struct button_desc *)_data;
	int down;
	int number;
	unsigned tmp;
	tmp = gpio_get_value(bdata->gpio);
	/* active low */
	down = !tmp;
	printk(KERN_INFO "tmp : %d",tmp);
	printk(KERN_DEBUG "KEY %d: %08x\n", bdata->number, down);
	number = bdata->number;
	if (down != (key_values[number] & 1)) 
	{
		key_values[number] = '0' + down;
		ev_press = 1;
		wake_up_interruptible(&button_waitq);
	} 
}
static irqreturn_t button_interrupt(int irq, void *dev_id)
{
	struct button_desc *bdata = (struct button_desc *)dev_id;
	mod_timer(&bdata->timer, jiffies + msecs_to_jiffies(40));
	return IRQ_HANDLED;
}
static int gec6818_buttons_open(struct inode *inode, struct file *file)
{
	int irq;
	int i;
	int err = 0;
	for (i = 0; i < ARRAY_SIZE(buttons); i++)
	{
		if (!buttons[i].gpio)
		continue;
		setup_timer(&buttons[i].timer, gec6818_buttons_timer,
		(unsigned long)&buttons[i]);
		irq = gpio_to_irq(buttons[i].gpio);
		err = request_irq(irq, button_interrupt, IRQ_TYPE_EDGE_BOTH, 
		buttons[i].name, (void *)&buttons[i]);
		if (err)
		break;
	}
	if (err)
	{
		i--;
		for (; i >= 0; i--)
		{
			if (!buttons[i].gpio)
			continue;
			irq = gpio_to_irq(buttons[i].gpio);
			disable_irq(irq);
			free_irq(irq, (void *)&buttons[i]);
			del_timer_sync(&buttons[i].timer);
		}
		return -EBUSY;
	}
	ev_press = 1;
	return 0;
}
static int gec6818_buttons_close(struct inode *inode, struct file *file)
{
	int irq, i;
	for (i = 0; i < ARRAY_SIZE(buttons); i++) 
	{
		if (!buttons[i].gpio)
		continue;
		irq = gpio_to_irq(buttons[i].gpio);
		free_irq(irq, (void *)&buttons[i]);
		del_timer_sync(&buttons[i].timer);
	}
	return 0;
}
static int gec6818_buttons_read(struct file *filp, char __user *buff,size_t count, loff_t *offp)
{
	unsigned long err;
	if (!ev_press)
	{
		if (filp->f_flags & O_NONBLOCK)
		return -EAGAIN;
		else
		wait_event_interruptible(button_waitq, ev_press);
	}
	ev_press = 0;
	err = copy_to_user((void *)buff, (const void *)(&key_values),
	min(sizeof(key_values), count));
	return err ? -EFAULT : min(sizeof(key_values), count);
}
static unsigned int gec6818_buttons_poll( struct file *file,struct poll_table_struct *wait)
{
	unsigned int mask = 0;
	poll_wait(file, &button_waitq, wait);
	if (ev_press)
	mask |= POLLIN | POLLRDNORM;
	return mask;
}
static struct file_operations dev_fops = {
	.owner = THIS_MODULE,
	.open = gec6818_buttons_open,
	.release = gec6818_buttons_close, 
	.read = gec6818_buttons_read,
	.poll = gec6818_buttons_poll,
};
static struct miscdevice misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DEVICE_NAME,
	.fops = &dev_fops,
};
static int __init button_dev_init(void)
{
	int ret;
	ret = misc_register(&misc);
	printk(DEVICE_NAME"\tinitialized\n");
	return ret;
}
static void __exit button_dev_exit(void)
{
	misc_deregister(&misc);
}
module_init(button_dev_init);
module_exit(button_dev_exit);
MODULE_LICENSE("GPL");
//MODULE_AUTHOR("zxs");	

