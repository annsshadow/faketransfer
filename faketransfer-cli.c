/*************************************************************************
	> File Name: faketransfer-cli.c
	> Author: shadowwen-annsshadow
	> Mail: cravenboy@163.com
	> Created Time: Thu 11 Aug 2016 05:14:46 PM HKT
 ************************************************************************/

#include "faketransfer.h"

int main(int argc, const char *argv[])
{
    if( argc != 6 )
    {
        printf("Usage:%s server-IP upload filename source_path destination_path\n", argv[0]);
        return 0;
    }

    if( strlen(argv[2]) + 1 > COMMAND_SIZE )
    {
        printf("Usage:can only use command:upload\n");
        return 0;
    }
    //get the command
    char command_buf[COMMAND_SIZE];
    memset(command_buf, 0, sizeof(command_buf));
    fake_strncpy(command_buf, argv[2], sizeof(command_buf));
    printf("Command--->%s\n", command_buf);

    if( strncmp(command_buf, "upload", sizeof("upload")) != 0)
    {
        printf("Usage:%s can only use upload\n", argv[0]);
        return 0;
    }

    //create TCP socket
    int m_socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if( m_socketfd == -1 )
    {
        printf("Error! Create socket failed!\n");
        return 0;
    }
    //define server IP + Port
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    int retval = inet_pton(AF_INET, argv[1], &serveraddr.sin_addr);
    if( retval == -1 )
    {
        printf("Error! inet_pton\n");
        return 0;
    }
    else if( retval == 0 )
    {
        printf("Error! Wrong address! such as:123.123.123.123\n");
        return 0;
    }
    serveraddr.sin_port = htons(SERVER_PORT);
    //connect to server
    if( -1 == connect(m_socketfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) )
    {
        printf("Error! Connect server failed!\n");
        close(m_socketfd);
        m_socketfd = -1;
        return 0;
    }
    //get the filename
    char filename_buf[FILENAME_SIZE];
    memset(filename_buf, 0, FILENAME_SIZE);
    fake_strncpy(filename_buf, argv[3], sizeof(filename_buf));
    printf("Filename--->%s\n", filename_buf);

    //send the file name
    if( fake_send_string(m_socketfd, filename_buf) < 0 )
    {
        printf("Error! Write filename failed!\n");
        close(m_socketfd);
        m_socketfd = -1;
        return 0;
    }

    //send the command
    if( fake_send_string(m_socketfd, command_buf) < 0 )
    {
        printf("Error! Write command failed!\n");
        close(m_socketfd);
        m_socketfd = -1;
        return 0;
    }

    //get the destination path
    char dst_file_path[FILENAME_SIZE];
    memset(dst_file_path, 0, FILENAME_SIZE);
    fake_strncpy(dst_file_path, argv[5], sizeof(dst_file_path));
    fake_strncat(dst_file_path, filename_buf, sizeof(dst_file_path));

    //get the source path
    char src_file_path[FILENAME_SIZE];
    memset(src_file_path, 0, FILENAME_SIZE);
    fake_strncpy(src_file_path, argv[4], sizeof(src_file_path));
    fake_strncat(src_file_path, filename_buf, sizeof(src_file_path));

    //upload file
    if( strncmp(command_buf, "upload", sizeof(command_buf)) == 0 )
    {
        int ret = fake_client_uploadfile(m_socketfd, src_file_path, dst_file_path);
        if( ret == 1 )
            printf("Upload Success--->%s\n", filename_buf);
        else if( ret == 0 )
            printf("Upload Interrupt--->%s\n", filename_buf);
        else	//-1
        {
            printf("Upload Error\n");
            close(m_socketfd);
            m_socketfd = -1;
            return 0;
        }
    }
    else
    {
        printf("Wrong Command! Use upload\n");
        close(m_socketfd);
        m_socketfd = -1;
        return 0;
    }
    close(m_socketfd);
    m_socketfd = -1;
    return 0;
}

