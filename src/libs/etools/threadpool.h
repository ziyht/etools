#ifndef __THREAD_POOL__
#define __THREAD_POOL__

typedef struct thread_pool_s* TP;

typedef void *(*TP_cb) (void *);

/**
 * @brief TP_New - create a new thread pool
 * @return [TP]     create ok
 *         [NULL]   create failed
 * @note:
 *  1. default in 4 thread, you can use TP_SetThreadsNum() the set the max thread num
 *  2. the thread will be created when adding a task
 */
TP   TP_New();

/**
 * @brief TP_Destroy - releasing thread_pool resources
 * @param tp  - the pointor to a threadpool returned by TP_New()
 * @param opt - options of quit behavior
 *              TP_JOIN_THREADS - will join inner threads, when you call TP_Join(), it will wait for all threads to quit 
 * @note:
 *  1. this operation is non-bloking
 *  2. the inner threads those who is running task will not quit immediately, 
 *     we ensure the task(including after_oprt add by TP_AddTask()) to running 
 *     over if you not quit the whole process
 */

#define TP_JOIN_THREADS 0x01
void TP_Destroy(TP tp, int opt);

/**
 * @brief TP_SetThreadsNum - set the max threads num of a threadpool
 * @param tp  - the pointor to a threadpool returned by TP_New()
 * @param max - the max threads num to set, must > 1
 * @note: 
 *  1. it will have no effect when the num of inner exist thread is bigger than @param max
 */
void TP_SetThreadsNum(TP tp, int max);

/**
 * @brief TP_Join - wait the threadpool to quit
 * @param tp  - the pointor to a threadpool returned by TP_New()
 */
void TP_Join(TP tp);

/**
 * @brief TP_AddTask - add a task into the threadpool
 * @param tp   - the pointor to a threadpool returned by TP_New()
 * @param tag  - the tag of the add task
 *               NULL or EMPTY: the task will always be added to the threadpool, but have no tag
 *               valid str: the task will be added to the threadpool only when the threadpool not have a running or waiting task with the same tag
 * @param oprt - task cb, can not be NULL
 * @param after_oprt - after task cb, we ensure this cb will be called if the task is running
 * @param data - a private date to the task
 * @return [0] - add failed
 *         [1] - add ok
 */
int  TP_AddTask(TP tp, const char* tag, TP_cb oprt, TP_cb after_oprt, void* data);

/**
 * @brief TP_QueryTask - to query a task whether is in the threadpool or not
 * @param tp  - the pointor to a threadpool returned by TP_New()
 * @param tag - the tag of the task you want to query
 * @return [0] - this task is running over or haven't been added to the threadpool
 *         [1] - this task is running or in the waiting list
 */
int  TP_QueryTask(TP tp, const char* tag);

#endif
