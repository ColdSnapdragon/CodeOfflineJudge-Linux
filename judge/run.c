#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<errno.h>
#include<wait.h>
#include<signal.h>
#include<pthread.h>

#define MAX_CASE 100
#define MAX_TIME 1 //1s
#define MAX_TIMEOUT 20 //2s
#define MAX_MEM 131072 //128MB

void perr(const char* str)
{
	perror(str);
	exit(1);
}

const char descr[10][20] = {"AC","WA","TLE","RE","CE","MLE","OLE","UKE"};
char bufin[MAX_CASE][50], bufout[MAX_CASE][50];

struct test_result{
	int state;
	int Mem;
	int Time;
};
typedef struct test_result res_t;

int get_int(char *p)
{
	if(p == NULL) return -1;
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
	int num  = (long)args;
	res_t *res = (res_t*)malloc(sizeof(res_t));
	res->state = 7; //Unknown Error
	res->Time = 0;
	res->Mem = 0;

	char info[10], output[10];
	sprintf(info, "info%d", num);
	sprintf(output, "out%d", num);
	int fd = open(info, O_RDWR | O_CREAT, 0666);
/*
	if(unlink(info) < 0){
		fprintf(stderr, "delete information error");
		return (void*)res;
	}
*/
	char arg[100];
	sprintf(arg, "timeout %ds /usr/bin/time -v -o %s ./Test_code.exe <%s >%s", MAX_TIMEOUT, info, bufin[num], output);
	int status = system(arg);//fork失败返回-1，execl失败返回127
	if(status == -1 || status == 127){
		fprintf(stderr, "timeout/time programming error on test-%d", num);
		return (void*)res;
	}
	
	if(WIFEXITED(status))
	{
		int rtv = WEXITSTATUS(status);
		//printf("%d---%d\n",num,rtv);
		if(rtv == 137 || rtv == 124) //timeout
		{
			res->state = 2; //TLE
			res->Time = MAX_TIMEOUT*100;
			return (void*)res;
		}
		
		char time_info[1024];
		read(fd, time_info, 1024);
		
		int sig = get_int(strstr(time_info, "Command terminated"));
		int utime = get_int(strstr(time_info, "User time"));
		int stime = get_int(strstr(time_info, "System time"));
		int ttime = utime + stime;
		int mem = get_int(strstr(time_info, "Maximum resident"));
		res->Time = ttime;
		res->Mem = mem;
		
		if(sig != -1)
		{
			res->state = 3; //RE
			return (void*)res;
		}
		if(ttime > MAX_TIME*100)
		{
			res->state = 2; //TLE;
			return (void*)res;
		}
		if(mem > MAX_MEM)
		{
			res->state = 5; //MLE
			return (void*)res;
		}

		sprintf(arg, "diff --ignore-all-space --ignore-blank-lines %s %s &> /dev/null", bufout[num], output);
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
		//
	}
		
	return (void*)res;	
}

void* Pending(void* args)
{
	printf("\nPending");
	int _cnt = 0; 
	while(1)
	{
		if(_cnt < 3)
			printf(".");
		else
			printf("\b\b\b   \b\b\b");
		_cnt = (_cnt + 1) % 4;
		fflush(stdout);
		usleep(200000);
	}
	return NULL;
}

int main(int argc, char *args[])
{
	pthread_t ptid;
	pthread_create(&ptid, NULL, Pending, NULL);

	//int fdin = open(args[1], O_RDONLY);
	//int fdout = open(args[2], O_RDONLY);
	FILE *fin = fopen(args[1], "r");
	FILE *fout = fopen(args[2], "r");
	
	int ret;
	pthread_t tid[MAX_CASE];
	
	int NO = 1;
	while(1)
	{
		ret = fscanf(fin, "%s", bufin[NO]);
		if(ret != 1) break;
		//if(ret == -1) perr("read error");
		
		ret = fscanf(fout, "%s", bufout[NO]);
		if(ret != 1) break;
		//if(ret == -1) perr("read error");
		
		pthread_create(tid+NO, NULL, tfn, (void*)(long)NO);	
		++NO;
	}
	--NO;

	fclose(fin);
	fclose(fout);
	
	res_t *Res[MAX_CASE];

	for(int i = 1; i <= NO; ++i)
	{
		ret = pthread_join(tid[i], (void**)&Res[i]);
		if(ret != 0){
			fprintf(stderr, "pthread_join error (test-%d): %s", i, strerror(ret));
			exit(1);
		}
	}
	
	pthread_cancel(ptid);
	printf("\b\b\b\b\b\b\b\b\b\b          \n");

	int count = 0;
	for(int i = 1; i <= NO; ++i)
	{
		count += !Res[i]->state;
		printf("\nTest-%d: %s\ttime=%03dms\tmemory=%dkb\n", i, descr[Res[i]->state], 10*Res[i]->Time, Res[i]->Mem);
		free(Res[i]);
	}
	
	if(count == NO)
		printf("\n-------Accept-------\n");
	else
		printf("\n-------Unaccept-------\n");

	return 0;
}
