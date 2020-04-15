#include"comm.h"

int commMsgqueue(int flags)
{
        key_t key=ftok(PATHNAME,PROJ_ID);#获取一个key
        if(key==-1){
                perror("ftok");
                return -1;
        }
        int msgid=msgget(key,flags);#根据key获取消息队列的id
        if(msgid<0){
                perror("msgget");
                return -1;
       }
        return msgid;
}
int createmsgqueue()
{
        return  commMsgqueue(IPC_CREAT|IPC_EXCL|0666);
}
int getmsgqueue()
{
        return  commMsgqueue(IPC_CREAT);
}
int destroymsgqueue(int msgid)
{
        int ret=msgctl(msgid,IPC_RMID,NULL);
        if(ret<0){
                perror("msgctl");
                return -1;
        }
        return 0;
}
int sendmsg(int msgid,int type,char*buf)
{
        struct Msgbuf msgbuf;
        msgbuf.mtype=type;
//      if(size>(sizeof(msgbuf.mtext)-1)){
//              printf("size is too large\n");
//              return -1;
//      }
        strcpy(msgbuf.mtext,buf);
        int ret=msgsnd(msgid,(void*)&msgbuf,sizeof(msgbuf.mtext),0);
        if(ret<0){
                perror("msgsnd");
                return -1;
        }
        return 0;
}
int recvmsg(int msgid,int rtype,char out[])
{
       struct Msgbuf msgbuf;
       int ret=msgrcv(msgid,(void*)&msgbuf,sizeof(msgbuf.mtext),rtype,0);
        if(ret<0){
               perror("msgrcv");
                return -1;
        }
        strcpy(out,msgbuf.mtext);
       return 0;
}