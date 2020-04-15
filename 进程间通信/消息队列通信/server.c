#include"comm.h"

int main()
{
        int msgid=createmsgqueue();
        char buf[1024];
        while(1){
                buf[0]=0;
                recvmsg(msgid,CLIENT_TYPE,buf);
				if ((buf[0]=='b')&&buf[1]==('y')&&buf[2]==('e')){
					sendmsg(msgid,SERVER_TYPE,buf);
					printf("client# %s\n",buf);
					destroymsgqueue(msgid);
					return 0;
				}
                printf("client# %s\n",buf);

                printf("please enter#:");
                fflush(stdout);
                ssize_t s=read(0,buf,sizeof(buf)-1);
                if(s>0){
                        buf[s]=0;
                        sendmsg(msgid,SERVER_TYPE,buf);
			if ((buf[0]=='b')&&buf[1]==('y')&&buf[2]==('e'))
			{destroymsgqueue(msgid);
					return 0;
			}
                        else{printf("send done,wait recv ...\n");}
                }
        }
        destroymsgqueue(msgid);
        return 0;
}
