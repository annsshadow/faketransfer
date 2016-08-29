/*************************************************************************
	> File Name: faketransfer-mt-svr-tpool.h
	> Author: annsshadow
	> Mail: cravenboy@163.com
	> Created Time: Wed 12 Aug 2016 15:17:38 PM HKT
 ************************************************************************/

#ifndef FAKETRANSFER_MT_SERVER_THREAD_POOL
#define FAKETRANSFER_MT_SERVER_THREAD_POOL

#include <pthread.h>

//task node
typedef struct tpool_work
{
    void*               (*routine)(void*);  //task function
    void                *arg;               //task params
    struct tpool_work   *next;
} tpool_work_t;

//threadpool
typedef struct tpool
{
    int             shutdown;               //button to destroy threadpool
    int             max_thr_num;            //the max number of thread in pool
    pthread_t       *thr_id;                //the head address for threadID[] array
    tpool_work_t    *queue_head;            //head of task
    tpool_work_t    *queue_tail; 		    //tail of task
    pthread_mutex_t queue_lock;
    pthread_cond_t  queue_ready;
} tpool_t;

/**
 * [tpool_create create threadpool]
 * @param  max_thr_num [max number of threadpool]
 * @return             [success:0, fail:-1]
 */
int tpool_create(int max_thr_num);

/**
 * [tpool_destroy free thread, threadID array, task, lock, condition, tpool struct]
 */
void tpool_destroy();

/**
 * [tpool_add_task add task to chain]
 * @param  routine [thread main function : take task from chain and do it]
 * @param  arg [thread params]
 * @return             [success:0, fail:-1]
 */
int tpool_add_task(void*(*routine)(void*), void *arg);

#endif
