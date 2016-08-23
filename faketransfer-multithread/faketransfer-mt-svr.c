/*************************************************************************
	> File Name: faketransfer-mt-svr.c
	> Author: annsshadow
	> Mail: cravenboy@163.com
	> Created Time: Wed 12 Aug 2016 15:18:28 PM HKT
 ************************************************************************/
#include "faketransfer-mt-svr-fun.h"
#include "faketransfer-mt-svr-tpool.h"

int main(int argc, char **argv)
{
    printf("################# FAKETRANSFER-SERVER #################\n");
    int port = PORT;
    if (argc > 1)
        port = atoi(argv[1]);

    //根据全局配置创建线程池
    if (tpool_create(THREAD_NUM) != 0)
    {
        printf("tpool_create failed\n");
        exit(-1);
    }
    printf("--- Thread Pool Ready ---\n");

    //初始化服务端
    int listenfd = Server_init(port);
    socklen_t sockaddr_len = sizeof(struct sockaddr);

    //初始化epoll
    static struct epoll_event ev, events[EPOLL_SIZE];
    int epfd = epoll_create(EPOLL_SIZE);
    ev.events = EPOLLIN;
    ev.data.fd = listenfd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev);

    //等待连接
    for(;;)
    {
        //被激活的连接数量
        int events_count = epoll_wait(epfd, events, EPOLL_SIZE, -1);
        int i = 0;

        //接收被激活的连接，添加work到工作队列
        for(; i < events_count; i++)
        {
            if(events[i].data.fd == listenfd)
            {
                int connection_fd;
                struct sockaddr_in  clientaddr;
                while( ( connection_fd = accept(listenfd, (struct sockaddr *)&clientaddr, &sockaddr_len) ) > 0 )
                {
                    printf("EPOLL: Received New Connection Request---connection_fd= %d\n", connection_fd);
                    struct args *p_args = (struct args *)malloc(sizeof(struct args));
                    p_args->fd = connection_fd;
                    p_args->recv_finfo = recv_fileinfo;
                    p_args->recv_fdata = recv_filedata;

                    //添加work到工作队列
                    tpool_add_work(worker, (void*)p_args);
                }
            }
        }
    }

    return 0;
}
