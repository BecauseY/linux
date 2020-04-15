#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#define K 1024
#define WRITELEN (128*K)
int main(void)
{
	int result = -1;					/*创建管道结果*/
	int fd[2],nbytes;					/*文件描述符，字符个数*/
	pid_t pid;							/*PID值*/
	char string[WRITELEN] = "你好，管道";
	char readbuffer[10*K];				/*读缓冲区*/
	/* 文件描述符1用于写，文件描述符0用于读 */
	int *write_fd = &fd[1];
	int *read_fd = &fd[0];
	
	result = pipe(fd);					/*建立管道*/
	if( -1 == result)					/*建立管道失败*/
	{
		printf("建立管道失败\n");			/*打印信息*/
		return -1;						/*返回错误结果*/
	}
	
	pid = fork();						/*分叉程序*/
	if( -1 == pid)						/*fork失败*/
	{
		printf("fork 进程失败\n");		/*打印信息*/
		return -1;						/*返回错误结果*/
	}
	
	if( 0 == pid)						/*子进程*/
	{
		int write_size = WRITELEN;		/*写入的长度*/
		result = 0;						/*结果*/
		close(*read_fd);				/*关闭读端*/
		while( write_size >= 0)			/*如果没有将数据写入继续操作*/
		{
			result = write(*write_fd,string,write_size); /*写入管道数据*/
			if(result >0)				/*写入成功*/
			{
				write_size -= result;	/*写入的长度*/
				printf("写入%d个数据，剩余%d个数据\n",result,write_size);
			}
			else						/*写入失败*/
			{
				sleep(10);				/*等待10s，读端将数据读出*/
			}
		}
		return 0;	
	}
	else								/*父进程*/
	{
		close(*write_fd);				/*关闭写端*/
		while(1)						/*一直读取数据*/
		{
			nbytes = read(*read_fd, readbuffer,sizeof(readbuffer)); 											/*读取数据*/		
			if(nbytes <= 0)				/*读取失败*/
			{
				printf("没有数据写入了\n");/*打印信息*/
				break;					/*退出循环*/
			}	
			printf("接收到%d个数据，内容为:”%s“\n",nbytes,readbuffer);
		}
				
	}
	
	return 0;
}