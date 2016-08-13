/*************************************************************************
	> File Name: faketransfer.h
	> Author: shadowwen-annsshadow
	> Mail: cravenboy@163.com
	> Created Time: Thu 11 Aug 2016 05:15:28 PM HKT
 ************************************************************************/
#ifndef __FAKETRANSFER__
#define __FAKETRANSFER__

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

#define SERVER_PORT     12321
#define FILENAME_SIZE	100
#define SEND_SIZE		1024*8
#define RECV_SIZE		1024*8
#define COMMAND_SIZE  9
#define MAX_LISTEN 5

/**
  *@brief client to upload
  *@param[in] m_connectfd
  *@param[in] src_file_path
  *@param[in] dst_file_path
  *@return success:1,interrupt:0,wrong:-1
  */
int fake_client_uploadfile(int m_socketfd, char *src_file_path, char *dst_file_path);

/**
  *@brief server to upload
  *@param[in] m_connectfd
  *@param[in] filename_buf
  *@return success:1,interrupt:0,wrong:-1
  */
int fake_server_uploadfile(int m_connectfd, char *filename_buf);


/**
  *@brief copy size string from src to dst
  *@param[in] dst
  *@param[out] src
  *@param[in] siz  sizeof(dst)
  */
size_t strlcpy( char *dst, const char *src, size_t size );

/**
  *@brief append size string from src to dst
  *@param[in] dst
  *@param[out] src
  *@param[in] siz  sizeof(dst)
  */
size_t strlcat( char* dst, const char* src, size_t siz );

/**
  *@brief send string
  *@param[in] socketfd
  *@param[out] string
  *@return success:1,fail:-1
  */
int fake_send_string(int socketfd, char *string);


#endif
