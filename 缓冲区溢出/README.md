# 文件说明：

exploits:6个攻击程序

vulnerables:6个漏洞程序

说明.pdf有6个程序的详细说明

# 使用说明：

环境：ubuntu16.04  32位

目的：编写exploits攻击漏洞程序获取有root权限的shell

1.安装prelink:sudo apt-get install prelink

2.进入到vulnerables文件夹，先执行make(执行一次即可),再执行sudo make install（每次重新开机都要执行）

3.进入到exploits文件夹，执行make(执行一次即可)

4.每次重新开机都要执行关闭ASLR(地址随机化)：sudo sh -c "echo 0 > /proc/sys/kernel/randomize_va_space"





