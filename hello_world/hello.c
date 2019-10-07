#include <linux/kernel.h>
#include <linux/module.h>

static int __init zxs_hello_module_init(void)
{
	printk(KERN_INFO "*******************Hello World**********\n");
	return 0;
}

static int __exit zxs_hello_module_clear(void)
{
	printk(KERN_INFO"------------------Good Bye--------------------\n");
}

module_init(zxs_hello_module_init);
module_exit(zxs_hello_module_clear);

MODULE_LICENSE("GPL");
