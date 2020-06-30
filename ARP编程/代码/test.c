#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/if_ether.h>
#include <netinet/in.h>
#include <net/if_arp.h>
#include <linux/if_packet.h>

struct arppacket
{
    struct arphdr ar_head;/*硬件类型、协议、地址长度、操作码*/
    unsigned char ar_sha[ETH_ALEN];/*发送方MAC*/
    struct in_addr ar_sip;/*发送方IP*/
    unsigned char ar_tha[ETH_ALEN];;/*目的MAC*/
    struct in_addr ar_tip;/*目的IP*/
}__attribute__((packed));

int main(int argc, char **argv)
{
	//初始化及参数检查
    struct in_addr pingaddr;
    struct in_addr netaddr;
    struct in_addr netmask;
    struct sockaddr_ll hwaddr;
    memset(&netaddr, 0, sizeof(netaddr));
    memset(&netmask, 0, sizeof(netmask));
    memset(&hwaddr, 0, sizeof(hwaddr));

	//输入错误，显示正确输入格式
    if(argc != 2)
    {
        perror("Usage: ./test xxx.xxx.xxx.xxx\n");
        exit(-1);
    }

	//参数错误，输入的不是ip地址
    if(inet_aton(argv[1], &pingaddr) < 0)
    {
        perror("not a correct ip address\n");
        exit(-1);
    }


    // 注册原始套接字
    int fd = socket(PF_PACKET, SOCK_RAW, htonl(0x0003));

    // 获得网卡信息
    char buf[1024];
    struct ifconf ifc;
    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = buf;
    if(ioctl(fd, SIOCGIFCONF, &ifc) == -1)
    {
        perror("get net interface error\n");
        exit(-1);
    }


    // get correct net interface
    int flag = 0;
	int i;
    struct ifreq *ifrPtr;
    int networkNum = ifc.ifc_len / sizeof(struct ifreq);
    for(i = 0; i < networkNum; i++)
    {
        ifrPtr = ((struct ifreq*)buf) + i;
        if(ioctl(fd, SIOCGIFADDR, ifrPtr) == -1)
        {
            perror("get ip address error\n");
            exit(-1);
        }
        netaddr = ((struct sockaddr_in*)&(ifrPtr->ifr_addr))->sin_addr;

        if(ioctl(fd, SIOCGIFNETMASK, ifrPtr) == -1)
        {
            perror("get netmask error\n");
            exit(-1);
        }
        netmask = ((struct sockaddr_in*)&(ifrPtr->ifr_netmask))->sin_addr;
		// 设置sockaddr_ll地址
        if((pingaddr.s_addr & netmask.s_addr) == (netaddr.s_addr & netmask.s_addr))
        {
            hwaddr.sll_family = PF_PACKET;
            hwaddr.sll_protocol = htons(ETH_P_ARP);
            hwaddr.sll_hatype = ARPHRD_ETHER;
            hwaddr.sll_pkttype = PACKET_OTHERHOST;
            hwaddr.sll_halen = ETH_ALEN;

            if(ioctl(fd, SIOCGIFINDEX, ifrPtr) == -1)
            {
                perror("get net interface index error\n");
                exit(-1);
            }
            hwaddr.sll_ifindex = ifrPtr->ifr_ifindex;

            if(ioctl(fd, SIOCGIFHWADDR, ifrPtr) == -1)
            {
                perror("get net interface hwaddr error\n");
                exit(-1);
            }
            memcpy(hwaddr.sll_addr, ifrPtr->ifr_hwaddr.sa_data, ETH_ALEN);

            flag = 1;
            break;
        }
    }

    if(flag != 1)
    {
        perror("can not find correct net interface\n");
        exit(-1);
    }


    // 构建ARP请求包
    char ef[ETH_FRAME_LEN];
	//使p_eth指向以太网帧的帧头
    struct ethhdr *p_eth = (struct ethhdr*)ef;
	//目的以太网地址
    memset(p_eth->h_dest, 0xff, ETH_ALEN);
	//源以太网地址
    memcpy(p_eth->h_source, hwaddr.sll_addr, ETH_ALEN);
	//设置协议类型
    p_eth->h_proto = htons(ETH_P_ARP);

	//定位ARP包地址
    struct arppacket *p_arp = (struct arppacket*)(ef + ETH_HLEN);
	//硬件类型
    p_arp->ar_head.ar_hrd = htons(ARPHRD_ETHER);/*arp硬件类型*/
    p_arp->ar_head.ar_pro = htons(ETH_P_IP);    /*协议类型*/
    p_arp->ar_head.ar_hln = ETH_ALEN;           /*硬件地址长度*/
    p_arp->ar_head.ar_pln = 4;                   /*IP地址长度*/
    p_arp->ar_head.ar_op = htons(ARPOP_REQUEST);

    /*复制源以太网地址*/
    memcpy(p_arp->ar_sha, hwaddr.sll_addr, ETH_ALEN);
	/*源IP地址*/
    p_arp->ar_sip = netaddr;
	/*复制目的以太网地址*/
    memset(p_arp->ar_tha, 0, ETH_ALEN);
	//目的IP地址
    p_arp->ar_tip = pingaddr;
	//绑定网卡
    if(bind(fd, (struct sockaddr*)&hwaddr, sizeof(struct sockaddr_ll)) == -1)
    {
        perror("bind network error\n");
        exit(-1);
    }
	//发送以太网帧
    write(fd, ef, 60);
	//读取并打印目的MAC
    while(1)
    {
        read(fd, ef, sizeof(ef));
        struct arppacket *recv_arp = (struct arppacket*)(ef+ETH_HLEN);
        if(recv_arp->ar_tip.s_addr == netaddr.s_addr)
        {
			printf("Find %s 's mac:",argv[1]);
			for (i = 0; i < ETH_ALEN - 1; i++)
				printf("%02x-", recv_arp->ar_sha[i]);
			printf("%02x\n", recv_arp->ar_sha[ETH_ALEN - 1]);

            break;
        }
    }
	return 0;
}


