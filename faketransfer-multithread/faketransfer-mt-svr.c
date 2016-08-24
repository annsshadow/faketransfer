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
    printf("****** faketransfer-mt-svr ******\n");
    int port = PORT;

    //create thread pool
    if (tpool_create(THREAD_NUM) != 0)
    {
        printf("tpool_create failed\n");
        exit(-1);
    }
    printf("--- Thread Pool Ready ---\n");

    //initial server connection
    int listenfd = server_connection_init(port);
    socklen_t sockaddr_len = sizeof(struct sockaddr);

    //initial epoll
    static struct epoll_event ev, events[EPOLL_SIZE];
    int epfd = epoll_create(EPOLL_SIZE);
    ev.events = EPOLLIN;
    ev.data.fd = listenfd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev);

    //wait for connection
    for(;;)
    {
        int events_count = epoll_wait(epfd, events, EPOLL_SIZE, -1);
        int index_event = 0;
        //accept connection and add task
        for(; index_event < events_count; index_event++)
        {
            if(events[index_event].data.fd == listenfd)
            {
                int connection_fd;
                struct sockaddr_in  clientaddr;
                while( ( connection_fd = accept(listenfd, (struct sockaddr *)&clientaddr, &sockaddr_len) ) > 0 )
                {
                    printf("epoll: Received New Connection Request---connection_fd= %d\n", connection_fd);
                    struct args *p_args = (struct args *)malloc(sizeof(struct args));
                    p_args->fd = connection_fd;
                    p_args->recv_finfo = recv_fileinfo;
                    p_args->recv_fdata = recv_filedata;

                    tpool_add_task(worker, (void*)p_args);
                }
            }
        }
    }

    return 0;
}
