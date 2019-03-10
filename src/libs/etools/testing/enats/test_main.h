#ifndef __ENATS_TEST
#define __ENATS_TEST

#include <string.h>

#include "enats.h"
#include "ecompat.h"

#if (!_WIN32)
#define __unused __attribute__((unused))
#else
#define __unused
#endif

static int64_t     total   = 1000000;

static volatile int64_t count   = 0;
static volatile int64_t dropped = 0;
static int64_t          start   = 0;
static volatile int64_t elapsed = 0;
static bool             print   = true;


void enats_pub_test();
void enats_multi_conn_test();

void enatp_nrom_test();
void enatp_lazy_test();

void server_max_collect_test();

void enats_sub_test();
void enatp_sub_test();

void enats_verify_test();
void enatp_verify_test();

void enats_join_test();
void enatp_join_test();



void ConnectedCB   (enats t, void* closure __unused);
void ClosedCB      (enats t, void* closure __unused);
void DisconnectedCB(enats t, void* closure __unused);
void ReconnectedCB (enats t, void* closure __unused);


#endif
