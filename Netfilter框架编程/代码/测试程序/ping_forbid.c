#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

#define SOE_BANDIP 0X6001
#define SOE_BANDPORT 0X6002
#define SOE_BANDPING 0X6003

#define NF_SUCCESS 0
#define NF_FAILURE 1

struct nf_bandport
{
    unsigned short protocol;

    unsigned short port;
};

struct band_status
{
    unsigned int band_ip;

    struct nf_bandport band_port;

    unsigned char band_ping;
};

int main()
{
    struct band_status b_status;
    b_status.band_ping = 1;
    socklen_t len = sizeof(b_status);

    int fd = socket(PF_INET, SOCK_RAW, IPPROTO_RAW);
    if(fd == -1)
    {
        perror("create socket error\n");
        exit(-1);
    }

    if(setsockopt(fd, IPPROTO_IP, SOE_BANDPING, &b_status, len) == -1)
    {
        perror("set sock opt error\n");
        printf("errno: %d\n", errno);
        exit(-1);
    }

    return 0;
}
