#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<errno.h>
#include<wait.h>
#include<signal.h>
#include<pthread.h>

void perr(const char* str)
{
	perror(str);
	exit(1);
}

const char descr[10][20]={"AC","WA","TLE","RE","CE","MLE","UKE"};
char bufin[101][50], bufout[101][50];

struct test_result{
	int state;
	int Mem;
	int Time;
};
typedef struct test_result res_t;

int get_int(char *p)
{
	int x = 0;
	while(*p < '0' || *p > '9') ++p;
	while(*p >='0' && *p <='9'){ 
		x=x*10+*p-'0'; ++p; 
		if(*p == '.') ++p;
	}
	return x;
}

void* tfn(void*args)
{
	int num  = (int)args;
	res_t *res = (res_t*)malloc(sizeof(res_t));
	res->state = 7; //Unknown Error
	res->Time = 0;
	res->Mem = 0;

	char info[10], output[10];
	sprintf(info, "info%d", num);
	sprintf(output, "out%d", num);
	int fd = open(info, O_RDWR | O_CREAT, 0666);

	if(unlink(info) < 0){
		fprintf(stderr, "delete information error");
		return (void*)res;
	}

	char arg[50];
	sprintf(arg, "timeout -s SIGKILL 2s /usr/bin/time -v -o %s ./Test_code.exe <%s >%s", info, bufin[num], output);
	int status = system(arg);//fork失败返回-1，execl失败返回127
	if(status == -1 || status == 127){
		fprintf(stderr, "timeout/time programming error on test-%d", num);
		return (void*)res;
	}
	
	if(WIFEXITED(status))
	{
		char time_info[256];
		read(fd, time_info, 256);
		int utime = get_int(strstr(time_info, "User time"));
		int stime = get_int(strstr(time_info, "System time"));
		int ttime = utime + stime;
		int mem = get_int(strstr(time_info, "Maximum resident"));
		res->Time = ttime;
		res->Mem = mem;
		
		if(ttime > 100)
		{
			res->state = 2; //TLE;
			return (void*)res;
		}

		sprintf(arg, "diff --ignore-all-space --ignore-blank-lines %s %s", bufout[num], output);
		int st = system(arg);
		if(WIFEXITED(st))
		{
			if(WEXITSTATUS(st) == 0)
			{
				res->state = 0; //AC
			}
			else
			{
				res->state = 1; //WA
			}
		}
	}

	if(WIFSIGNALED(status))
	{
		res->state = 3; //RE
		if(WTERMSIG(status) == SIGKILL)
			res->state = 2; //TLE
	}
		
	return (void*)res;	
}

int main(int argc, char *args[])
{
	int fdin = open(args[1], O_RDONLY);
	int fdout = open(args[2], O_RDONLY);
	int ret;
	
	pthread_t tid[101];
	
	int NO = 1;
	while(1)
	{
		ret = read(fdin, bufin[NO], 1024);
		if(ret == 0) break;
		if(ret == -1) perr("read error");
		
		ret = read(fdout, bufout[NO], 1024);
		if(ret == 0) break;
		if(ret == -1) perr("read error");
		
		pthread_create(tid+NO, NULL, tfn, (void*)NO);	
		++NO;
	}
	
	close(fdin);
	close(fdout);

	res_t *Res[101];

	for(int i = 1; i <= NO; ++i)
	{
		ret = pthread_join(tid[i], (void**)&Res[i]);
		if(ret != 0){
			fprintf(stderr, "pthread_join error (test-%d): %s", i, strerror(ret));
			exit(1);
		}
	}
	
	int count = 0;
	for(int i = 1; i <= NO; ++i)
	{
		count += !Res[i]->state;
		printf("Test-%d: %s time=%3dms\n", i, descr[Res[i]->state], Res[i]->Time);
		free(Res[i]);
	}
	
	if(count == NO)
		printf("Accept\n");
	else
		printf("Unaccept\n");

	return 0;
}
