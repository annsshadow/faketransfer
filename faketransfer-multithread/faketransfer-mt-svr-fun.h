/*************************************************************************
	> File Name: faketransfer-mt-svr-fun.h
	> Author: annsshadow
	> Mail: cravenboy@163.com
	> Created Time: Wed 12 Aug 2016 15:15:58 PM HKT
 ************************************************************************/

#ifndef FAKETRANSFER_MT_SERVER
#define FAKETRANSFER_MT_SERVER

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <sys/epoll.h>
#include <sys/mman.h>

#define PORT 12321
#define LISTEN_QUEUE_LEN 10
#define THREAD_NUM  2               //线程池大小
#define CONN_MAX  10                //支持最大连接数，一个连接包含多个socket连接（多线程）
#define EPOLL_SIZE  50
#define FILENAME_MAXLEN   40
#define INT_SIZE    4

//一次rece接收数据大小
//#define RECVBUF_SIZE    4096        //4K
#define RECVBUF_SIZE    16384       //16K
//#define RECVBUF_SIZE    32768       //32K
//#define RECVBUF_SIZE    131072      //128K
//#define RECVBUF_SIZE    262144      //256K
//#define RECVBUF_SIZE    65536       //64K

//文件信息
struct fileinfo
{
    char filename[FILENAME_MAXLEN];     //文件名
    unsigned int filesize;             //文件大小
    int count;                          //分块数量
    unsigned int bs;                   //标准分块大小
};

//分块头部信息
struct head
{
    char filename[FILENAME_MAXLEN];     //文件名
    int id;                             //分块所属文件的id，gconn[CONN_MAX]数组的下标
    unsigned int offset;               //分块在原文件中偏移
    unsigned int bs;                   //本文件块实际大小
};

//与客户端关联的连接，每次传输建立一个，在多线程之间共享
struct conn
{
    int info_fd;                      //信息交换socket：接收文件信息、文件传送通知client
    char filename[FILENAME_MAXLEN];   //文件名
    unsigned int filesize;           //文件大小
    unsigned int bs;                 //分块大小
    int count;                        //分块数量
    int recvcount;                    //已接收块数量，recv_count == count表示传输完毕
    char *mbegin;                     //mmap起始地址
    int used;                         //使用标记，1代表使用，0代表可用
};

//线程参数
struct args
{
    int fd;
    void (*recv_finfo)(int fd);
    void (*recv_fdata)(int fd);
};

//创建大小为size的文件
int createfile(char *filename, unsigned int size);

//初始化服务端
int Server_init(int port);

//设置fd非阻塞
void set_fd_noblock(int fd);

//接收文件信息
void recv_fileinfo(int sockfd);

//接收文件块
void recv_filedata(int sockfd);

//线程函数
void * worker(void *argc);

#endif
