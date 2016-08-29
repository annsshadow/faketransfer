/*************************************************************************
	> File Name: faketransfer-mt-cli-fun.c
	> Author: annsshadow
	> Mail: cravenboy@163.com
	> Created Time: Wed 12 Aug 2016 15:15:38 PM HKT
 ************************************************************************/

#include "faketransfer-mt-cli-fun.h"

extern char *g_mbegin;
extern int g_port;

int fileinfo_len = sizeof(struct fileinfo);
int head_len = sizeof(struct head);
socklen_t sockaddr_len = sizeof(struct sockaddr);

/**
 * [new_fb_head create new file block header info with new offset]
 * @param  filename [file name with absolute path]
 * @param  freeid   [connection suffix]
 * @param  offset   [block offset in source file]
 * @return          [struct head * p_fhead]
 */
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

/**
 * [send_fileinfo description]
 * @param sock_fd [connection socket fd]
 * @param fname   [file name with absolute path]
 * @param p_fstat [file property]
 * @param p_finfo [file info]
 * @param flag    [whether the last send block is normal(flag==0) or not(flag==1)]
 */
void send_fileinfo(int sock_fd, char *fname, struct stat* p_fstat, struct fileinfo *p_finfo, unsigned int *p_last_bs)
{
    //get file name and size
    bzero(p_finfo, fileinfo_len);
    strcpy(p_finfo->filename, fname);
    p_finfo->filesize = p_fstat->st_size;

    //get blocks count according to BLOCKSIZE
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

    //send request type and file info
    char send_buf[100] = {0};
    int type = 0;
    memcpy(send_buf, &type, INT_SIZE);
    memcpy(send_buf + INT_SIZE, p_finfo, fileinfo_len);
    send(sock_fd, send_buf, fileinfo_len + INT_SIZE, 0);

    //print the info
    printf("-------- Fileinfo -------\n");
    printf("Filename= %s\nFilesize= %u\nCount= %d\nBlocksize= %u\n", p_finfo->filename, p_finfo->filesize, p_finfo->count, p_finfo->bs);
    printf("-------------------------\n");
    return;
}

/**
 * [send_fileblock send file block and free params, close connection fd]
 * @param  args [type:struct head * p_fhead]
 * @return      [NULL]
 */
void * send_fileblock(void *args)
{
    //get and print block info
    struct head * p_fhead = (struct head *)args;
    printf("------- blockhead -------\n");
    printf("Filename= %s\nFiledata id= %d\nOffset= %u\nbs= %u\n", p_fhead->filename, p_fhead->id, p_fhead->offset, p_fhead->bs);
    printf("-------------------------\n");

    int sock_fd = client_connect_init(SERVER_IP);
    //set_fd_noblock(sock_fd);

    //send request type and block info
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

    //send file block data
    printf("Thread : start to send fileblock\n");
    int index_count = 0, send_size = 0, send_count = p_fhead->bs / SEND_SIZE;
    char *fp = g_mbegin + p_fhead->offset;
    for(index_count = 0; index_count < send_count; index_count++)
    {
        if( (send_size = send(sock_fd, fp, SEND_SIZE, 0)) == SEND_SIZE)
        {
            fp += SEND_SIZE;
        }
        else
        {
            printf("Error! send_fileblock send_size = %d", send_size);
        }
    }

    //close connection fd and free head info
    printf("Thread : have send a fileblock\n");
    close(sock_fd);
    free(args);
    return NULL;
}

/**
 * [client_connect_init initial client connection to server]
 * @param  ip [server IP address]
 * @return    [success:socket fd, fail:-1]
 */
int client_connect_init(char *ip)
{
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(g_port);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    if (connect(sock_fd, (struct sockaddr *)&server_addr, sockaddr_len) < 0)
    {
        perror("Error! client connect initial");
        exit(-1);
    }
    return sock_fd;
}

void set_fd_noblock(int fd)
{
    int opts;
    opts = fcntl(fd, F_GETFL);
    if (opts < 0)
    {
        fprintf(stderr, "fcntl get fd options fail\n");
        return;
    }
    opts = opts | O_NONBLOCK;
    if (fcntl(fd, F_SETFL, opts) < 0)
    {
        fprintf(stderr, "fcntl set fd noblocking fail\n");
        return;
    }
    return;
}
