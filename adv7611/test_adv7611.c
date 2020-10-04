#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <time.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>//SOL_TCP, TCP_KEEPIDLE 
#include <sys/stat.h>	// dir
#include <dirent.h>
#include <sys/vfs.h>
#include <sys/ioctl.h>

#include <unistd.h>
#include <fcntl.h>

#include "adv7611.h"


int main(void)
{
	printf("adv7611 check hdmiinfo \n");

	int fd;
	unsigned int cable;
	unsigned int val;

	HDMIINFO checkhdmiinfo;


	fd = open("/dev/adv7611", O_RDWR);
	if(fd < 0)
	{
		printf("open device error\n");
		return -1;
	}


	val = 1;
	ioctl(fd, EN_EXTEDTD, &val);

	while(1)
	{

		ioctl(fd, CHECKCABLE, &cable);

		if(cable == 1)
		{
			ioctl(fd, CHECKHDMIINFO, &checkhdmiinfo);

			printf("hdmiinfo scanmode:%d width:%d height:%d fps:%d audiosamplerate:%d \n",\
				checkhdmiinfo.sacnmode, checkhdmiinfo.width, checkhdmiinfo.height, checkhdmiinfo.fps, checkhdmiinfo.samplerate);
		}
		else
			printf("hdmi disconnect \n");
		sleep(1);
	}	
		

	return -1; 
}



