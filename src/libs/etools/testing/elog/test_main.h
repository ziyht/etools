#ifndef __ELOG_TEST__
#define __ELOG_TEST__

#define ELOG_MUTE 0     // set to 1 to test mute contrl

#include <stdio.h>
#include "elog.h"

void elog_basic_test();
void elog_level_test();
void elog_buffer_test();
void elog_rtt_test();
void elog_opts_test();
void elog_multi_thread_test();

#endif
