/*************************************************************************
	> File Name: faketransfer-mt-svr-fun.c
	> Author: annsshadow
	> Mail: cravenboy@163.com
	> Created Time: Wed 12 Aug 2016 15:16:58 PM HKT
 ************************************************************************/

#include "faketransfer-mt-svr-fun.h"

int connection_suffix = 0;
struct conn g_connection[CONN_MAX];
pthread_mutex_t conn_lock = PTHREAD_MUTEX_INITIALIZER;

int fileinfo_len = sizeof(struct fileinfo);
int head_len = sizeof(struct head);
int conn_len = sizeof(struct conn);
socklen_t sockaddr_len = sizeof(struct sockaddr);

/**
 * [create_file create a new file on svr]
 * @param  filename [filename with absolute path]
 * @param  size     [file size]
 * @return          [success:0]
 */
int create_file(char *filename, unsigned int size)
{
    int fd = open(filename, O_RDWR | O_CREAT);
    fchmod(fd, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    lseek(fd, size - 1, SEEK_SET);
    write(fd, "", 1);
    close(fd);
    return 0;
}

/**
 * [worker thread main work : receive file info or file block according to request type]
 * @param  argc [thread_param]
 * @return      [NULL]
 */
void * worker(void *argc)
{
    //receive request type
    struct args *thread_param = (struct args *)argc;
    int conn_fd = thread_param->fd;
    char type_buf[INT_SIZE] = {0};
    char *p_type = type_buf;
    int recv_size = 0;
    for(;;)
    {
        if( recv(conn_fd, p_type, 1, 0) == 1 )
        {
            ++recv_size;
            if(recv_size == INT_SIZE)
                break;
            ++p_type;
        }
    }
    uint32_t int_32 = 0;
    memcpy(&int_32, type_buf, 4);
    int type = int_32;

    //switch to receive file info or file block
    switch (type)
    {
    case 0:
        printf("## Worker ##\nType %d: the work is receiving fileinfo\n", type);
        thread_param->recv_finfo(conn_fd);
        break;
    case 255:
        printf("## Worker ##\nType %d: the work is receiving fileblock\n", type);
        thread_param->recv_fdata(conn_fd);
        break;
    default:
        printf("Error! Unknown request type!\n");
        return NULL;
    }

    return NULL;
}

/**
 * [recv_fileinfo receive file info, use mmap and g_connection, send back connection_suffix]
 * @param sockfd [server socket file descriptor]
 */
void recv_fileinfo(int sockfd)
{
    //get the file info
    char fileinfo_buf[100] = {0};
    bzero(fileinfo_buf, fileinfo_len);
    int index_info = 0;
    for(index_info = 0; index_info < fileinfo_len; index_info++)
    {
        recv(sockfd, &fileinfo_buf[index_info], 1, 0);
    }

    struct fileinfo finfo;
    memcpy(&finfo, fileinfo_buf, fileinfo_len);

    printf("------- Fileinfo -------\n");
    printf("Filename = %s\nFilesize = %u\nCount = %d\nBs = %u\n", finfo.filename, finfo.filesize, finfo.count, finfo.bs);
    printf("------------------------\n");

    //create / open file
    char filepath[100] = {0};
    strcpy(filepath, finfo.filename);
    create_file(filepath, finfo.filesize);
    int file_fd = 0;
    if((file_fd = open(filepath, O_RDWR)) == -1 )
    {
        printf("Error! create_file\n");
        exit(-1);
    }

    //use mmap and close file fd
    char *map = (char *)mmap(NULL, finfo.filesize, PROT_WRITE | PROT_READ, MAP_SHARED, file_fd, 0);
    close(file_fd);

    //add a new connection to g_connection[] with mutex lock
    pthread_mutex_lock(&conn_lock);
    printf("--- receive fileinfo : Lock conn_lock, enter g_connection[] ---\n");
    while( g_connection[connection_suffix].used )
    {
        ++connection_suffix;
        connection_suffix = connection_suffix % CONN_MAX;
    }
    bzero(&g_connection[connection_suffix].filename, FILENAME_MAXLEN);
    g_connection[connection_suffix].info_fd = sockfd;
    strcpy(g_connection[connection_suffix].filename, finfo.filename);
    g_connection[connection_suffix].filesize = finfo.filesize;
    g_connection[connection_suffix].count = finfo.count;
    g_connection[connection_suffix].bs = finfo.bs;
    g_connection[connection_suffix].mbegin = map;
    g_connection[connection_suffix].recvcount = 0;
    g_connection[connection_suffix].used = 1;
    pthread_mutex_unlock(&conn_lock);
    printf("--- receive fileinfo : Unock conn_lock, exit g_connection[] ---\n");

    //send connection_suffix(g_connection[]) to client as the ack of every block later
    char connection_suffix_buf[INT_SIZE] = {0};
    memcpy(connection_suffix_buf, &connection_suffix, INT_SIZE);
    send(sockfd, connection_suffix_buf, INT_SIZE, 0);
    uint32_t int_32 = 0;
    memcpy(&int_32, connection_suffix_buf, 4);
    int connection_suffix = int_32;
    printf("connection_suffix = %d\n", connection_suffix);

    return;
}

/**
 * [recv_filedata receive file blocks, head info and munmap]
 * @param sockfd [server socket file descriptor]
 */
void recv_filedata(int sockfd)
{
    //get block head info
    int recv_size = 0;
    char head_buf[100] = {0};
    char *p_head = head_buf;
    for(;;)
    {
        if( recv(sockfd, p_head, 1, 0) == 1 )
        {
            ++recv_size;
            if(recv_size == head_len)
                break;
            ++p_head;
        }
    }
    struct head fhead;
    memcpy(&fhead, head_buf, head_len);
    int recv_id = fhead.id;

    //compute the begin address of this file block in mmap
    unsigned int recv_offset = fhead.offset;
    char *map_block_begin = g_connection[recv_id].mbegin + recv_offset;
    printf("------- Blockhead -------\n");
    printf("Filename = %s\nFiledata id = %d\nOffset=%u\nBs = %u\nstart addr= %p\n", fhead.filename, fhead.id, fhead.offset, fhead.bs, map_block_begin);
    printf("-------------------------\n");

    //get block data and write to mmap
    unsigned int remain_size = fhead.bs;
    int have_recv_size = 0;
    while(remain_size > 0)
    {
        if((have_recv_size = recv(sockfd, map_block_begin, RECVBUF_SIZE, 0)) > 0)
        {
            map_block_begin += have_recv_size;
            remain_size -= have_recv_size;
        }
    }
    printf("------------- Have receive a fileblock -------------\n");

    //increase recv_count with mutex lock, if it is the last one, munmap the file and release the g_connection and close the connection
    pthread_mutex_lock(&conn_lock);
    g_connection[recv_id].recvcount++;
    if(g_connection[recv_id].recvcount == g_connection[recv_id].count)
    {
        munmap((void *)g_connection[recv_id].mbegin, g_connection[recv_id].filesize);

        printf("------------- OK! Have receive a File -------------\n ");

        int file_fd = g_connection[recv_id].info_fd;
        close(file_fd);
        bzero(&g_connection[recv_id], conn_len);
    }
    pthread_mutex_unlock(&conn_lock);
    //the main connection of client
    close(sockfd);
    return;
}

/**
 * [server_connection_init initial server connection]
 * @param  port [server port]
 * @return      [socket fd]
 */
int server_connection_init(int port)
{
    int listen_fd;
    struct sockaddr_in server_addr;
    if((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        fprintf(stderr, "Creating server socket failed.");
        exit(-1);
    }
    set_fd_noblock(listen_fd);

    bzero(&server_addr, sockaddr_len);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(listen_fd, (struct sockaddr *) &server_addr, sockaddr_len) == -1)
    {
        fprintf(stderr, "Server bind failed.");
        exit(-1);
    }

    if(listen(listen_fd, LISTEN_QUEUE_LEN) == -1)
    {
        fprintf(stderr, "Server listen failed.");
        exit(-1);
    }
    return listen_fd;
}

/**
 * [set_fd_noblock set fd with option O_NOBLOCK]
 * @param fd [file descriptor]
 */
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
