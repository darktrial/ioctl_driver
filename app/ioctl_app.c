#define _XOPEN_SOURCE 500
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/time.h>

#include "../dev/ioctl.h"

typedef struct _measure_time
{
	struct timeval tvBegin, tvEnd, tvDiff;
}measure_time;

#define IOCTL_DRIVER_NAME "/dev/ioctl"

int open_driver(const char* driver_name);
void close_driver(const char* driver_name, int fd_driver);

int open_driver(const char* driver_name) {

    printf("* Open Driver\n");

    int fd_driver = open(driver_name, O_RDWR);
    if (fd_driver == -1) {
        printf("ERROR: could not open \"%s\".\n", driver_name);
        printf("    errno = %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

	return fd_driver;
}

void close_driver(const char* driver_name, int fd_driver) {

    printf("* Close Driver\n");

    int result = close(fd_driver);
    if (result == -1) {
        printf("ERROR: could not close \"%s\".\n", driver_name);
        printf("    errno = %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}


measure_time *start_measure()
{
	measure_time *mt=(measure_time *)malloc(sizeof(measure_time));
	memset(mt,0,sizeof(measure_time));
	gettimeofday(&(mt->tvBegin), NULL);
	return mt;
}

int timeval_subtract(struct timeval *result, struct timeval *t2, struct timeval *t1)
{
	long int diff = (t2->tv_usec + 1000000 * t2->tv_sec) - (t1->tv_usec + 1000000 * t1->tv_sec);
	result->tv_sec = diff / 1000000;
	result->tv_usec = diff % 1000000;

	return (diff<0);
}



void timeval_print(struct timeval *tv)
{
 	printf("Total execution time:%ld.%06ld seconds\n", tv->tv_sec, tv->tv_usec);
}

void stop_measure(measure_time *mt)
{
	gettimeofday(&(mt->tvEnd), NULL);
	timeval_subtract(&(mt->tvDiff),&(mt->tvEnd),&(mt->tvBegin));
	timeval_print(&(mt->tvDiff));
	if (mt) free(mt);
}

void testgetpid_bypidof()
{
    char pidline[1024];
    FILE *fp = NULL;
    fp=popen("pidof systemd-udevd 2>&1","r");
    if (fp){
    	fgets(pidline,1024,fp);
    	printf("%s",pidline);
    	pclose(fp);
    }
}

void testgetpid_byioctl()
{
	int fd_ioctl = open_driver(IOCTL_DRIVER_NAME);
	char value[128];
  
	//get process ID by kernel module
	memset(value,0,128);
	strcpy(value,"systemd-udevd");
	if (ioctl(fd_ioctl, IOCTL_BASE_GET_PSID, value) < 0) {
			perror("Error ioctl PL_AXI_DMA_GET_NUM_DEVICES");
			exit(EXIT_FAILURE);
	}
	else printf("process id:%s\n",value);

	close_driver(IOCTL_DRIVER_NAME, fd_ioctl);	
}


int main(void) {

	measure_time *start_time;
 	//get process id by pidof
	start_time=start_measure();
	testgetpid_bypidof();
	stop_measure(start_time);
	
	start_time=start_measure();
	testgetpid_byioctl();
	stop_measure(start_time);
	return EXIT_SUCCESS;
}


