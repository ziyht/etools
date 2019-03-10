/// =====================================================================================
///
///       Filename:  etimer.h
///
///    Description:  a easier timer to run task
///
///        Version:  1.0
///        Created:  03/13/2017 11:00:34 AM
///       Revision:  none
///       Compiler:  gcc
///
///         Author:  Haitao Yang, joyhaitao@foxmail.com
///        Company:
///
/// =====================================================================================

#ifndef __ETIMER_H__
#define __ETIMER_H__

#include "etype.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct etloop_s* etloop;
typedef struct etimer_s
{
    void* data;
}etimer_t, * etimer;

typedef void (*etm_cb)(etimer t);

/// ------------------- etimer loop ---------------------
///
///     to get a new loop or default loop
///

etloop etloop_new(int maxthread);
etloop etloop_df (int maxthread);

void   etloop_stop(etloop loop);

/// ------------------- etimer --------------------------
///
///     etimer creator ...
///
etimer etimer_new(etloop loop);
void   etimer_destroy(etimer e);

int    etimer_start(etimer e, etm_cb cb, u64 timeout, u64 repeat);
int    etimer_stop (etimer e);

int    etimer_runing(etimer _e);

/// ------------------- etimer time ---------------------
///
///     to get the timestamp or str of now
///
i64    etimer_now();                                    // return the timestamp of now in ms
cstr   etimer_nowstr(cstr dest, int dlen);              // get the time string of now in man's role in YYYYMMDDhhmmss.[9]ns

cstr   etimer_secstr(cstr dest, int dlen, time_t sec);  // reformat timestamp to str in YYYYMMDDhhmmss

/** ---------------------------
%a 星期几的简写
%A 星期几的全称
%b 月分的简写
%B 月份的全称
%c 标准的日期的时间串
%C 年份的后两位数字
%d 十进制表示的每月的第几天
%D 月/天/年
%e 在两字符域中，十进制表示的每月的第几天
%F 年-月-日
%g 年份的后两位数字，使用基于周的年
%G 年分，使用基于周的年
%h 简写的月份名
%H 24小时制的小时
%I 12小时制的小时
%j 十进制表示的每年的第几天
%m 十进制表示的月份
%M 十时制表示的分钟数
%n 新行符
%p 本地的AM或PM的等价显示
%r 12小时的时间
%R 显示小时和分钟：hh:mm
%S 十进制的秒数
%t 水平制表符
%T 显示时分秒：hh:mm:ss
%u 每周的第几天，星期一为第一天 （值从0到6，星期一为0）
%U 第年的第几周，把星期日做为第一天（值从0到53）
%V 每年的第几周，使用基于周的年
%w 十进制表示的星期几（值从0到6，星期天为0）
%W 每年的第几周，把星期一做为第一天（值从0到53）
%x 标准的日期串
%X 标准的时间串
%y 不带世纪的十进制年份（值从0到99）
%Y 带世纪部分的十进制年份
%z，%Z 时区名称，如果不能得到时区名称则返回空字符。
%% 百分号
--------------------------- */

/**
 * @brief etimer_strfstr
 * @param dest
 * @param dlen
 * @param dfmt
 * @param from
 * @param ffmt
 * @return ok  : dest
 *         fail: null
 *
 *  etimer_strfstr()
 *      input                  output [%F %T]
 *      17:43:03               1900-01-00 17:43:03
 *      17:43:03 2018          2018-01-00 17:43:03
 *      Jun 17:43:03 2018      2018-06-00 17:43:03
 *      Jun 27 17:43:03 2018   2018-06-27 17:43:03
 *
 *
 *  etimer_strfsec()
 *      input                  output [%F %T]
 *      31723506(1y2d4h5m6s)   1971-01-03 12:05:06
 *      1530092583             2018-06-27 17:43:03
 *      1                      1970-01-01 08:00:01
 *
 *  etimer_strpsec()
 *      input                  output
 *      Jun 27 17:43:03 2018   1530092583
 *
 *
 *
 */
cstr   etimer_strfstr(cstr dest, int dlen, constr dfmt, constr from, constr ffmt);
cstr   etimer_strfsec(cstr dest, int dlen, constr dfmt, i64   stamp);               // format unix timestamp(sec)
i64    etimer_strpsec(constr from, constr ffmt);                                    // return unix timestamp(sec)
i64    etimer_strpms (constr from, constr ffmt);

/**
 * @brief etimer_elapsefstr
 * @param dest
 * @param dlen
 * @param dfmt
 * @param from
 * @param ffmt
 * @return
 *
 *
 *   etimer_elapsefstr
 *      input                       output
 *      367 04:05:06 1 [%j %T %Y]   2y 002d 04:05:06 [%Yy %jd %T]
 *      367 04:05:06 1 [%j %T %Y]   732d 04:05:06    [%jd %T]
 *      367 04:05:06 1 [%j %T %Y]   2y 52:05:06      [%Yy %T]
 *      367 04:05:06 1 [%j %T %Y]   17572:05:06      [%T]
 *
 *  etimer_elapsefsec
 *      input                       output
 *      31723506(1y2d4h5m6s)        1y 002d 04:05:06 [%Yy %jd %T]
 *      31723506                    367d 04:05:06    [%jd %T]
 *      31723506                    1y 52:05:06      [%Yy %T]
 *      31723506                    8812:05:06       [%T]
 *      1                           0y 000d 00:00:01 [%Yy %jd %T]
 *
 *  etimer_elapsepsec
 *      input                       output
 *   1  2 04:05:06 1 [%j %T %Y]     31723506   use %j as days, not yday likely in tm, this case is specific if only using %Y %j %T in ffmt
 *   2  Jan 0 00:00:03 0000         -1
 *   3  Jan 1 00:00:03 0000         3
 *   4  Jan 2 02:00:03 0000         93603
 */
cstr   etimer_elapsefstr(cstr dest, int dlen, constr dfmt, constr from, constr ffmt);
cstr   etimer_elapsefsec(cstr dest, int dlen, constr dfmt, i64 sec);
i64    etimer_elapsepsec(constr from, constr ffmt);
i64    etimer_elapsepms (constr from, constr ffmt);

//int    etimer_sync(constr server, int timeout);         // todo

/// ------------------- etimer sync tool ----------------
///
///     this is a tool for external using, to get a sync
/// information between local machine to the target server,
/// using ntp protocol.
///

typedef struct etimer_sync_stat_s{
    int    status;          // 1: ok | 0: faild

    i64    offusec;         // offset of usec, 1 sec = 1000000 usec
    i64    nowusec;         // now of usec

    constr err;             // if status is 0, we show err info here, but do not free it
}esync_t, esync;

esync etimer_esyncGet(constr server, int timeout);   // server can have port(lg:"1.1.1.1:123"), default port is 123(ntp); the unit of timeout is ms

#ifdef __cplusplus
}
#endif

#endif
