#ifndef __EB64_TEST__
#define __EB64_TEST__

#include <stdio.h>
#include <stdlib.h>
#include "echan.h"

#ifdef  ETHREAD_NOT_USING_PTHREAD_MINGEWIN_IN_ECHAN
#define ETHREAD_NOT_USING_PTHREAD_MINGEWIN
#endif

#include "ethread.h"

void assert_true(int expression, echan chan, char* msg);
void pass();
void show_pass();

i64 utils_timer_now_ms();

void echan_new_test();
void echan_close_test();
void echan_send_test();

void echan_bin_test();
void echan_int_test();
void echan_double_test();
void echan_srt_test();
void echan_ptr_test();
void echan_obj_test();

void echan_sigs_test();

void test_chan_multi();
void test_chan_multi2();

void echan_time_recv_test();

void echan_performance_test();

#endif
