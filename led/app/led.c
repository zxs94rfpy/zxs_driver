#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/ioctl.h>
#include <unistd.h>

#include <stdio.h>

#define LED_ON _IOW('G',1,int)
#define LED_OFF _IOW('G',2,int)

#define LED_ON_ALL _IO('G',3);
#define LED_OFF_ALL _IO('G',4);




#define DEVICES_NAME "/dev/zxs_misc"

int main( int argc , char * argv[])
{
	int fd ;
	fd = open(DEVICES_NAME, O_RDONLY);
	if(fd <0)
	{
		perror("failed to open\n");
		return -1;
	}
	int ledN = 0;
	while(1)
	{
		ledN = ledN%4 + 1;
		ioctl(fd,LED_ON,&ledN);
		sleep(1);
		ioctl(fd,LED_OFF,&ledN);
		sleep(1);
	}
	close(fd);
	return 0;
}

