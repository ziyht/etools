/// =====================================================================================
///
///       Filename:  epopen.h
///
///    Description:  new popen operations in linux
///
///        Version:  1.0
///        Created:  12/14/2017 17:08:34 PM
///       Revision:  none
///       Compiler:  gcc
///
///         Author:  Haitao Yang, joyhaitao@foxmail.com
///        Company:
///
/// =====================================================================================

#ifndef __E_POPEN_H__
#define __E_POPEN_H__

#define EPOPEN_VERSION     "epopen 1.0.2"         // fix ret wrong exit value of process

#include <stdio.h>

#include "etype.h"

typedef struct eio_s
{
    FILE* ifp;
    FILE* ofp;
}* eio;


eio epopen(constr cmd);

int epclose    (eio e);     // close handle
int epkill     (eio e);     // kill the running popen process so epclose could down immediately
int epkillclose(eio e);     // kill and close handle, you should not using it with epclose() in the same time

int eio_gets (eio e, cstr buf, int len);
int eio_tgets(eio e, cstr buf, int len, int timeout);

#endif
