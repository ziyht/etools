/// =====================================================================================
///
///       Filename:  etime.h
///
///    Description:  a compat time impletation for different platforms
///
///        Version:  1.0
///        Created:  06/11/2019 11:00:34 AM
///       Revision:  none
///       Compiler:  gcc
///
///         Author:  Haitao Yang, joyhaitao@foxmail.com
///        Company:
///
/// =====================================================================================

#include "time.h"

#include "etype.h"

typedef struct etm_s {
        int     tm_sec;         /* seconds after the minute [0-60] */
        int     tm_min;         /* minutes after the hour [0-59] */
        int     tm_hour;        /* hours since midnight [0-23] */
        int     tm_mday;        /* day of the month [1-31] */
        int     tm_mon;         /* months since January [0-11] */
        int     tm_year;        /* years since 1900 */
        int     tm_wday;        /* days since Sunday [0-6] */
        int     tm_yday;        /* days since January 1 [0-365] */
        int     tm_isdst;       /* Daylight Savings Time flag */
        long    tm_gmtoff;      /* offset from UTC in seconds */
        char    *tm_zone;       /* timezone abbreviation */
        long    tm_nsec;        /* nanoseconds */
}etm;

i64 e_nowns();
i64 e_nowms();
i64 e_nows ();

//! get a str of time in format like this:
//!     20130603150702.352432467
//!
//!  the output len is limited by dlen - 1
//!
cstr e_nowstr(cstr desc, int dlen);
cstr e_secstr(cstr desc, int dlen, time_t sec);

//!
//!
//! \brief estrfstr - format to time str from another formated time str
//! \brief estrfsec - format to time str from a timestamp(sec)
//! \brief estrpsec - parse a time str to timestamp(sec)
//! \brief estrpms  - parse a time str to timestamp(ms)
//!
cstr e_strfstr(cstr dest, int dlen, constr dfmt, constr from, constr ffmt);
cstr e_strfsec(cstr dest, int dlen, constr dfmt, time_t sec );
i64  e_strpsec(constr from, constr ffmt);
i64  e_strpms (constr from, constr ffmt);


//!
//!
//! \brief eelapsefstr - format to elapse str from a time str
//! \brief eelapsefsec - format to elapse str from a sec timestamp
//! \brief eelapsepsec - parse a elapse str to passed seconds
//! \brief eelapsepms  - parse a elapse str to passed ms
//!
cstr e_elapsefstr();
cstr e_elapsefsec();
i64  e_elapsepsec();
i64  e_elapsepms();
