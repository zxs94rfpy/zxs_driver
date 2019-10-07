#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>

#include <linux/slab.h>
#include <linux/uaccess.h>

#include <linux/ioctl.h>
#include <linux/gpio.h>
#include <cfg_type.h>

MODULE_LICENSE("GPL");

#define LED_ON _IOW('G',1,int)
#define LED_OFF _IOW('G',2,int)

#define LED_ON_ALL _IO('G',3)
#define LED_OFF_ALL _IO('G',4)


#define ZXS_6818_LED_1 1
#define ZXS_6818_LED_2 2
#define ZXS_6818_LED_3 3
#define ZXS_6818_LED_4 4

#define ZXS_6818_LED_ON 0
#define ZXS_6818_LED_OFF 1

static int zxs_6818_led_gpio_init(void)
{
	int r = 0;
	r = gpio_request(PAD_GPIO_E+13,"GPIOE13");
	if(r!=0)
	{
		printk(KERN_ERR "failed to gpio_request GPIOE13\n");
		return -1;
	}
	gpio_direction_output(PAD_GPIO_E+13, 1);
	
	r = gpio_request(PAD_GPIO_C+17,"GPIOC17");
	if(r!=0)
	{
		printk(KERN_ERR "failed to gpio_request GPIOC17\n");
		return -1;
	}
	gpio_direction_output(PAD_GPIO_C+17, 1);
		
	r = gpio_request(PAD_GPIO_C+8,"GPIOC8");
	if(r!=0)
	{
		printk(KERN_ERR "failed to gpio_request GPIOC8\n");
		return -1;
	}
	gpio_direction_output(PAD_GPIO_C+8, 1);
	
	r = gpio_request(PAD_GPIO_C+7,"GPIOC8");
	if(r!=0)
	{
		printk(KERN_ERR "failed to gpio_request GPIOC7\n");
		return -1;
	}
	gpio_direction_output(PAD_GPIO_C+7, 1);

	return r;
	
}

static void zxs_6818_led_gpio_free(void)
{
	gpio_free(PAD_GPIO_E + 13);
	gpio_free(PAD_GPIO_C + 17);
	gpio_free(PAD_GPIO_C + 8);
	gpio_free(PAD_GPIO_C + 7);
}

static void zxs_6818_led_ctl(int ledNum , int onoff)
{
	switch(ledNum)
	{
		case ZXS_6818_LED_1 : 
		{
			if(onoff == ZXS_6818_LED_ON)
			{
				gpio_set_value(PAD_GPIO_E+13 , 0);
			}
			else
			{
				gpio_set_value(PAD_GPIO_E+13 , 1);
			}
			break;
		}
			
		case ZXS_6818_LED_2 : 
		{
			if(onoff == ZXS_6818_LED_ON)
			{
				gpio_set_value(PAD_GPIO_C+17 , 0);
			}
			else
			{
				gpio_set_value(PAD_GPIO_C+17 , 1);
			}
			break;
		}

		case ZXS_6818_LED_3 : 
		{
			if(onoff == ZXS_6818_LED_ON)
			{
				gpio_set_value(PAD_GPIO_C+8 , 0);
			}
			else
			{
				gpio_set_value(PAD_GPIO_C+8 , 1);
			}
			break;
		}
	
		case ZXS_6818_LED_4 : 
		{
			if(onoff == ZXS_6818_LED_ON)
			{
				gpio_set_value(PAD_GPIO_C+7 , 0);
			}
			else
			{
				gpio_set_value(PAD_GPIO_C+7 , 1);
			}
			break;
		}
		
		default:
			break;
	}
}
long zxs_6818_led_ioctl(struct file * _file, unsigned int cmd ,unsigned long arg)
{
	switch(cmd)
	{
		case LED_ON:
		{
			int * __user ptr = (int * __user)arg;
			int ledN ; 
			get_user(ledN,ptr);

			zxs_6818_led_ctl(ledN,ZXS_6818_LED_ON);
			break;
		}
		case LED_OFF:
		{
			int * __user ptr = (int * __user)arg;
			int ledN ; 
			get_user(ledN,ptr);

			zxs_6818_led_ctl(ledN,ZXS_6818_LED_OFF);
			break;
		}
		case LED_ON_ALL:
		{
			zxs_6818_led_ctl(ZXS_6818_LED_1,ZXS_6818_LED_ON);
			zxs_6818_led_ctl(ZXS_6818_LED_2,ZXS_6818_LED_ON);
			zxs_6818_led_ctl(ZXS_6818_LED_3,ZXS_6818_LED_ON);
			zxs_6818_led_ctl(ZXS_6818_LED_4,ZXS_6818_LED_ON);
			break;
		}
		case LED_OFF_ALL:
		{
			zxs_6818_led_ctl(ZXS_6818_LED_1,ZXS_6818_LED_OFF);
			zxs_6818_led_ctl(ZXS_6818_LED_2,ZXS_6818_LED_OFF);
			zxs_6818_led_ctl(ZXS_6818_LED_3,ZXS_6818_LED_OFF);
			zxs_6818_led_ctl(ZXS_6818_LED_4,ZXS_6818_LED_OFF);
			break;
		}
		default :
			break;
	}
	return 0;
}

static struct file_operations zxs_6818_led_ops = {
	.unlocked_ioctl = zxs_6818_led_ioctl,
};
static struct miscdevice zxs_misc ={
	.minor = MISC_DYNAMIC_MINOR ,
	.name  = "zxs_misc",
	.fops = &zxs_6818_led_ops 

};
static int __init zxs_6818_led_drv_init(void)
{
	int r = zxs_6818_led_gpio_init();
	if(r!=0)
	{
		printk(KERN_ERR "failed to zxs_6818_led_gpio_init\n");
	}
	return misc_register(&zxs_misc);
}
static void __exit zxs_6818_led_drv_exit(void)
{
	zxs_6818_led_gpio_free();
	misc_deregister(&zxs_misc);
}

module_init(zxs_6818_led_drv_init);
module_exit(zxs_6818_led_drv_exit);

