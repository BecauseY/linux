#include <linux/netfilter_ipv4.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/skbuff.h>
#include <linux/ip.h>					/*IP头部结构*/
#include <net/tcp.h> 				/*TCP头部结构*/
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#ifndef __NF_SOCKOPTE_H__
#define __NF_SOCKOPTE_H__
/* cmd命令定义：
SOE_BANDIP：IP地址发送禁止命令
SOE_BANDPORT：端口禁止命令
SOE_BANDPING：ping禁止
 */
#define SOE_BANDIP    0x6001
#define SOE_BANDPORT  0x6002
#define SOE_BANDPING  0x6003
/* 禁止端口结构*/
typedef struct nf_bandport
{
	/* band protocol, TCP?UDP  */
	unsigned short protocol;
	
	/* band port */
	unsigned short port;
} nf_bandport;
/* 与用户交互的数据结构 */
typedef struct band_status
{
	/* IP发送禁止，IP地址，当为0时，未设置  */
	unsigned int  band_ip;
	
	/* 端口禁止，当协议和端口均为0时，未设置 */
	nf_bandport band_port;
	
	/* 是否允许ping回显响应，为0时响应，为1时禁止 */
	unsigned char band_ping;
} band_status;
#endif /* __NF_SOCKOPTE_H__ */

/* 版权声明*/
MODULE_LICENSE("Dual BSD/GPL");
/* NF初始化状态宏 */
#define NF_SUCCESS 0
#define NF_FAILURE 1
/* 初始化绑定状态 */
band_status b_status ;
/*快速绑定操作宏*/
/* 判断是否禁止TCP的端口*/
#define IS_BANDPORT_TCP(status)( status.band_port.port != 0 && status.band_port.protocol == IPPROTO_TCP)
/*判断是否禁止UDP端口 */
#define IS_BANDPORT_UDP(status)( status.band_port.port != 0 && status.band_port.protocol == IPPROTO_UDP)
/* 判断端是否禁止 PING */
#define IS_BANDPING(status)( status.band_ping )
/* 判断是否禁止IP协议 */
#define IS_BANDIP(status)( status.band_ip )
/* nf sock 选项扩展操作*/
static int 
nf_sockopt_set(struct sock *sock, 
	int cmd, 
	void __user *user, 
	unsigned int len)
{
	int ret = 0;
	struct band_status status;
	
	/* 权限检查 */
	if(!capable(CAP_NET_ADMIN))				/*没有足够权限*/
	{
		ret = -EPERM;
		goto ERROR;
	}
	/* 从用户空间复制数据*/
	ret = copy_from_user(&status, user,len);
	if(ret != 0)								/*复制数据失败*/
	{
		ret = -EINVAL;
		goto ERROR;
	}
	
	/* 命令类型 */
	switch(cmd)
	{
		case SOE_BANDIP:							/*禁止IP协议*/
			/* 设置禁止IP协议 */
			if(IS_BANDIP(status))					/*设置禁止IP协议*/
				b_status.band_ip = status.band_ip;
			else									/*取消禁止*/
				b_status.band_ip = 0;
	
			break;
		case SOE_BANDPORT:						/*禁止端口*/
			/* 设置端口禁止和相关的协议类型 */
			if(IS_BANDPORT_TCP(status))				/*禁止TCP*/
			{
				b_status.band_port.protocol = IPPROTO_TCP;
				b_status.band_port.port = status.band_port.port;
			}
			else if(IS_BANDPORT_UDP(status))			/*禁止UDP*/
			{
				b_status.band_port.protocol = IPPROTO_UDP;
				b_status.band_port.port = status.band_port.port;
			}
			else									/*其他*/
			{
				b_status.band_port.protocol = 0;
				b_status.band_port.port = 0;
			}
			
			break;
		case SOE_BANDPING:						/*禁止ping*/
			if(IS_BANDPING(status))				/*禁止PING*/
			{
				b_status.band_ping = 1;
			}
			else 									/*取消禁止*/
			{
				b_status.band_ping = 0;
			}
			
			break;
		default:									/*其他为错误命令*/
			ret = -EINVAL;
			break;
	}
	
ERROR:
	return ret;
}
/* nf sock 操作扩展命令操作*/
static int 
nf_sockopt_get(struct sock *sock, 
	int cmd, 
	void __user *user, 
	unsigned int len)
{
	int ret = 0;
	
	/* 权限检查*/
	if(!capable(CAP_NET_ADMIN))				/*没有权限*/
	{
		ret = -EPERM;
		goto ERROR;
	}	
	
	/* 将数据从内核空间复制到用户空间 */
	switch(cmd)
	{
		case SOE_BANDIP:
		case SOE_BANDPORT:
		case SOE_BANDPING:
			/*复制数据*/
			ret = copy_to_user(user, &b_status,len);
			if(ret != 0)								/*复制数据失败*/
			{
				ret = -EINVAL;
				goto ERROR;
			}
			break;
		default:
			ret = -EINVAL;
			break;
	}
	
ERROR:
	return ret;
}
/* 在LOCAL_OUT上挂接钩子 */
static unsigned int nf_hook_out(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    struct iphdr *iph = ip_hdr(skb);
	unsigned int src_ip = iph->saddr;
	unsigned int des_ip = iph->daddr;

    switch (iph->protocol)
    {
        case IPPROTO_ICMP:
            if(IS_BANDPING(b_status))
            {
                printk(KERN_ALERT "DROP one ICMP packet from %d.%d.%d.%d to %d.%d.%d.%d\n"  ,
								 (src_ip&0xff000000ff)>>0,
								 (src_ip&0xff0000ff00)>>8,
								 (src_ip&0x00ff0000)>>16,
								 (src_ip&0xff000000)>>24,
								 (des_ip&0xff000000ff)>>0,
								 (des_ip&0xff0000ff00)>>8,
								 (des_ip&0x00ff0000)>>16,
								 (des_ip&0x00ff000000)>>24);
                return NF_DROP;
            }
            break;
        default:
            break;
    }

    return NF_ACCEPT;
}

/* 在LOCAL_IN挂接钩子 */
static unsigned int nf_hook_in(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
	struct iphdr *iph = ip_hdr(skb);
	unsigned int src_ip = iph->saddr;
	unsigned int des_ip = iph->daddr;
	struct tcphdr *tcph = NULL;
	struct udphdr *udph = NULL;
	
	if(IS_BANDIP(b_status))
    {
        if(b_status.band_ip == iph->saddr)
        {
		printk(KERN_ALERT "DROP one packet from %d.%d.%d.%d to this host\n"  ,
								 (src_ip&0xff000000ff)>>0,
								 (src_ip&0xff0000ff00)>>8,
								 (src_ip&0x00ff0000)>>16,
								 (src_ip&0xff000000)>>24);
            return NF_DROP;
        }
    }
	
	switch(iph->protocol)								/*IP协议类型*/
	{
		case IPPROTO_TCP:								/*TCP协议*/
			/*丢弃禁止端口的TCP数据*/
			if(IS_BANDPORT_TCP(b_status))
			{
				tcph = tcp_hdr(skb);							/*获得TCP头*/
				if(tcph->dest == b_status.band_port.port)	/*端口匹配*/
				{
					printk(KERN_ALERT "drop one tcp packet from %d.%d.%d.%d to the port %d\n"  ,
								 (src_ip&0xff000000ff)>>0,
								 (src_ip&0xff0000ff00)>>8,
								 (src_ip&0x00ff0000)>>16,
								 (src_ip&0xff000000)>>24,
								 ntohs(tcph->dest));
					return NF_DROP;							/*丢弃该数据*/
				}
			}
			
			break;
		case IPPROTO_UDP:								/*UDP协议*/
			/*丢弃UDP数据*/
			if(IS_BANDPORT_UDP(b_status))					/*设置了丢弃UDP协议*/
			{
				udph = udp_hdr(skb);							/*UDP头部*/
				if(udph->dest == b_status.band_port.port)	/*UDP端口判定*/
				{
					printk(KERN_ALERT "drop one udp packet from %d.%d.%d.%d to the port %d\n"  ,
								 (src_ip&0xff000000ff)>>0,
								 (src_ip&0xff0000ff00)>>8,
								 (src_ip&0x00ff0000)>>16,
								 (src_ip&0xff000000)>>24,
								ntohs(udph->dest));
					return NF_DROP;								/*丢弃该数据*/
				}
			}
			
			break;
		default:
			break;
	}
	
	return NF_ACCEPT;
}

/*注册钩子函数，高版本没有所以自己写*/
static int nf_register_hook(struct nf_hook_ops *reg)
{
    struct net *net, *last;
    int ret;

    rtnl_lock();
    for_each_net(net)
    {
        ret = nf_register_net_hook(net, reg);
        if(ret && ret != -ENOENT)
            goto rollback;
    }
    rtnl_unlock();
    return 0;

    rollback:
    last = net;
    for_each_net(net)
    {
        if(net == last)
            break;
        nf_unregister_net_hook(net, reg);
    }
    rtnl_unlock();
    return ret;
}

/*取消注册钩子函数，高版本没有所以自己写*/
static void nf_unregister_hook(struct nf_hook_ops *reg)
{
    struct net *net;

    rtnl_lock();
    for_each_net(net)
    {
        nf_unregister_net_hook(net, reg);
    }
    rtnl_unlock();
}

/* 初始化nfin钩子，在钩子LOCAL_IN上 */
static struct nf_hook_ops nfin = 
{
	.hook = nf_hook_in,
	.hooknum = NF_INET_LOCAL_IN,
	.pf = PF_INET,
	.priority = NF_IP_PRI_FILTER
};
/*初始化nfout钩子，在钩子LOCAL_OUT上*/
static struct nf_hook_ops nfout=
{
	.hook = nf_hook_out,
	.hooknum = NF_INET_LOCAL_OUT,
	.pf = PF_INET,
	.priority = NF_IP_PRI_FILTER
};
/* 初始化nf套接字选项 */
static struct nf_sockopt_ops nfsockopt = {
 .pf	= PF_INET,
 .set_optmin = SOE_BANDIP,
 .set_optmax = SOE_BANDIP+3,
 .set	= nf_sockopt_set,
 .get_optmin = SOE_BANDIP,
 .get_optmax = SOE_BANDIP+3,
 .get	= nf_sockopt_get,
};
/* 初始化模块 */
static int __init init(void)
{
	nf_register_hook(&nfin);					/*注册LOCAL_IN的钩子*/
	nf_register_hook(&nfout);					/*注册LOCAL_OUT的钩子*/
	nf_register_sockopt(&nfsockopt);			/*注册扩展套接字选项*/
	
	printk(KERN_ALERT "2017301510052-zbw netfilter init successfully\n");
												/*打印信息*/
	return NF_SUCCESS;
}
/* 清理模块 */
static void __exit exit(void)
{	
	nf_unregister_hook(&nfin);					/*注销LOCAL_IN的钩子*/
	nf_unregister_hook(&nfout);					/*注销LOCAL_OUT的钩子*/
	nf_unregister_sockopt(&nfsockopt);			/*注销扩展套接字选项*/
	printk(KERN_ALERT "2017301510052-zbw netfilter clean successfully\n");
}
module_init(init);								/*初始化模块*/
module_exit(exit);								/*模块退出*/
/* 作者、描述、版本、别名*/
MODULE_AUTHOR("Jingbin Song");				/*作者声明*/
MODULE_DESCRIPTION("netfilter DEMO");		/*模块描述信息声明*/
MODULE_VERSION("0.0.1");					/*模块版本声明*/
MODULE_ALIAS("ex17.2");						/*模块别名声明*/