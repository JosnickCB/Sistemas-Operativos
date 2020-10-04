#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/wait.h>

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
		printf(" (%d:%d:%d.%d) \n",hora->h,hora->m,hora->s,hora->ms);
	}
}

int main(void){
	struct item nproduced;
	struct region* ptr;
	int fd;
	fd = shm_open(name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
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
	pid_t pid;
	struct clocky hour;
	while(1){
		int i = 0;
		for(;i<size;i++){
			if(ptr->buff[i].pid==0){
				break;
			}
			if(i==size-1){
				kill(getpid(),SIGKILL);
				kill(getppid(),SIGKILL);
				return -1;
			}
				
		}
		sleep(rand()%6);
		pid = fork();
		if(pid<0){
			printf("Fallo creacion de fork()");
			return -1;
		}else if(pid==0){
			ptr->buff[i].pid = getpid();
			update(&hour);
			ptr->buff[i].thehour.h = hour.h;
			ptr->buff[i].thehour.m = hour.m;
			ptr->buff[i].thehour.s = hour.s;
			ptr->buff[i].thehour.ms = hour.ms;
			see_h(&hour,'f');
			printf("P(%d): Creando Proceso %d",getppid(),ptr->buff[i].pid);
			see_h(&hour,'s');
		}else{
			wait(NULL);
		}
	}
	return 0;
}