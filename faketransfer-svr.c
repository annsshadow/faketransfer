/*************************************************************************
	> File Name: faketransfer-svr.c
	> Author: shadowwen-annsshadow
	> Mail: cravenboy@163.com
	> Created Time: Thu 11 Aug 2016 05:15:03 PM HKT
 ************************************************************************/

#include "faketransfer.h"

int main(int argc, char *argv[])
{
    if( argc != 1)
    {
        printf("Usage:$./swserver\n");
        return 0;
    }
    //create TCP socket
    int m_socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if(m_socketfd == -1)
    {
        printf("socket failed!\n");
        return 0;
    }
    //define server IP + Port
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(SERVER_PORT);
    if(-1 == bind(m_socketfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)))
    {
        printf("Error! Bind failed\n");
        close(m_socketfd);
        m_socketfd = -1;
        return 0;
    }
    //make the port reuse
    int on = 1;
    if( -1 == setsockopt(m_socketfd, SOL_SOCKET, SO_REUSEADDR,
                         (const char*)&on, sizeof(on)) )
    {
        printf("Error! Setsockopt failed\n");
        close(m_socketfd);
        m_socketfd = -1;
        return 0;
    }
    //Listen the port
    if( -1 == listen(m_socketfd, MAX_LISTEN) )
    {
        printf("Error! Listen failed\n");
        close(m_socketfd);
        m_socketfd = -1;
        return 0;
    }

    struct sockaddr_in clientaddr;
    int clientaddr_len = sizeof(clientaddr);
    int  m_connectfd = -1;

    char peer_addr[16];
    char filename_buf[FILENAME_SIZE];
    char command_buf[9];

    for(;;)
    {
        memset(&clientaddr, 0, clientaddr_len);
        m_connectfd = accept(m_socketfd, (struct sockaddr*)&clientaddr, &clientaddr_len);
        if( m_connectfd == -1 )
        {
            printf("Error!Accept failed\n");
            continue;
        }
        memset(peer_addr, 0, sizeof(peer_addr));
        printf("You have a new connection--->[%s:%hu]\n",
               inet_ntop(AF_INET, &clientaddr.sin_addr, peer_addr, clientaddr_len),
               ntohs(clientaddr.sin_port));

        //get the filename
        int filename_len = 0;
        if( read(m_connectfd, &filename_len, sizeof(int)) < 0 )
        {
            printf("Error! Read filename_len failed\n");
            close(m_connectfd);
            m_connectfd = -1;
            continue;
        }
        memset(filename_buf, 0, FILENAME_SIZE);
        if( read(m_connectfd, filename_buf, filename_len) < 0 )
        {
            printf("Error!Read filename failed\n");
            close(m_connectfd);
            m_connectfd = -1;
            continue;
        }
        printf("filename:%s\n", filename_buf);

        //get the command
        int command_len = 0;
        if( read(m_connectfd, &command_len, sizeof(int)) < 0 )
        {
            printf("Error!Read command_len failed\n");
            close(m_connectfd);
            m_connectfd = -1;
            continue;
        }
        memset(command_buf, 0, sizeof(command_buf));
        if( read(m_connectfd, command_buf, command_len) < 0 )
        {
            printf("Error!Read command failed\n");
            close(m_connectfd);
            m_connectfd = -1;
            continue;
        }
        printf("command:%s\n", command_buf);

        //upload the file
        if( strncmp(command_buf, "upload", sizeof("download")) == 0 )
        {
            int transfer_state = fake_server_uploadfile(m_connectfd, filename_buf);
            if( transfer_state == 1 )
            {
                printf("Upload Success--->%s\n", filename_buf);
                close(m_connectfd);
                m_connectfd = -1;
                continue;
            }
            else if( transfer_state == 0)
            {
                printf("Upload Interrupt--->%s\n");
                close(m_connectfd);
                m_connectfd = -1;
                continue;
            }
            else
            {
                printf("Upload Error\n");
                close(m_connectfd);
                m_connectfd = -1;
                continue;
            }
        }
        else
        {
            printf("Wrong Command! Use upload\n");
            close(m_connectfd);
            m_connectfd = -1;
            continue;;
        }

    }
    close(m_connectfd);
    close(m_socketfd);
    m_connectfd = -1;
    m_socketfd = -1;
    return 0;
}
