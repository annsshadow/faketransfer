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

#define SERVER_IP   "10.26.99.58"
#define PORT 12321
#define FILENAME_MAXLEN   40
#define INT_SIZE    4

#define SEND_SIZE 16384     //16K
//#define SEND_SIZE 32768   //32K
//#define SEND_SIZE 65536   //64K
//#define SEND_SIZE	131072  //128K
//#define SEND_SIZE	262144  //256K

#define BLOCKSIZE 134217728		//128M
//#define BLOCKSIZE 268435456	//256M
//#define BLOCKSIZE 536870912	//512M
//#define BLOCKSIZE	1073741824	//1G


struct fileinfo
{
    char filename[FILENAME_MAXLEN];
    unsigned int filesize;
    int count;                          //block count
    unsigned int bs;                   //standard block size
};

struct head
{
    char filename[FILENAME_MAXLEN];
    int id;                             //the suffix of this link in g_connection[]
    unsigned int offset;               //offset by source file
    unsigned int bs;                   //the size of this block
};

/**
 * [set_fd_noblock set fd with option O_NOBLOCK]
 * @param fd [file descriptor]
 */
void set_fd_noblock(int fd);

/**
 * [client_connect_init initial client connection to server]
 * @param  ip [server IP address]
 * @return    [success:socket fd, fail:-1]
 */
int client_connect_init(char *ip);

/**
 * [send_fileinfo description]
 * @param sock_fd [connection socket fd]
 * @param fname   [file name with absolute path]
 * @param p_fstat [file property]
 * @param p_finfo [file info]
 * @param flag    [whether the last send block is normal(flag==0) or not(flag==1)]
 */
void send_fileinfo(int sock_fd, char *fname, struct stat* p_fstat, struct fileinfo *p_finfo, unsigned int *flag);

/**
 * [send_fileblock send file block and free params, close connection fd]
 * @param  args [type:struct head * p_fhead]
 * @return      [NULL]
 */
void * send_fileblock(void *args);

/**
 * [new_fb_head create new file block header info with new offset]
 * @param  filename [file name with absolute path]
 * @param  freeid   [connection suffix]
 * @param  offset   [block offset in source file]
 * @return          [struct head * p_fhead]
 */
struct head * new_fb_head(char *filename, int freeid, unsigned int *offset);

#endif
