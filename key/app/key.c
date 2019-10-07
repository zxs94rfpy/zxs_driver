#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#define ARRY_SIZE(x) (sizeof(x)/sizeof(x[0]))
int main(int argc , char **argv)
{
	int button_fd ;
	char current_button_value[4]={0}; //板载八个按键 key2-9
	char prior_button_value[4]={0}; //用于保存按键的前键值
	button_fd = open("/dev/buttons",O_RDONLY);
	if(button_fd <0)
	{
		perror("open device :");
		exit(1);
	}
	while(1)
	{
		int i ;
		if (read(button_fd, current_button_value, sizeof(current_button_value)) != sizeof(current_button_value)) 
		{ 
			perror("read buttons:");
			exit(1);
		}
		for(i = 0;i < ARRY_SIZE(current_button_value);i++)
		{
			if(prior_button_value[i] != current_button_value[i])
			{ 
			//判断当前获得的键值与上一的键值是否一致，以判断按键有没有被按下或者释放
				prior_button_value[i] = current_button_value[i];
				switch(i)
				{
					case 0:
						printf("K1\t%s\n",current_button_value[i]=='0'?"Release":"Pressed");
						break;
					case 1:
						printf("K2 \t%s\n",current_button_value[i]=='0'?"Release":"Pressed");
						break;
					case 2:
						printf("K3 \t%s\n",current_button_value[i]=='0'?"Release":"Pressed");
						break;
					case 3:
						printf("K4 \t%s\n",current_button_value[i]=='0'?"Release":"Pressed");
						break;
					default:
						printf("\n");
						break;
				} 
			}
		} 
	}
	return 0;
}

