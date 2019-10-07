#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include <linux/cdev.h>
#include <linux/err.h>
#include <linux/fs.h>

#include <linux/device.h>

#include <linux/slab.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");

uint devno_major = 0; //主设备号
uint devno_monir = 1;//此设备号

char kern_buf[50] ="zxs yu zhou wu di shuai";
//指向字符设备类
static struct cdev *p = NULL;
//指向设备类
static struct class *plcs = NULL;
//指向基类设备
static struct devices*pb=NULL;

struct my_pipe_buffer
{
	char *data;
	int max_len;//data指向的空间最大的长度
	int rpos;	//下一次读的位置
	int wpos;	//下一次写的位置
	int avail_len;	//data指向的缓冲区中可读的字节数
};
struct my_pipe_buffer *pbuf =NULL;

static struct my_pipe_buffer * alloc_pipe_buffer(int maxl)
{
	struct my_pipe_buffer *p = kmalloc(sizeof(*p),GFP_KERNEL);
	p->data = kmalloc(maxl,GFP_KERNEL);
	p->max_len = maxl;
	p->rpos=0;
	p->wpos=0;
	p->avail_len = 0;
	
	return p;
}


int zxs_open(struct inode* _node, struct file*_file)
{
	printk(KERN_INFO "%s %d \n",__FUNCTION__,__LINE__);
	printk(KERN_INFO "**zxs** wu di shuai de zxs lai le \n");
	return 0;
}
int zxs_close (struct inode * _node,struct file * _file)
{
	printk(KERN_INFO "%s %d \n",__FUNCTION__,__LINE__);
	printk(KERN_INFO "**zxs** wu di shuai de zxs zou lo \n");
	return 0;
}

ssize_t zxs_read(struct file* _file,char __user *buf,size_t count, loff_t*offset)
{

	
	int fail_r1 = 0;				//fail_r1 第一次读取失败的字节数
	int fail_r2 = 0;				//fail_r2 第二次读取失败的字节数
	int can_r = count > pbuf->avail_len? pbuf->avail_len:count; //实际能读的字节数
	int read_r = 0 	;				//正在读的字节数
	if(pbuf->avail_len==0)
	{
		printk(KERN_INFO "nothing to read\n");
		return 0;
	}
	if(can_r < pbuf->max_len-pbuf->rpos)//不用从头开始，就读完了
	{
		read_r = can_r;
		fail_r1 = copy_to_user(buf,pbuf->data+pbuf->rpos,read_r);
		pbuf->rpos +=can_r- fail_r1;
		pbuf->avail_len-=can_r;
		
		return can_r-fail_r1; //返回成功读到的字节数
	}
	else
	{
		read_r = pbuf->max_len-pbuf->rpos;
		fail_r1 = copy_to_user(buf,pbuf->data+ pbuf->rpos,read_r);
		
		pbuf->rpos +=read_r-fail_r1;
		pbuf->avail_len-=read_r-fail_r1;
		if(pbuf->rpos == pbuf->max_len)
		{
			pbuf->rpos = 0;
		}
		else							//有些字节没读取成功
		{
			return read_r - fail_r1; 
		}
		
		int remain = can_r - read_r ;//剩余需要copy的空间
		if(remain)
		{
			fail_r2 = copy_to_user(buf+read_r,pbuf->data+pbuf->rpos,remain);
			pbuf->rpos+=remain-fail_r2;
			pbuf->avail_len-=remain-fail_r2;
			return can_r - fail_r2;
		}
		
		return  read_r - fail_r1;

		
	}
	
	
}


ssize_t xxx_read (struct file *_file, char __user * buf, 
				size_t count, loff_t *offset)
{
	printk(KERN_INFO "%s %d \n",__FUNCTION__,__LINE__);
	struct my_pipe_buffer *p = pbuf;
	int ret;
	int w=0;//本次实际从缓冲区读到的数据字节数
//	int max_space = p->max_len - p->avail_len;//可读区域

	int want_bytes =min(p->avail_len,count);

	if(want_bytes == 0)
	{
		return 0;
	}
	if(p->rpos < p->wpos)
	{
		ret = copy_to_user(buf,p->data+p->rpos,want_bytes);
		w+=want_bytes -ret;
		p->rpos +=w;
	}
	else
	{
		int w1=0,w2=0;
		int right_space =p->max_len -p->rpos ;
		if(want_bytes<right_space)
		{
			ret = copy_to_user(buf,p->data+p->rpos,want_bytes);
			w1+=want_bytes -ret;
			p->rpos +=w1;
		}
		else
		{
			ret = copy_to_user(buf,p->data+p->rpos,right_space);
			w1+=right_space -ret ;

			p->rpos+=w1;
			if(p->rpos == p->max_len)
			{
				p->rpos=0;
			}
			int remain=want_bytes - right_space;
			if(w1 == right_space && remain)
			{
				ret = copy_to_user(buf+w1,p->data+p->rpos,remain);
				w2 += remain - ret;
				p->rpos +=w2;
			}
		}
		w+=w1+w2;
	}
	p->avail_len -=w;
	printk(KERN_INFO " %d bytes read successfully\n", w);
}

ssize_t zxs_write(struct file* _file, const char __user *buf , size_t count, loff_t*offset)
{

	printk(KERN_INFO "%s %d \n",__FUNCTION__,__LINE__);
	int fail_w1 = 0;//写失败的字节数
	int	fail_w2 = 0;//写失败的字节数
	int can_r_max =pbuf->max_len-pbuf->avail_len; 	  //最多能写的字节数
	int want_w = can_r_max >count ?count: can_r_max ; //能够写的字节数
	int remain_space = pbuf->avail_len - pbuf->wpos ;//pbuf->wpos距离末尾的空间。
	if(pbuf->max_len == pbuf->avail_len)
	{
		printk(KERN_INFO "nothing to write\n");
		return 0;
	}
	printk(KERN_INFO "%s %d \n",__FUNCTION__,__LINE__);
	if(want_w<remain_space)
	{
		fail_w1 = copy_from_user(buf,pbuf->data+pbuf->wpos,want_w);
		pbuf->wpos+=want_w- fail_w1;
		pbuf->max_len+=want_w-fail_w1;
		printk(KERN_INFO "%s %d \n",__FUNCTION__,__LINE__);
		printk(KERN_INFO "pbuf->avail_len: %d",pbuf->avail_len);
		return want_w-fail_w1;
	}
	else
	{
		fail_w1 = copy_from_user(buf,pbuf->data+pbuf->wpos,remain_space);
		pbuf->wpos+=remain_space- fail_w1;
		pbuf->avail_len+=remain_space -fail_w1;
		if(fail_w1!=0)
		{
			printk(KERN_INFO "%s %d \n",__FUNCTION__,__LINE__);
			printk(KERN_INFO "pbuf->avail_len: %d",pbuf->avail_len);
			return remain_space - fail_w1;
		}
		pbuf->wpos = 0;
		int remain = want_w - remain_space ; //剩余要读的字节数 
		if(remain)
		{
			fail_w2 = copy_from_user(buf , pbuf->data+pbuf->wpos,remain);
			pbuf->wpos+=remain-fail_w2;
			pbuf->avail_len+=remain-fail_w2;
			printk(KERN_INFO "%s %d \n",__FUNCTION__,__LINE__);
			printk(KERN_INFO "pbuf->avail_len: %d",pbuf->avail_len);
			return want_w -fail_w2;
		}
		printk(KERN_INFO "%s %d \n",__FUNCTION__,__LINE__);
		printk(KERN_INFO "pbuf->avail_len: %d",pbuf->avail_len);
		return remain_space - fail_w1;
	}


#if 0
	int r = copy_from_user(buf,kern_buf,count);
	return count-r;
#endif
}



ssize_t  xxx_write (struct file *_file, const char __user * buf,
					size_t count, loff_t * offset)
{
	printk(KERN_INFO "%s %d \n",__FUNCTION__,__LINE__);
	struct my_pipe_buffer *p = pbuf;
	int ret;
	int w = 0; //本次实际写入缓冲区的数据字节数
	//此时缓冲区时最多可以写max_space
	// max_space就是缓冲区空白空间的大小
	int max_space = p->max_len - p->avail_len;


	//本次要写的字节数应该为: MIN{max_space, count}
	int want_bytes = min(max_space, count);


	if (want_bytes == 0)
	{
		return 0;
	}



	if (p->wpos < p->rpos)
	{
		ret = copy_from_user(p->data + p->wpos,  buf, want_bytes);
		w += want_bytes - ret;

		p->wpos += w;
		
	}
	else //if (p->wpos > p->rpos)
	{
		int w1 = 0, w2 = 0;
		int right_space = p->max_len - p->wpos;//右边的空白空间的大小

		if (want_bytes < right_space)
		{
			//只需要一次拷贝
			ret = copy_from_user(p->data + p->wpos,  buf, want_bytes);
			w1 += want_bytes - ret;

			p->wpos += w1;
		}
		else
		{
			//需要两次拷贝
			//第一次拷贝 right_space
			ret = copy_from_user(p->data + p->wpos, buf, right_space);
			w1 += right_space - ret;

			p->wpos += w1;
			if (p->wpos == p->max_len)
			{
				p->wpos = 0;
			}

			//进行第二次拷贝有一个前提:
			//	第一次拷贝完全成功　　&& 还有剩余数据要拷贝
			int remain = want_bytes - right_space;
			if ( w1 == right_space &&  remain )
			{
				ret = copy_from_user(p->data + p->wpos, buf + w1,  remain);
				w2 += remain - ret;
				p->wpos += w2;
				
			}
		}


		w += w1 + w2;
	}


	
	p->avail_len += w;

	return w; //返回实际写入的字节数

}

struct file_operations myops ={
	.open = zxs_open,
	.release = zxs_close,
	.read= xxx_read,
	.write = xxx_write,

};

static int __init  zxs_test_init(void)
{
	int r;
	dev_t devno ; 
	if(devno_major >0)
	{
		devno = MKDEV(devno_major,devno_monir);
		//参数1 设备号
		//参数2 次设备的个数
		//参数3 驱动的名字
		r = register_chrdev_region(devno, 1,"zxs007");
	}
	else
	{	//参数1 denvo获得一个动态分配的设备号
		//参数2   次设备号的基准,从第几个次设备号开始
		//参数3 次设备号的个数
		//参数4 驱动的名字
		r=alloc_chrdev_region(&devno,devno_monir,1,"zxs007");
	}
	if(r!= 0)
	{
		printk(KERN_INFO "sorry,failed to register char devno \n");
		return -1;
	}
	devno_major = MAJOR(devno);
	devno_monir = MINOR(devno);

	printk(KERN_INFO "denvo_major=%d denvo_minor=%d\n",devno_major,devno_monir);
	p=cdev_alloc();
	if(IS_ERR(p))
	{

		//在linux内核中，判断一个指针是否合法 一般不是通过p==NULL
		//在Linux内核中，判断一个指针是否有错误通过宏IS_ERR(p),
		//why? 把错误码当做一个指针返回
		printk(KERN_ERR "sorry , failed to cdev_alloc\n");

		unregister_chrdev_region(devno,1 );
		return -1;
	}
	cdev_init(p,&myops);
	cdev_add(p,devno, 1);
	//create device node
	//方法一 通过命令
	// mknod /dev/device_name c major minor
	//方法二
	plcs = class_create(THIS_MODULE,"zxs_test_char");
	if(IS_ERR(plcs))
	{
		printk(KERN_ERR "sorry failed to class_create \n");
		//删除字符设备节点
		cdev_del(p);
		//释放设备号
		unregister_chrdev_region(devno, 1);

		return -1;
	}
	pb=device_create(plcs,NULL,devno,NULL,"test"); // /dev/test
	if(IS_ERR(pb))
	{
		printk(KERN_ERR "sorry failed to class_create \n");
		//销毁设备类
		class_destroy(plcs);
		//删除字符设备节点
		cdev_del(p);
		//释放设备号
		unregister_chrdev_region(devno, 1);

		return -1;
	}
	printk(KERN_INFO "MODULE w_r init \n");
	pbuf = alloc_pipe_buffer(1024);
	return 0;
}
static void __exit zxs_test_exit(void)
{
	dev_t devno = MKDEV(devno_major,devno_monir);

	
	//删除字符设备节点
	cdev_del(p);

	//释放设备号
	unregister_chrdev_region(devno, 1);

	//删除基类设备
	device_destroy(plcs, devno);

	//删除设备类
	class_destroy(plcs);
//	kfree(pbuf);
	printk(KERN_INFO "MODULE w_r exit zxs\n");
}

module_init(zxs_test_init);
module_exit(zxs_test_exit);

