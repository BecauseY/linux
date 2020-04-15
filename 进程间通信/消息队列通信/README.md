# 文件说明：
comm.h  comm.c 编写通用的函数

server.c server端的处理

client.c client端的处理

makefile 编译文件

# 使用说明
1.在命令行输入  make  ,进行编译

2.运行./server 

3.运行./client

4.得先运行服务端，因为这里创建消息队列操作在服务端进行的。

5.输入以  bye   开头的任意字符可同时关闭两进程。

