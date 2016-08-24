/*************************************************************************
	> File Name: faketransfer-mt-svr-tpool.c
	> Author: annsshadow
	> Mail: cravenboy@163.com
	> Created Time: Wed 12 Aug 2016 15:16:38 PM HKT
 ************************************************************************/

#include "faketransfer-mt-svr-tpool.h"
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>

static tpool_t *tpool = NULL;

//thread main function : take task from chain and do it
static void* thread_routine()
{
    tpool_work_t *work;

    for(;;)
    {
        //if empty task chain and threadpool is open, thread block and wait for task with mutex lock
        pthread_mutex_lock(&tpool->queue_lock);
        while(!tpool->queue_head && !tpool->shutdown)
        {
            pthread_cond_wait(&tpool->queue_ready, &tpool->queue_lock);
        }

        //check if threadpool is open or shutdown and exit
        if (tpool->shutdown)
        {
            pthread_mutex_unlock(&tpool->queue_lock);
            pthread_exit(NULL);
        }

        //take task from chain and do it
        work = tpool->queue_head;
        tpool->queue_head = tpool->queue_head->next;
        pthread_mutex_unlock(&tpool->queue_lock);
        work->routine(work->arg);

        //after finish task, free params and task
        free(work->arg);
        free(work);
    }
    return NULL;
}

//create threadpool
int tpool_create(int max_thread_num)
{
    int index_num;

    //initial threadpool struct
    tpool = calloc(1, sizeof(tpool_t));
    if (!tpool)
    {
        printf("Error! %s: calloc tpool failed\n", __FUNCTION__);
        exit(1);
    }

    //initial task chain
    tpool->max_thr_num = max_thread_num;
    tpool->shutdown = 0;
    tpool->queue_head = NULL;
    tpool->queue_tail = NULL;

    //initial mutex lock
    if (pthread_mutex_init(&tpool->queue_lock, NULL) != 0)
    {
        printf("Error! %s: pthread_mutex_init failed, errno:%d, error:%s\n",
               __FUNCTION__, errno, strerror(errno));
        exit(-1);
    }

    //initial condition
    if (pthread_cond_init(&tpool->queue_ready, NULL) != 0 )
    {
        printf("Error! %s: pthread_cond_init failed, errno:%d, error:%s\n",
               __FUNCTION__, errno, strerror(errno));
        exit(-1);
    }

    //create thread worker
    tpool->thr_id = calloc(max_thread_num, sizeof(pthread_t));
    if (!tpool->thr_id)
    {
        printf("Error! %s: calloc thr_id failed\n", __FUNCTION__);
        exit(1);
    }
    for (index_num = 0; index_num < max_thread_num; ++index_num)
    {
        if (pthread_create(&tpool->thr_id[index_num], NULL, thread_routine, NULL) != 0)
        {
            printf("Error! %s:pthread_create failed, errno:%d, error:%s\n", __FUNCTION__, errno, strerror(errno));
            exit(-1);
        }
    }
    return 0;
}

//destroy threadpool
void tpool_destroy()
{
    int i;
    tpool_work_t *member;

    //make "shutdown threadpool" come into force
    if (tpool->shutdown)
    {
        return;
    }
    tpool->shutdown = 1;

    //broadcast all blocked threads with mutex lock
    pthread_mutex_lock(&tpool->queue_lock);
    pthread_cond_broadcast(&tpool->queue_ready);
    pthread_mutex_unlock(&tpool->queue_lock);

    //recycle thread resource
    for (i = 0; i < tpool->max_thr_num; ++i)
    {
        pthread_join(tpool->thr_id[i], NULL);
    }

    //release the threadID array
    free(tpool->thr_id);

    //release the rest task
    while(tpool->queue_head)
    {
        member = tpool->queue_head;
        tpool->queue_head = tpool->queue_head->next;
        free(member->arg);
        free(member);
    }

    //destroy mutex lock and condition
    pthread_mutex_destroy(&tpool->queue_lock);
    pthread_cond_destroy(&tpool->queue_ready);

    //release the whole threadpool struct
    free(tpool);
}

//add task to chain
int tpool_add_task(void*(*routine)(void*), void *arg)
{
    tpool_work_t *work;

    //check params
    if (!routine)
    {
        printf("Error! %s:Invalid argument\n", __FUNCTION__);
        return -1;
    }

    //alloc new space for work
    work = malloc(sizeof(tpool_work_t));
    if (!work)
    {
        printf("Error! %s:malloc work failed\n", __FUNCTION__);
        return -1;
    }
    work->routine = routine;
    work->arg = arg;
    work->next = NULL;

    //add task to chain with mutex lock
    pthread_mutex_lock(&tpool->queue_lock);
    if ( !tpool->queue_head )
    {
        tpool->queue_head = work;
        tpool->queue_tail = work;
    }
    else
    {
        tpool->queue_tail->next = work;
        tpool->queue_tail = work;
    }

    //send signal to all thread : have new task
    pthread_cond_signal(&tpool->queue_ready);
    pthread_mutex_unlock(&tpool->queue_lock);
    return 0;
}
