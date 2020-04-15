#include"comm.h"

int main()
{
        int msgid=getmsgqueue();
        char buf[1024]={0};
        while(1){
                buf[0]=0;
                printf("please enter# ");
                fflush(stdout);
                ssize_t s=read(0,buf,sizeof(buf)-1);
                if(s>0){
                        buf[s]=0;
                       sendmsg(msgid,CLIENT_TYPE,buf);
                       printf("send done ,wait recv...\n");
               }
                recvmsg(msgid,SERVER_TYPE,buf);
				if ((buf[0]=='b')&&buf[1]==('y')&&buf[2]==('e')){
					printf("server #%s\n",buf);
					return 0;
				}
                printf("server #%s\n",buf);
       }
       return 0;
 }
