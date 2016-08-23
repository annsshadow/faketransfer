/*************************************************************************
	> File Name: faketransfer-mt-cli-fun.h
	> Author: annsshadow
	> Mail: cravenboy@163.com
	> Created Time: Wed 12 Aug 2016 15:17:28 PM HKT
 ************************************************************************/

#ifndef FAKETRANSFER_MT_CLIENT
#define FAKETRANSFER_MT_CLIENT

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/mman.h>
#include <time.h>

#define SERVER_IP   "10.26.99.58"    //server IP
#define PORT 12321
#define THREAD_NUM  2               //线程池大小
#define FILENAME_MAXLEN   40        //文件名最大长度
#define INT_SIZE    4               //int类型长度

#define SEND_SIZE    16384       	//16K
//#define SEND_SIZE    32768       	//32K
//#define SEND_SIZE    65536       	//64K
//#define SEND_SIZE	131072			//128K
//#define SEND_SIZE	262144			//256K

#define BLOCKSIZE   134217728		//128M
//#define BLOCKSIZE   268435456		//256M
//#define BLOCKSIZE   536870912		//512M
//#define BLOCKSIZE	1073741824		//1G

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

//创建文件
int createfile(char *filename, unsigned int size);

//设置fd非阻塞
void set_fd_noblock(int fd);

//初始化客户端
int Client_init(char *ip);

//发送文件信息，初始化分块头部信息
//last_bs==0:所有分块都是标准分块
//flag>0:最后一个分块不是标准分块，last_bs即为最后一块大小
void send_fileinfo(int sock_fd                  //要发送文件fd
                   , char *fname                //filename
                   , struct stat* p_fstat       //文件属性结构体
                   , struct fileinfo *p_finfo   //返回初始化后的文件信息
                   , unsigned int *flag);                 //最后一个分块是否时标准分块，0代表是；1代表不是

//发送文件数据块
void * send_filedata(void *args);

//生成文件块头部
struct head * new_fb_head(char *filename, int freeid, unsigned int *offset);

#endif
