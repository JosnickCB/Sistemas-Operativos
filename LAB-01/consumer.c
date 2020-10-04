#include <sys/mman.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>

#define name "/forker"
#define size 10

struct clocky{
	int h,m,s,ms;
};

struct item{
	struct clocky thehour;
	int in,out;
	pid_t pid;
};	

struct region{
	struct item buff[size];
};

void update(struct clocky* hora){
	struct timeval actual;
	struct tm* info;
	time_t timer;
	gettimeofday(&actual,NULL);
	timer = actual.tv_sec;
	info = localtime(&timer);
	hora->h = info->tm_hour;
	hora->m = info->tm_min;
	hora->s = info->tm_sec;
	hora->ms = (actual.tv_usec)/1000;
}

void see_h(struct clocky* hora, char e){
	if(e=='f'){
		printf("[%d:%d:%d.%d] ",hora->h,hora->m,hora->s,hora->ms);
	}else{
		printf("(%d:%d:%d.%d) ",hora->h,hora->m,hora->s,hora->ms);
	}
}

int main(void){
	int fd;
	struct region* ptr;	
	fd = shm_open(name,O_RDWR, S_IRUSR | S_IWUSR);
	if(fd == -1) {
		printf("Fallo en creacion de memoria compartida\n");
		exit(-1);
	}
	if(ftruncate(fd,sizeof(struct region)) == -1){
		printf("Fallo truncate\n");
		exit(-1);
	}
	ptr = mmap(NULL,sizeof(struct region),PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
	if(ptr == MAP_FAILED){
		printf("Fallo mapeo\n");
	}
	int idx=0;
	struct clocky hour;
	while(1){
		int i = 0;
		for(;i<size;i++){
			if(ptr->buff[i].pid!=0){
				break;
			}
			if(i==size-1) i = 0;
		}
		update(&hour);
		kill(ptr->buff[i].pid,SIGKILL);
		see_h(&hour,'f');
		printf("C(%d): Matando Proceso %d ",getpid(),ptr->buff[i].pid);
		ptr->buff[i].pid = 0;
		printf("(vivió %d:%d)\n",abs(hour.s-ptr->buff[i].thehour.s),abs(hour.ms-ptr->buff[i].thehour.ms));
		sleep(rand()%6);
	}
	return 0;
}