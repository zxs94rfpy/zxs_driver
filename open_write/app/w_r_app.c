#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int main()
{
	int fd;
	char buf[50]={"zxs wu di shuai "};
	char buf1[50]={0};
	fd = open("/dev/test",O_RDONLY);
	if(fd<0)
	{
		perror("failed to open");
		return -1;
	}
	write(fd,buf,50);
	read(fd,buf1,50);
	printf("buf1 :%s\n",buf1);
	
	printf("b\n");
	close(fd);
	return 0;
}

