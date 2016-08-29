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
//thread number of threadpool
#define THREAD_NUM 2
//how many thread can connect to server in one client main connection
#define CONN_MAX 10
#define EPOLL_SIZE 50
#define FILENAME_MAXLEN 40
#define INT_SIZE 4

//一次rece接收数据大小
//#define RECVBUF_SIZE 4096      //4K
#define RECVBUF_SIZE 16384       //16K
//#define RECVBUF_SIZE 32768     //32K
//#define RECVBUF_SIZE 131072    //128K
//#define RECVBUF_SIZE 262144    //256K
//#define RECVBUF_SIZE 65536     //64K

struct fileinfo
{
    char filename[FILENAME_MAXLEN];
    unsigned int filesize;
    int count;                  //the number of file blocks
    unsigned int bs;            //the size of every block
};

//every file block info
struct head
{
    char filename[FILENAME_MAXLEN];
    int id;                      //connection_suffix in g_connection[]
    unsigned int offset;        //the offset of this block in source file
    unsigned int bs;            //the real size of this block
};

//the connection of every thread to send block in client, shared in threads
struct conn
{
    int info_fd;                 //the thread connection sock with server
    char filename[FILENAME_MAXLEN];
    unsigned int filesize;
    unsigned int bs;
    int count;                  //the number of blocks
    int recvcount;              //the number of blocks hava received
    char *mbegin;               //the begin address of mmap
    int used;                   //flag: 1 means have used, 0 means haven't used
};

//thread params
struct args
{
    int fd;
    void (*recv_finfo)(int fd);
    void (*recv_fdata)(int fd);
};

/**
 * [create_file create a new file on svr]
 * @param  filename [filename with absolute path]
 * @param  size     [file size]
 * @return          [success:0]
 */
int create_file(char *filename, unsigned int size);

/**
 * [server_connection_init initial server connection]
 * @param  port [server port]
 * @return      [socket fd]
 */
int server_connection_init(int port);

/**
 * [set_fd_noblock set fd with option O_NOBLOCK]
 * @param fd [file descriptor]
 */
void set_fd_noblock(int fd);

/**
 * [recv_fileinfo receive file info, use mmap and g_connection, send back connection_suffix]
 * @param sockfd [server socket file descriptor]
 */
void recv_fileinfo(int sockfd);

/**
 * [recv_filedata receive file blocks, head info and munmap]
 * @param sockfd [server socket file descriptor]
 */
void recv_filedata(int sockfd);

/**
 * [worker thread main work : receive file info or file block according to request type]
 * @param  argc [thread_param]
 * @return      [NULL]
 */
void * worker(void *argc);

#endif
