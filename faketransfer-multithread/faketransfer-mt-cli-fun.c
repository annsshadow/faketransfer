/*************************************************************************
	> File Name: faketransfer-mt-cli-fun.c
	> Author: annsshadow
	> Mail: cravenboy@163.com
	> Created Time: Wed 12 Aug 2016 15:15:38 PM HKT
 ************************************************************************/

#include "faketransfer-mt-cli-fun.h"

//信息fd
int info_fd;
//外部变量
extern char *mbegin;
extern int port;
//各种结构体长度
int fileinfo_len = sizeof(struct fileinfo);
socklen_t sockaddr_len = sizeof(struct sockaddr);
int head_len = sizeof(struct head);

/*
int createfile(char *filename, unsigned int size)
{
    int fd = open(filename, O_RDWR | O_CREAT);
    fchmod(fd, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    lseek(fd, size - 1, SEEK_SET);
    write(fd, "", 1);
    close(fd);
    return 0;
}
*/

//创建新的结构体保存文件块信息
//为了传输下一个文件
struct head * new_fb_head(char *filename, int freeid, unsigned int *offset)
{
    struct head * p_fhead = (struct head *)malloc(head_len);
    bzero(p_fhead, head_len);
    strcpy(p_fhead->filename, filename);
    p_fhead->id = freeid;
    p_fhead->offset = *offset;
    p_fhead->bs = BLOCKSIZE;
    *offset += BLOCKSIZE;
    return p_fhead;
}

//发送文件信息
void send_fileinfo(int sock_fd, char *fname, struct stat* p_fstat, struct fileinfo *p_finfo, unsigned int *p_last_bs)
{
    //初始化fileinfo
    bzero(p_finfo, fileinfo_len);
    strcpy(p_finfo->filename, fname);
    p_finfo->filesize = p_fstat->st_size;

    //根据是否可以标准分块调整分块数量
    int count = p_fstat->st_size / BLOCKSIZE;
    if(p_fstat->st_size % BLOCKSIZE == 0)
    {
        p_finfo->count = count;
    }
    else
    {
        p_finfo->count = count + 1;
        *p_last_bs = p_fstat->st_size - BLOCKSIZE * count;
    }
    p_finfo->bs = BLOCKSIZE;

    //发送type和文件信息
    char send_buf[100] = {0};
    int type = 0;
    memcpy(send_buf, &type, INT_SIZE);
    memcpy(send_buf + INT_SIZE, p_finfo, fileinfo_len);
    send(sock_fd, send_buf, fileinfo_len + INT_SIZE, 0);

    //发送之后显示文件信息
    printf("-------- Fileinfo -------\n");
    printf("Filename= %s\nFilesize= %u\nCount= %d\nBlocksize= %u\n", p_finfo->filename, p_finfo->filesize, p_finfo->count, p_finfo->bs);
    printf("-------------------------\n");
    return;
}

//发送文件数据
void * send_filedata(void *args)
{
    //显示文件块信息
    struct head * p_fhead = (struct head *)args;
    printf("------- blockhead -------\n");
    printf("Filename= %s\nFiledata id= %d\nOffset= %u\nbs= %u\n", p_fhead->filename, p_fhead->id, p_fhead->offset, p_fhead->bs);
    printf("-------------------------\n");

    int sock_fd = Client_init(SERVER_IP);
    //set_fd_noblock(sock_fd);

    //发送type和数据块头部
    char send_buf[100] = {0};
    int type = 255;
    memcpy(send_buf, &type, INT_SIZE);
    memcpy(send_buf + INT_SIZE, p_fhead, head_len);
    int sendsize = 0, len = head_len + INT_SIZE;
    char *p = send_buf;
    while(1)
    {
        if((send(sock_fd, p, 1, 0) > 0))
        {
            ++sendsize;
            if(sendsize == len)
                break;
            ++p;
        }
    }

    //开始发送数据块
    printf("Thread : start to send fileblock\n");
    int i = 0, send_size = 0, num = p_fhead->bs / SEND_SIZE;
    char *fp = mbegin + p_fhead->offset;
    for(i = 0; i < num; i++)
    {
        if( (send_size = send(sock_fd, fp, SEND_SIZE, 0)) == SEND_SIZE)
        {
            fp += SEND_SIZE;
            //若取消注释会在屏幕显示超多_(:з」∠)_
            //printf("fp = %p ; a SEND_SIZE ok\n", fp);
        }
        else
        {
            //如果挂的多了，这里也会显示超多_(:з」∠)_
            printf("send_size = %d ;  a SEND_SIZE erro\n", send_size);
        }
    }

    printf("Thread : have send a fileblock\n");
    close(sock_fd);
    free(args);
    return NULL;
}

//初始化客户端
int Client_init(char *ip)
{
    //创建socket
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    //构建地址结构体
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    //连接服务器
    if (connect(sock_fd, (struct sockaddr *)&server_addr, sockaddr_len) < 0)
    {
        perror("connect error");
        exit(-1);
    }
    return sock_fd;
}

//设置fd非阻塞
void set_fd_noblock(int fd)
{
    int flag = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flag | O_NONBLOCK);
    return;
}
