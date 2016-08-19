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

//#define SEND_SIZE		1024*8
//#define RECV_SIZE		1024*8
#define SEND_SIZE		1024*16
#define RECV_SIZE		1024*16
//#define SEND_SIZE		1024*32
//#define RECV_SIZE		1024*32
//#define SEND_SIZE		1024*64
//#define RECV_SIZE		1024*64
//#define SEND_SIZE		1024*128
//#define RECV_SIZE		1024*128

#define MAX_LISTEN 5
#define COMMAND_SIZE  9

/**
 * [fake_client_uploadfile what client should do when upload]
 * @param  {[int]} m_socketfd [connection fd]
 * @param  {[char *]} src_file_path [point to suorce file path]
 * @param  {[char *]} dest_file_path [point to destination file path]
 * @return {[int]}  success:1, interrupt:0, failed:-1 [status code]
 */
int fake_client_uploadfile(int m_socketfd, char *src_file_path, char *dst_file_path);

/**
 * [fake_server_uploadfile what server should do when upload]
 * @param  {[int]} m_connectfd [connection fd]
 * @param  {[char *]} filename_buf [point to filename]
 * @return {[int]} success:1, interrupt:0, failed:-1  [status code]
 */
int fake_server_uploadfile(int m_connectfd, char *filename_buf);

/**
 * [fake_client_downloadfile what client should do when download]
 * @param  {[int]} m_socketfd [connection fd]
 * @param  {[char *]} src_file_path [point to suorce file path]
 * @param  {[char *]} dest_file_path [point to destination file path]
 * @return {[int]}  success:1, interrupt:0, failed:-1 [status code]
 */
int fake_client_downloadfile(int m_socketfd, char *src_file_path, char *dst_file_path);

/**
 * [fake_server_downloadfile what server should do when download]
 * @param  {[int]} m_connectfd [connection fd]
 * @param  {[char *]} filename_buf [point to filename]
 * @return {[int]} success:1, interrupt:0, failed:-1  [status code]
 */
int fake_server_downloadfile(int m_connectfd, char *filename_buf);

/**
 * [fake_strncpy most copy (size -1) chars from source to destination,always end by '/0']
 * @param  {[char *]} dst [point to string destination]
 * @param  {[const char *]} src [point to string source]
 * @param  {[size_t]} size [most copy (size -1) chars]
 * @return {[size_t]} [the length copy from source string, not include the end '/0']
 */
size_t fake_strncpy( char *dst, const char *src, size_t size );

/**
 * [fake_strncat most append string from src to dst, no longer than the length of dst]
 * @param  {[char *]} dst [point to string destination]
 * @param  {[const char *]} src [point to string source]
 * @param  {[size_t]} size [most copy (size -1) chars]
 * @return {[size_t]} [the new length of dst]
 */
size_t fake_strncat( char* dst, const char* src, size_t size );

/**
 * [fake_send_string write some string to connection fd]
 * @param  {[int]} socketfd [the connection fd]
 * @param  {[char *]} string [point to the string ]
 * @return {[int]} [success:1,fail:-1]
 */
int fake_send_string(int socketfd, char *string);


#endif
