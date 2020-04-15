#ifndef _COMM_H_  
#define _COMM_H_  
#include<stdio.h>
#include<sys/types.h>
#include<sys/msg.h>
#include<string.h>
#include<sys/ipc.h>
#define PATHNAME "./msg"
#define PROJ_ID 0x6666
#define SERVER_TYPE  1
#define CLIENT_TYPE  2

struct Msgbuf{
        long mtype;
        char mtext[1024];
 };

int commMsgqueue(int flags);
int createmsgqueue();
int getmsgqueue();
int destroymsgqueue(int msgid);
int sendmsg(int msgid,int type,char*buf);
int recvmsg(int msgid,int rtype,char out[]);
#endif
