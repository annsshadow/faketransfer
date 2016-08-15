/*************************************************************************
	> File Name: fakedownload.c
	> Author: shadowwen-annsshadow
	> Mail: cravenboy@163.com
	> Created Time: Sat 13 Aug 2016 07:47:48 PM HKT
 ************************************************************************/

#include "faketransfer.h"

/**
  *success:1,interrupt:0,fail:-1
  */
int fake_server_downloadfile(int m_connectfd, char *filename_buf)
{
    //check the param
    if( filename_buf == NULL )
    {
        return -1;
    }

    //get the source path length
    int srcpath_len = 0;
    if( read(m_connectfd, &srcpath_len, sizeof(int)) < 0 )
    {
        printf("Error! Read file_path failed\n");
        return -1;
    }
    //printf("srcpath_len=%d\n",srcpath_len);
    //get the source path length
    char src_file_path[FILENAME_SIZE];
    memset(src_file_path, 0, FILENAME_SIZE);
    if( read(m_connectfd, src_file_path, srcpath_len) < 0 )
    {
        printf("Error! Read file_path failed\n");
        return -1;
    }
    printf("Source file path--->%s\n", src_file_path);

    //open file by read
    FILE* downloadfile_fp = fopen(src_file_path, "r");
    if( downloadfile_fp == NULL )
    {
        perror(src_file_path);
        return -1;
    }
    else
    {
        //get the length of file
        struct stat srcfile_stat;
        if( stat(src_file_path, &srcfile_stat) == -1)
        {
            perror(src_file_path);
            fclose(downloadfile_fp);
            downloadfile_fp = NULL;
            return -1;
        }
        long long int file_length = srcfile_stat.st_size;

        //send the file length
        if( write(m_connectfd, &file_length, sizeof(int)) < 0 )
        {
            printf("Error! Send file_length fail\n");
            fclose(downloadfile_fp);
            downloadfile_fp = NULL;
            return -1;
        }

        //start to download file
        char sendbuf[SEND_SIZE];
        memset(sendbuf, 0, sizeof(sendbuf));
        int transfer_length = 0;
        int send_length = 0;
        long long int transfer_sum = 0;
        while( (transfer_length = fread(sendbuf, 1, SEND_SIZE, downloadfile_fp)) > 0 )
        {
            if( (send_length = send(m_connectfd, sendbuf, transfer_length, MSG_NOSIGNAL)) < 0 )
            {
                perror(filename_buf);
                break;
            }
            memset(sendbuf, 0, sizeof(sendbuf));
            transfer_sum += send_length;
        }

        fclose(downloadfile_fp);
        downloadfile_fp = NULL;
        //printf("transfer_length=%d\n",transfer_length);
        //success
        if( transfer_sum == file_length )
            return 1;
    }
    //interrupt
    return 0;
}

/**
  *success:1,interrupt:0,fail:-1
  */
int fake_client_downloadfile(int m_socketfd, char *source_file_path, char *dest_file_path)
{
    //check the param
    if( source_file_path == NULL || dest_file_path == NULL )
    {
        return -1;
    }
    char *src_file_path = source_file_path;
    char *dst_file_path = dest_file_path;

    //send the source path and length
    int srcpath_len = strlen(src_file_path) + 1;
    printf("Source path length--->%d\n", srcpath_len);
    if( write(m_socketfd, &srcpath_len, sizeof(int)) < 0 )
    {
        printf("Error! Write file_path len failed\n");
        return -1;
    }
    /*printf("srcpath_len=%d\n",srcpath_len);*/
    if( write(m_socketfd, src_file_path, srcpath_len) < 0 )
    {
        printf("Error! Write file_path failed\n");
        return -1;
    }

    //open file by write
    FILE *downloadfile_fp = fopen(dst_file_path, "w");
    if(downloadfile_fp == NULL)
    {
        printf("Error! Open download dst file failed\n");
        return -1;
    }
    else
    {
        //get the file length
        long long int file_length = 0;
        if( read(m_socketfd, &file_length, sizeof(int)) < 0 )
        {
            printf("Error! Recive file length fail\n");
            fclose(downloadfile_fp);
            downloadfile_fp = NULL;
            return -1;
        }
        printf("File length--->%lld\n", file_length);
        if( file_length == 0 )
        {
            printf("Error! There are no such file on server\n");
            return -1;
        }
        //start to download file
        char recvbuf[RECV_SIZE];
        memset(recvbuf, 0, sizeof(recvbuf));
        int transfer_length = 0;
        long long int transfer_sum = 0;
        int percent = 0;
        while((transfer_length = recv(m_socketfd, recvbuf, sizeof(recvbuf), 0)) > 0)
        {
            if( fwrite(recvbuf, sizeof(char), transfer_length, downloadfile_fp) < (unsigned int)0 )
            {
                printf("Error! File download failed\n");
                break;
            }
            memset(recvbuf, 0, sizeof(recvbuf));
            transfer_sum += transfer_length;
            //get the process
            percent = (transfer_sum & 0xff00000000000000) ? transfer_sum / (file_length / 100) : transfer_sum * 100 / file_length;
            printf("\rDownload finished--->%d%%", percent);
            fflush(stdout);

        }
        printf("\n");
        fclose(downloadfile_fp);
        downloadfile_fp = NULL;
        //printf("tranfer_length = %d\n",transfer_length);
        if( percent == 100 )
        {
            return 1;
        }
    }

    return 0;
}
