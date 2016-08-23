/*************************************************************************
	> File Name: faketransfer-mt-cli.c
	> Author: annsshadow
	> Mail: cravenboy@163.com
	> Created Time: Wed 12 Aug 2016 15:15:48 PM HKT
 ************************************************************************/

#include "faketransfer-mt-cli-fun.h"

int port = PORT;	//默认Port
char *mbegin;	//map起始地址

int main(int argc, char **argv)
{
    //初始化Client
    if (argc > 1)
        port = atoi(argv[1]);
    int info_fd = Client_init(SERVER_IP);

    //输入文件名并打开文件
    char filename[FILENAME_MAXLEN] = {0};
    printf("BLOCKSIZE=  %d\n", BLOCKSIZE);
    printf("Please input filename with absolutely path: ");
    scanf("%s", filename);
    int fd = 0;
    if((fd = open(filename, O_RDWR)) == -1 )
    {
        printf("open file error ！\n");
        exit(-1);
    }

    //计时器开始
    printf("OK!Timer start!\n");
    time_t t_start, t_end;
    t_start = time(NULL);

    //发送文件信息
    struct stat filestat;
    fstat(fd, &filestat);
    unsigned int last_bs = 0;
    struct fileinfo finfo;
    send_fileinfo(info_fd, filename, &filestat, &finfo, &last_bs);

    //接收Server分配的ID
    char id_buf[INT_SIZE] = {0};
    int n = 0;
    for(n = 0; n < INT_SIZE; n++)
    {
        read(info_fd, &id_buf[n], 1);
    }
    //int freeid = *((int *)id_buf);
    uint32_t test = 0;
    memcpy(&test, id_buf, 4);
    int freeid = test;
    printf("freeid = %d\n", freeid);

    //使用mmap，同时关闭fd
    mbegin = (char *)mmap(NULL, filestat.st_size, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
    close(fd);

    //向任务队列添加任务
    //任务数为文件的分块情况num
    int j = 0, num = finfo.count;
    unsigned int offset = 0;
    pthread_t pid[num];
    memset(pid, 0, num * sizeof(pthread_t));
    int head_len = sizeof(struct head);
    //如果文件可以被标准分块-直接分块传输好
    if(last_bs == 0)
    {
        for(j = 0; j < num; j++)
        {
            struct head * p_fhead = new_fb_head(filename, freeid, &offset);
            if (pthread_create(&pid[j], NULL, send_filedata, (void *)p_fhead) != 0)
            {
                printf("%s:pthread_create failed, errno:%d, error:%s\n", __FUNCTION__, errno, strerror(errno));
                exit(-1);
            }
        }
    }
    //如果文件不能被标准分块，标准传输num-1块，然后传输最后一块，已last_bs区分
    else
    {
        for(j = 0; j < num - 1; j++)
        {
            struct head * p_fhead = new_fb_head(filename, freeid, &offset);
            if (pthread_create(&pid[j], NULL, send_filedata, (void *)p_fhead) != 0)
            {
                printf("%s:pthread_create failed, errno:%d, error:%s\n", __FUNCTION__, errno, strerror(errno));
                exit(-1);
            }
        }
        //最后一个分块
        struct head * p_fhead = (struct head *)malloc(head_len);
        bzero(p_fhead, head_len);
        strcpy(p_fhead->filename, filename);
        p_fhead->id = freeid;
        p_fhead->offset = offset;
        p_fhead->bs = last_bs;

        if (pthread_create(&pid[j], NULL, send_filedata, (void *)p_fhead) != 0)
        {
            printf("%s:pthread_create failed, errno:%d, error:%s\n", __FUNCTION__, errno, strerror(errno));
            exit(-1);
        }
    }

    //回收线程
    for (j = 0; j < num; ++j)
    {
        pthread_join(pid[j], NULL);
    }

    //计时器结束
    t_end = time(NULL);
    printf("Master prosess exit!\n");
    printf("Time cost%.0fs\n", difftime(t_end, t_start));

    return 0;
}

