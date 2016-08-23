/*************************************************************************
	> File Name: faketransfer-mt-svr-fun.c
	> Author: annsshadow
	> Mail: cravenboy@163.com
	> Created Time: Wed 12 Aug 2016 15:16:58 PM HKT
 ************************************************************************/

#include "faketransfer-mt-svr-fun.h"

//gconn[]�����������������Ϣ
int freeid = 0;
struct conn gconn[CONN_MAX];
pthread_mutex_t conn_lock = PTHREAD_MUTEX_INITIALIZER;
//���ֽṹ�峤��
int fileinfo_len = sizeof(struct fileinfo);
socklen_t sockaddr_len = sizeof(struct sockaddr);
int head_len = sizeof(struct head);
int conn_len = sizeof(struct conn);

//����һ�����ļ�
int createfile(char *filename, unsigned int size)
{
    int fd = open(filename, O_RDWR | O_CREAT);
    fchmod(fd, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    lseek(fd, size - 1, SEEK_SET);
    write(fd, "", 1);
    close(fd);
    return 0;
}

//����typeֵ�ַ��ļ���Ϣ/���ݵĹ����߳�
void * worker(void *argc)
{
    struct args *pw = (struct args *)argc;
    int conn_fd = pw->fd;
    //���չ�������
    char type_buf[INT_SIZE] = {0};
    char *p = type_buf;
    int recv_size = 0;
    for(;;)
    {
        if( recv(conn_fd, p, 1, 0) == 1 )
        {
            ++recv_size;
            if(recv_size == INT_SIZE)
                break;
            ++p;
        }
    }

    //int type = *((int*)type_buf);
    uint32_t test = 0;
    memcpy(&test, type_buf, 4);
    int type = test;
    switch (type)
    {
    //�ļ���Ϣ
    case 0:
        printf("## Worker ##\nType %d: the work is receiving fileinfo\n", type);
        pw->recv_finfo(conn_fd);
        break;
    //�ļ�����
    case 255:
        printf("## Worker ##\nType %d: the work is receiving fileblock\n", type);
        pw->recv_fdata(conn_fd);
        break;
    default:
        printf("Error! Unknown type!\n");
        return NULL;
    }

    return NULL;
}

//�����ļ���Ϣ
void recv_fileinfo(int sockfd)
{
    //�����ļ���Ϣ
    char fileinfo_buf[100] = {0};
    bzero(fileinfo_buf, fileinfo_len);
    int n = 0;
    for(n = 0; n < fileinfo_len; n++)
    {
        recv(sockfd, &fileinfo_buf[n], 1, 0);
    }

    struct fileinfo finfo;
    memcpy(&finfo, fileinfo_buf, fileinfo_len);

    printf("------- Fileinfo -------\n");
    printf("Filename = %s\nFilesize = %u\nCount = %d\nBs = %u\n", finfo.filename, finfo.filesize, finfo.count, finfo.bs);
    printf("------------------------\n");

    //����Ŀ���ļ�
    char filepath[100] = {0};
    strcpy(filepath, finfo.filename);
    createfile(filepath, finfo.filesize);
    int fd = 0;
    if((fd = open(filepath, O_RDWR)) == -1 )
    {
        printf("open file erro\n");
        exit(-1);
    }
    //ʹ��mmap
    char *map = (char *)mmap(NULL, finfo.filesize, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
    close(fd);

    //��gconn[]���������-��������
    pthread_mutex_lock(&conn_lock);

    printf("--- receive fileinfo : Lock conn_lock, enter gconn[] ---\n");
    while( gconn[freeid].used )
    {
        ++freeid;
        freeid = freeid % CONN_MAX;
    }
    //��¼��Ϣ��gconn
    bzero(&gconn[freeid].filename, FILENAME_MAXLEN);
    gconn[freeid].info_fd = sockfd;
    strcpy(gconn[freeid].filename, finfo.filename);
    gconn[freeid].filesize = finfo.filesize;
    gconn[freeid].count = finfo.count;
    gconn[freeid].bs = finfo.bs;
    gconn[freeid].mbegin = map;
    gconn[freeid].recvcount = 0;
    gconn[freeid].used = 1;

    pthread_mutex_unlock(&conn_lock);
    printf("--- receive fileinfo : Unock conn_lock, exit gconn[] ---\n");

    //����freeid--gconn[]�����±꣬��Ϊȷ�ϣ�ÿ���ֿ鶼��Я��id
    char freeid_buf[INT_SIZE] = {0};
    memcpy(freeid_buf, &freeid, INT_SIZE);
    send(sockfd, freeid_buf, INT_SIZE, 0);
    uint32_t test = 0;
    memcpy(&test, freeid_buf, 4);
    int freeid = test;
    printf("freeid = %d\n", freeid);

    return;
}

//�����ļ�����
void recv_filedata(int sockfd)
{
    //��ȡ�ֿ�ͷ����Ϣ
    int recv_size = 0;
    char head_buf[100] = {0};
    char *p = head_buf;
    while(1)
    {
        if( recv(sockfd, p, 1, 0) == 1 )
        {
            ++recv_size;
            if(recv_size == head_len)
                break;
            ++p;
        }
    }

    struct head fhead;
    memcpy(&fhead, head_buf, head_len);
    int recv_id = fhead.id;

    //���㱾�ļ����ݿ���map����ʼ��ַfp
    unsigned int recv_offset = fhead.offset;
    char *fp = gconn[recv_id].mbegin + recv_offset;

    printf("------- Blockhead -------\n");
    printf("Filename = %s\nFiledata id = %d\nOffset=%u\nBs = %u\nstart addr= %p\n", fhead.filename, fhead.id, fhead.offset, fhead.bs, fp);
    printf("-------------------------\n");

    //�������ݣ���mmap�ڴ�д
    unsigned int remain_size = fhead.bs;
    int have_recv_size = 0;
    while(remain_size > 0)
    {
        if((have_recv_size = recv(sockfd, fp, RECVBUF_SIZE, 0)) > 0)
        {
            fp += have_recv_size;
            remain_size -= have_recv_size;
        }
    }

    printf("------------- Have receive a fileblock -------------\n");

    //����recv_count����������һ���ֿ飬ͬ��map���ļ����ͷ�gconn
    //������
    pthread_mutex_lock(&conn_lock);
    gconn[recv_id].recvcount++;
    if(gconn[recv_id].recvcount == gconn[recv_id].count)
    {
        munmap((void *)gconn[recv_id].mbegin, gconn[recv_id].filesize);

        printf("------------- OK! Have receive a File -------------\n ");

        int fd = gconn[recv_id].info_fd;
        close(fd);
        bzero(&gconn[recv_id], conn_len);
    }
    pthread_mutex_unlock(&conn_lock);

    close(sockfd);
    return;
}

//��ʼ�������
int Server_init(int port)
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

//����fd������
void set_fd_noblock(int fd)
{
    int flag = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flag | O_NONBLOCK);
    return;
}
