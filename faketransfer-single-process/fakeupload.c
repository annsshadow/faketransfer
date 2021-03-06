/*************************************************************************
	> File Name: faketransfer.c
	> Author: shadowwen-annsshadow
	> Mail: cravenboy@163.com
	> Created Time: Thu 11 Aug 2016 05:15:11 PM HKT
 ************************************************************************/
#include "faketransfer.h"

/**
 * [fake_server_uploadfile what server should do when upload]
 * @param  {[int]} m_connectfd [connection fd]
 * @param  {[char *]} filename_buf [point to filename]
 * @return {[int]} success:1, interrupt:0, failed:-1  [status code]
 */
int fake_server_uploadfile(int m_connectfd, char *filename_buf)
{
    //check the file name
    if( filename_buf == NULL )
    {
        return -1;
    }
    //get the destination path length
    int dstpath_len = 0;
    if( read(m_connectfd, &dstpath_len, sizeof(int)) < 0 )
    {
        printf("Error! Write file_path len failed\n");
        return -1;
    }
    //get the destination path
    char dst_file_path[FILENAME_SIZE];
    memset(dst_file_path, 0, FILENAME_SIZE);
    if( read(m_connectfd, dst_file_path, dstpath_len) < 0 )
    {
        printf("Error! Write file_path failed\n");
        return -1;
    }
    printf("Destination file path--->%s\n", dst_file_path);

    //open file
    FILE *uploadfile_fp = fopen(dst_file_path, "w");
    if( uploadfile_fp == NULL )
    {
        printf("Error! Open upload file failed\n");
        return -1;
    }
    else
    {
        //get the file length
        long long int file_length = 0;
        if( read(m_connectfd, &file_length, sizeof(int)) < 0 )
        {
            perror("Error! Read file_length failed");
            fclose(uploadfile_fp);
            uploadfile_fp = NULL;
            return -1;
        }
        if( file_length == 0 )
        {
            unlink(dst_file_path);
            return -1;
        }

        //start to upload file to server
        char recvbuf[RECV_SIZE];
        memset(recvbuf, 0, sizeof(recvbuf));
        int transfer_length = 0;
        long long int transfer_sum = 0;
        while((transfer_length = recv(m_connectfd, recvbuf, RECV_SIZE, 0)) > 0)
        {
            if( fwrite(recvbuf, sizeof(char), transfer_length, uploadfile_fp) < (unsigned int)0 )
            {
                printf("Error! Fwrite upload file failed\n");
                break;
            }
            memset(recvbuf, 0, sizeof(recvbuf));
            transfer_sum += transfer_length;
        }
        fclose(uploadfile_fp);
        uploadfile_fp = NULL;
        //success
        if( transfer_sum == file_length )
            return 1;
    }
    //interrupt
    return 0;
}


/**
 * [fake_client_uploadfile what client should do when upload]
 * @param  {[int]} m_socketfd [connection fd]
 * @param  {[char *]} src_file_path [point to suorce file path]
 * @param  {[char *]} dest_file_path [point to destination file path]
 * @return {[int]}  success:1, interrupt:0, failed:-1 [status code]
 */
int fake_client_uploadfile(int m_socketfd, char *source_file_path, char *dest_file_path)
{
    //check the param
    if( source_file_path == NULL || dest_file_path == NULL )
    {
        return -1;
    }
    char *src_file_path = source_file_path;
    char *dst_file_path = dest_file_path;

    //get and send the destination file path
    int dstpath_len = strlen(dst_file_path) + 1;
    if( write(m_socketfd, &dstpath_len, sizeof(int)) < 0 )
    {
        printf("Error! Write file_path length failed\n");
        return -1;
    }

    if( write(m_socketfd, dst_file_path, dstpath_len) < 0 )
    {
        printf("Error! Write file_path failed\n");
        return -1;
    }

    //open source file by read
    FILE *uploadfile_fp = fopen(src_file_path, "r");
    if(uploadfile_fp == NULL)
    {
        printf("Error! Can't find file on local\n");
        return -1;
    }
    else
    {
        //get file length
        struct stat srcfile_stat;
        if( stat(src_file_path, &srcfile_stat) == -1 )
        {
            perror(src_file_path);
            fclose(uploadfile_fp);
            uploadfile_fp = NULL;
            return -1;
        }
        long long int file_length = srcfile_stat.st_size;

        //send file length
        if( write(m_socketfd, &file_length, sizeof(int)) < 0 )
        {
            perror("Error! Send file length failed\n");
            fclose(uploadfile_fp);
            uploadfile_fp = NULL;
            return -1;
        }
        //upload file
        char sendbuf[SEND_SIZE];
        memset(sendbuf, 0, sizeof(sendbuf));
        int transfer_length = 0;
        int write_length = 0;
        long long int transfer_sum = 0;
        int percent = 0;
        while( (transfer_length = fread(sendbuf, sizeof(char), sizeof(sendbuf), uploadfile_fp)) > 0 )
        {
            if( (write_length = write(m_socketfd, sendbuf, transfer_length)) < 0 )
            {
                printf("Error! File upload fail\n");
                break;
            }
            memset(sendbuf, 0, sizeof(sendbuf));
            transfer_sum += write_length;
            //get the process
            percent = (transfer_sum & 0xff00000000000000) ? transfer_sum / (file_length / 100) : transfer_sum * 100 / file_length;
            printf("\rUpload file finished:%d%%", percent);
            fflush(stdout);
        }
        printf("\n");
        fclose(uploadfile_fp);
        uploadfile_fp = NULL;
        if( transfer_sum == 0 )
        {
            printf("Error! There is no such file\n");
            return -1;
        }
        if( percent == 100 )
        {
            return 1;
        }
    }

    return 0;
}
