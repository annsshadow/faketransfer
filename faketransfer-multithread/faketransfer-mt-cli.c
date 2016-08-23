/*************************************************************************
	> File Name: faketransfer-mt-cli.c
	> Author: annsshadow
	> Mail: cravenboy@163.com
	> Created Time: Wed 12 Aug 2016 15:15:48 PM HKT
 ************************************************************************/

#include "faketransfer-mt-cli-fun.h"

int g_port = PORT;
char *g_mbegin = NULL; //the begin address of mmap

int main(int argc, char **argv)
{
    printf("****** faketransfer-mt-cli ******\n");
    //initial client connection
    int info_fd = client_connect_init(SERVER_IP);

    //get filename and open file
    char filename[FILENAME_MAXLEN] = {0};
    printf("BLOCKSIZE=  %d\n", BLOCKSIZE);
    printf("Please input filename with absolute path: ");
    scanf("%s", filename);
    int file_fd = 0;
    if((file_fd = open(filename, O_RDWR)) == -1 )
    {
        printf("Error! Open input file\n");
        exit(-1);
    }

    //timer begine
    printf("Ready! Timer start!\n");
    time_t t_start, t_end;
    t_start = time(NULL);

    //send file info
    struct stat filestat;
    fstat(file_fd, &filestat);
    unsigned int last_bs = 0;//this is the limit of how big the file can be transfer
    struct fileinfo finfo;
    send_fileinfo(info_fd, filename, &filestat, &finfo, &last_bs);

    //receive the id(the suffix of this link in g_connection[]) from server in order to send every fileblock with that
    char suffix_buf[INT_SIZE] = {0};
    int index_to_send = 0;
    for(index_to_send = 0; index_to_send < INT_SIZE; index_to_send++)
    {
        read(info_fd, &id_buf[index_to_send], 1);
    }
    uint32_t int_32 = 0;
    memcpy(&int_32, suffix_buf, INT_SIZE);
    int connection_suffix = int_32;
    printf("connection_suffix = %d\n", connection_suffix);

    //use mmap and close the file fd
    g_mbegin = (char *)mmap(NULL, filestat.st_size, PROT_WRITE | PROT_READ, MAP_SHARED, file_fd, 0);
    close(file_fd);

    //use thread to send file blocks according to the count of blocks
    int index_count = 0, bs_count = finfo.count;
    unsigned int offset = 0;//this is the limit of how big the file can be transfer
    pthread_t pid[bs_count];
    memset(pid, 0, bs_count * sizeof(pthread_t));
    int head_len = sizeof(struct head);

    if(last_bs == 0)//if we can divide the file with BLOCKSIZE just right
    {
        for(index_count = 0; index_count < bs_count; index_count++)
        {
            struct head * p_fhead = new_fb_head(filename, connection_suffix, &offset);
            if (pthread_create(&pid[j], NULL, send_fileblock, (void *)p_fhead) != 0)
            {
                printf("Error! %s:pthread_create failed, errno:%d, error:%s\n", __FUNCTION__, errno, strerror(errno));
                exit(-1);
            }
        }
    }
    else
    {
        for(index_count = 0; index_count < bs_count - 1; index_count++)
        {
            struct head * p_fhead = new_fb_head(filename, connection_suffix, &offset);
            if (pthread_create(&pid[index_count], NULL, send_fileblock, (void *)p_fhead) != 0)
            {
                printf("Error! %s:pthread_create failed, errno:%d, error:%s\n", __FUNCTION__, errno, strerror(errno));
                exit(-1);
            }
        }
        //the last unnormal file block
        struct head * p_fhead = (struct head *)malloc(head_len);
        bzero(p_fhead, head_len);
        strcpy(p_fhead->filename, filename);
        p_fhead->id = connection_suffix;
        p_fhead->offset = offset;
        p_fhead->bs = last_bs;

        if (pthread_create(&pid[index_count], NULL, send_fileblock, (void *)p_fhead) != 0)
        {
            printf("Error! %s:pthread_create failed, errno:%d, error:%s\n", __FUNCTION__, errno, strerror(errno));
            exit(-1);
        }
    }

    //recycle the thread
    for (index_count = 0; index_count < bs_count; ++index_count)
    {
        pthread_join(pid[index_count], NULL);
    }

    //timer over
    t_end = time(NULL);
    printf("Master prosess exit successfully!\n");
    printf("Time cost : %.0fs\n", difftime(t_end, t_start));

    return 0;
}

