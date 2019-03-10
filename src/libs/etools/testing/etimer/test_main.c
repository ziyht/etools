#include "test_main.h"
#include <time.h>

int main()
{
    //etimer_timeline_test();
    //etimer_ntpSync_test();

     //etimer_test_performance();
     //etimer_test_performance2();

    etimer_test_fmt();
}


void etimer_test_fmt()
{
    char buf[64]; time_t secs;  cstr ret;

    ret = etimer_strfstr(buf, 64, "%F %T", "17:43:03 2018", "%T %Y");
    ret = etimer_strfstr(buf, 64, "%Y-%m-%d %T", "17:43:03", "%T");

    ret = etimer_strfstr(buf, 64, "%Y-%m-%d %T", "Wed Jun 27 17:43:03 2018", "%a %b %d %T %Y");
    ret = etimer_strfstr(buf, 64, "%Y-%m-%d %T", "Jun 17:43:03 2018", "%b %T %Y");
    ret = etimer_strfstr(buf, 64, "%Y-%m-%d %T", "17:43:03 2018", "%T %Y");
    ret = etimer_strfstr(buf, 64, "%Y-%m-%d %T", "17:43:03", "%T");

    secs = etimer_strpsec("Wed Jun 27 17:43:03 2018", "%a %b %d %T %Y");
    ret  = etimer_strfsec(buf, 64, "%Y-%m-%d %T", secs);

    ret  = etimer_strfsec(buf, 64, "%Y-%m-%d %T", 31723506);
    ret  = etimer_strfsec(buf, 64, "%Y-%m-%d %T", 1530092583);
    ret  = etimer_strfsec(buf, 64, "%Y-%m-%d %T", 1);

    int year = 1,
        day  = 2,
        hour = 4,
        min  = 5,
        sec  = 6;


    secs = year * (365 * 24 * 3600) +
           day  * (24 * 3600) +
           hour * (3600) +
           min  * 60 +
           sec;   // 31723506



    ret = etimer_strfsec(buf, 64, "%Y-%m-%d %T", secs);

    etimer_elapsefsec(buf, 64, "%Yy %jd %T", secs);
    etimer_elapsefsec(buf, 64, "%jd %T"    , secs);
    etimer_elapsefsec(buf, 64, "%Yy %T"    , secs);
    etimer_elapsefsec(buf, 64, "%T"        , secs);

    etimer_elapsefsec(buf, 64, "%Yy %jd %T", 1);
    etimer_elapsefsec(buf, 64, "%jd %T"    , 1);
    etimer_elapsefsec(buf, 64, "%Yy %T"    , 1);
    etimer_elapsefsec(buf, 64, "%T"        , 1);

    etimer_elapsefstr(buf, 64, "%Yy %jd %T", "367 04:05:06 1", "%j %T %Y");
    etimer_elapsefstr(buf, 64, "%jd %T"    , "367 04:05:06 1", "%j %T %Y");
    etimer_elapsefstr(buf, 64, "%Yy %T"    , "367 04:05:06 1", "%j %T %Y");
    etimer_elapsefstr(buf, 64, "%T"        , "367 04:05:06 1", "%j %T %Y");

    etimer_elapsefstr(buf, 64, "%Yy %jd %T", "Wed Jan 2 00:00:03 0000", "%a %b %d %T %Y");

    secs = etimer_elapsepsec("01", "%S");
    secs = etimer_elapsepsec("01:01", "%M:%S");
    secs = etimer_elapsepsec("01:01:01", "%H:%M:%S");
    secs = etimer_elapsepsec("01:01:01", "%T");
    secs = etimer_elapsepsec("1 01:01:01", "%j %T");
    secs = etimer_elapsepsec("2 04:05:06 1", "%j %T %Y");
    secs = etimer_elapsepsec("Wed Jan 1 00:00:03", "%a %b %d %T");
    secs = etimer_elapsepsec("Wed Jan 0 00:00:03 0000", "%a %b %d %T %Y");
    secs = etimer_elapsepsec("Wed Jan 1 00:00:03 0000", "%a %b %d %T %Y");
    secs = etimer_elapsepsec("Wed Jan 2 02:00:03 0000", "%a %b %d %T %Y");
    secs = etimer_elapsepsec("Wed Jan 2 02:00:03 0001", "%a %b %d %T %Y");
    secs = etimer_elapsepsec("Wed Jan 2 02:00:03 0002", "%a %b %d %T %Y");
    secs = etimer_elapsepsec("Wed Jan 2 02:00:03 1900", "%a %b %d %T %Y");
    secs = etimer_elapsepsec("Wed Jan 2 02:00:03 1970", "%a %b %d %T %Y");
    secs = etimer_elapsepsec("Wed Jan 2 02:00:03 2018", "%a %b %d %T %Y");

    secs = etimer_elapsepms("3-02:49:50", "%j-%T");
}
