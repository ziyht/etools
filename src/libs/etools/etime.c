/// =====================================================================================
///
///       Filename:  etime.c
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

#define _CRT_SECURE_NO_WARNINGS

#include <time.h>
#include <stdio.h>

#include "ecompat.h"
#include "eutils.h"

#include "etime.h"

typedef enum {
  _CLOCK_PRECISE = 0,  /* Use the highest resolution clock available. */
  _CLOCK_FAST    = 1   /* Use the fastest clock with <= 1ms granularity. */
} clocktype_t;

/* Available from 2.6.32 onwards. */
#ifndef CLOCK_MONOTONIC_COARSE
#define CLOCK_MONOTONIC_COARSE 6
#endif

#ifndef _WIN32
#include <sys/time.h>
static i64     __hrtime_nsec_offset;
static clock_t __fast_clock_id;
static inline int __hrtime_init()
{

    struct timespec t1 = {0, 0},
                    t2 = {0, 0};
    struct timeval  t3 = {0, 0};

    // -- check function
    if(clock_gettime(CLOCK_MONOTONIC, &t1))
    {
        if(clock_gettime(CLOCK_REALTIME, &t1))
        {
            perror("clock_gettime not working correctly");
            abort();        /* Not really possible. */
        }

        return 1;           // offset is 0;
    }

    // -- get offset
    if(-1 == gettimeofday(&t3, 0))
    {
        clock_gettime(CLOCK_REALTIME, &t2);
        __hrtime_nsec_offset = (i64)t2.tv_sec * 1000000000 + (i64)t2.tv_nsec - (i64)t1.tv_sec * 1000000000 + (i64)t1.tv_nsec;
    }
    else
        __hrtime_nsec_offset = (i64)t3.tv_sec * 1000000000 + (i64)t3.tv_usec*1000 - (i64)t1.tv_sec * 1000000000 + (i64)t1.tv_nsec;

    // -- check fast get
    if (clock_getres(CLOCK_MONOTONIC_COARSE, &t1) == 0 && t1.tv_nsec <= 1 * 1000 * 1000)
        __fast_clock_id = CLOCK_MONOTONIC_COARSE;
    else
        __fast_clock_id = CLOCK_MONOTONIC;

    return 1;
}

static inline u64 __hrtime_ns(clocktype_t type)
{
    static int __hrtime_init_needed = 1;
    struct timespec t;
    clock_t clock_id;

    if( __hrtime_init_needed ) {
        __hrtime_init();
        __hrtime_init_needed = 0;
    }

    clock_id = (type == _CLOCK_FAST) ? __fast_clock_id : CLOCK_MONOTONIC;

    if (clock_gettime(clock_id, &t))
        clock_gettime(CLOCK_REALTIME, &t);

    return t.tv_sec * 1000000000 + t.tv_nsec + __hrtime_nsec_offset;
}

#else

#include <sys/timeb.h>
static i64    __hrtime_nsec_offset;
static double __hrtime_interval;
#define __hrtime_precise 1000000000LL
static inline int __hrtime_init()
{
    if(__hrtime_interval == 0)
    {
        LARGE_INTEGER perf_frequency; struct timeb tm; LARGE_INTEGER counter;

        if (QueryPerformanceFrequency(&perf_frequency)) {	__hrtime_interval = 1.0 / perf_frequency.QuadPart;}
        else{
            perror("clock_gettime not working correctly");
            abort();        /* Not really possible. */
        }

        ftime(&tm);										// note: the PRECISE of window of this func is 15ms
        QueryPerformanceCounter(&counter);
        __hrtime_nsec_offset = __hrtime_precise * tm.time + __hrtime_precise / 1000 * tm.millitm - (i64) ((double) counter.QuadPart * __hrtime_interval * __hrtime_precise);
    }

    return 1;
}

static inline u64 __hrtime_ns(clocktype_t type)
{
    LARGE_INTEGER counter;

    if(__hrtime_interval == 0) __hrtime_init();

    QueryPerformanceCounter(&counter);

    return (u64) ((double) counter.QuadPart * __hrtime_interval * __hrtime_precise) + __hrtime_nsec_offset;
}
#endif

i64  e_nowns() { return __hrtime_ns(_CLOCK_PRECISE)             ; }
i64  e_nowms() { return __hrtime_ns(_CLOCK_FAST   ) / 1000000   ; }
i64  e_nows () { return __hrtime_ns(_CLOCK_FAST   ) / 1000000000; }

cstr e_nowstr(cstr buf, int len)
{
    struct tm time; u64 now_ns; time_t sec;

    now_ns = e_nowns();
    sec    = now_ns / 1000000000;

    localtime_r(&sec, &time);

    len < 17 ? snprintf(buf, len, "%4d%02d%02d%02d%02d%02d "           , time.tm_year + 1900, time.tm_mon + 1, time.tm_mday, time.tm_hour, time.tm_min, time.tm_sec)
             : snprintf(buf, len, "%4d%02d%02d%02d%02d%02d.%09"PRIi64"", time.tm_year + 1900, time.tm_mon + 1, time.tm_mday, time.tm_hour, time.tm_min, time.tm_sec, now_ns % 1000000000);

    return buf;
}

char* estrptime(const char *buf, const char* fmt, etm* tm);
cstr e_strfstr(cstr dest, int dlen, constr dfmt, constr from, constr ffmt)
{
    etm _tm = {0};

    if(estrptime(from, ffmt, &_tm))
    {
        return strftime(dest, dlen, dfmt, (const struct tm*)&_tm) ? dest : 0;
    }

    return 0;
}

#define locale_t int

#undef isspace_l
#undef isdigit_l
#undef isupper_l
#undef strtol_l
#undef isleap
#undef strcasecmp_l
#undef strncasecmp_l

#define isspace_l(A, _)         isspace(A)
#define isdigit_l(A, _)         isdigit(A)
#define isupper_l(A, _)         isupper(A)
#define strtol_l(A, B, C, _)    strtol(A, B, C)

#define strcasecmp_l(A, B, _)       e_strcasecmp(A, B)
#define strncasecmp_l(A, B, C, _)   e_strncasecmp(A, B, C)

#define isleap(y)   (((y) % 4) == 0 && (((y) % 100) != 0 || ((y) % 400) == 0))

#define YEARSPERREPEAT		400	/* years before a Gregorian repeat */

#define SECSPERMIN	60
#define MINSPERHOUR	60
#define HOURSPERDAY	24
#define DAYSPERWEEK	7
#define DAYSPERNYEAR	365
#define DAYSPERLYEAR	366
#define SECSPERHOUR	(SECSPERMIN * MINSPERHOUR)
#define SECSPERDAY	(SECSPERHOUR * HOURSPERDAY)
#define MONSPERYEAR	12

#define TM_SUNDAY	0
#define TM_MONDAY	1
#define TM_TUESDAY	2
#define TM_WEDNESDAY	3
#define TM_THURSDAY	4
#define TM_FRIDAY	5
#define TM_SATURDAY	6

#define TM_JANUARY	0
#define TM_FEBRUARY	1
#define TM_MARCH	2
#define TM_APRIL	3
#define TM_MAY		4
#define TM_JUNE		5
#define TM_JULY		6
#define TM_AUGUST	7
#define TM_SEPTEMBER	8
#define TM_OCTOBER	9
#define TM_NOVEMBER	10
#define TM_DECEMBER	11

#define TM_YEAR_BASE	1900

#define EPOCH_YEAR	1970
#define EPOCH_WDAY	TM_THURSDAY

struct lc_time_T {
    const char	*mon[12];
    const char	*month[12];
    const char	*wday[7];
    const char	*weekday[7];
    const char	*X_fmt;
    const char	*x_fmt;
    const char	*c_fmt;
    const char	*am;
    const char	*pm;
    const char	*date_fmt;
    const char	*alt_month[12];
    const char	*md_order;
    const char	*ampm_fmt;
};const struct lc_time_T _C_time_locale =
{
    {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    }, {
        "January", "February", "March", "April", "May", "June",
        "July", "August", "September", "October", "November", "December"
    }, {
        "Sun", "Mon", "Tue", "Wed",
        "Thu", "Fri", "Sat"
    }, {
        "Sunday", "Monday", "Tuesday", "Wednesday",
        "Thursday", "Friday", "Saturday"
    },

    /* X_fmt */
    "%H:%M:%S",

    /*
     * x_fmt
     * Since the C language standard calls for
     * "date, using locale's date format," anything goes.
     * Using just numbers (as here) makes Quakers happier;
     * it's also compatible with SVR4.
     */
    "%m/%d/%y",

    /*
     * c_fmt
     */
    "%a %b %e %H:%M:%S %Y",

    /* am */
    "AM",

    /* pm */
    "PM",

    /* date_fmt */
    "%a %b %e %H:%M:%S %Z %Y",

    /* alt_month
     * Standalone months forms for %OB
     */
    {
        "January", "February", "March", "April", "May", "June",
        "July", "August", "September", "October", "November", "December"
    },

    /* md_order
     * Month / day order in dates
     */
    "md",

    /* ampm_fmt
     * To determine 12-hour clock format time (empty, if N/A)
     */
    "%I:%M:%S %p"
};
#define __get_current_time_locale(X) (&_C_time_locale)

#define	asizeof(a)	(sizeof(a) / sizeof((a)[0]))

#define	FLAG_NONE	(1 << 0)
#define	FLAG_YEAR	(1 << 1)
#define	FLAG_MONTH	(1 << 2)
#define	FLAG_YDAY	(1 << 3)
#define	FLAG_MDAY	(1 << 4)
#define	FLAG_WDAY	(1 << 5)

/*
 * Calculate the week day of the first day of a year. Valid for
 * the Gregorian calendar, which began Sept 14, 1752 in the UK
 * and its colonies. Ref:
 * http://en.wikipedia.org/wiki/Determination_of_the_day_of_the_week
 */
static inline int first_wday_of(int year)
{
    return (((2 * (3 - (year / 100) % 4)) + (year % 100) +
        ((year % 100) / 4) + (isleap(year) ? 6 : 0) + 1) % 7);
}

static const int	mon_lengths[2][MONSPERYEAR] = {
    { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
    { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
};

static const int	year_lengths[2] = {
    DAYSPERNYEAR, DAYSPERLYEAR
};

#define LEAPS_THRU_END_OF(y)	((y) / 4 - (y) / 100 + (y) / 400)

static void timesub(
        const time_t * const    timep,
        const long				offset,
              etm * const		tmp)
{
    long			days;
    long			 rem;
    long			   y;
    int			   yleap;
    const int *		  ip;

    days = (long)(*timep / SECSPERDAY);
    rem  = *timep % SECSPERDAY;
    rem += (offset);
    while (rem < 0) {
        rem += SECSPERDAY;
        --days;
    }
    while (rem >= SECSPERDAY) {
        rem -= SECSPERDAY;
        ++days;
    }
    tmp->tm_hour = (int) (rem / SECSPERHOUR);
    rem = rem % SECSPERHOUR;
    tmp->tm_min = (int) (rem / SECSPERMIN);
    /*
    ** A positive leap second requires a special
    ** representation.  This uses "... ??:59:60" et seq.
    */
    tmp->tm_sec = (int) (rem % SECSPERMIN) ;
    tmp->tm_wday = (int) ((EPOCH_WDAY + days) % DAYSPERWEEK);
    if (tmp->tm_wday < 0)
        tmp->tm_wday += DAYSPERWEEK;

    y = EPOCH_YEAR;

    while (days < 0 || days >= (long) year_lengths[yleap = isleap(y)]) {
        long	newy;

        newy = y + days / DAYSPERNYEAR;
        if (days < 0)
            --newy;
        days -= (newy - y) * DAYSPERNYEAR +
            LEAPS_THRU_END_OF(newy - 1) -
            LEAPS_THRU_END_OF(y - 1);
        y = newy;
    }
    tmp->tm_year = y - TM_YEAR_BASE;
    tmp->tm_yday = (int) days;
    ip = mon_lengths[yleap];
    for (tmp->tm_mon = 0; days >= (long) ip[tmp->tm_mon]; ++(tmp->tm_mon))
        days = days - (long) ip[tmp->tm_mon];
    tmp->tm_mday = (int) (days + 1);
    tmp->tm_isdst = 0;
}

/*
* Re-entrant version of gmtime.
*/
etm* _gmtime_r(const time_t* timep, etm *tm)
{
    timesub(timep, 0L, tm);
    return tm;
}

#define gmtime_r _gmtime_r

static char *
_strptime(const char *buf, const char *fmt, etm *tm, int *GMTp,
        locale_t locale)
{
    char	c;
    const char *ptr;
    int	day_offset = -1, wday_offset;
    int week_offset;
    int	i, len;
    int flags;
    int Ealternative, Oalternative;
    const struct lc_time_T *tptr = __get_current_time_locale(locale);
    static int start_of_month[2][13] = {
        {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365},
        {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366}
    };

    flags = FLAG_NONE;

    ptr = fmt;
    while (*ptr != 0) {
        c = *ptr++;

        if (c != '%') {
            if (isspace_l((unsigned char)c, locale))
                while (*buf != 0 &&
                       isspace_l((unsigned char)*buf, locale))
                    buf++;
            else if (c != *buf++)
                return (NULL);
            continue;
        }

        Ealternative = 0;
        Oalternative = 0;
label:
        c = *ptr++;
        switch (c) {
        case '%':
            if (*buf++ != '%')
                return (NULL);
            break;

        case '+':
            buf = _strptime(buf, tptr->date_fmt, tm, GMTp, locale);
            if (buf == NULL)
                return (NULL);
            flags |= FLAG_WDAY | FLAG_MONTH | FLAG_MDAY | FLAG_YEAR;
            break;

        case 'C':
            if (!isdigit_l((unsigned char)*buf, locale))
                return (NULL);

            /* XXX This will break for 3-digit centuries. */
            len = 2;
            for (i = 0; len && *buf != 0 &&
                 isdigit_l((unsigned char)*buf, locale); buf++) {
                i *= 10;
                i += *buf - '0';
                len--;
            }
            if (i < 19)
                return (NULL);

            tm->tm_year = i * 100 - TM_YEAR_BASE;
            flags |= FLAG_YEAR;

            break;

        case 'c':
            buf = _strptime(buf, tptr->c_fmt, tm, GMTp, locale);
            if (buf == NULL)
                return (NULL);
            flags |= FLAG_WDAY | FLAG_MONTH | FLAG_MDAY | FLAG_YEAR;
            break;

        case 'D':
            buf = _strptime(buf, "%m/%d/%y", tm, GMTp, locale);
            if (buf == NULL)
                return (NULL);
            flags |= FLAG_MONTH | FLAG_MDAY | FLAG_YEAR;
            break;

        case 'E':
            // CockroachDB: unsupported
            return (NULL);
            if (Ealternative || Oalternative)
                break;
            Ealternative++;
            goto label;

        case 'O':
            // CockroachDB: unsupported
            return (NULL);
            if (Ealternative || Oalternative)
                break;
            Oalternative++;
            goto label;

        case 'F':
            buf = _strptime(buf, "%Y-%m-%d", tm, GMTp, locale);
            if (buf == NULL)
                return (NULL);
            flags |= FLAG_MONTH | FLAG_MDAY | FLAG_YEAR;
            break;

        case 'R':
            buf = _strptime(buf, "%H:%M", tm, GMTp, locale);
            if (buf == NULL)
                return (NULL);
            break;

        case 'r':
            buf = _strptime(buf, tptr->ampm_fmt, tm, GMTp, locale);
            if (buf == NULL)
                return (NULL);
            break;

        case 'T':
            buf = _strptime(buf, "%H:%M:%S", tm, GMTp, locale);
            if (buf == NULL)
                return (NULL);
            break;

        case 'X':
            buf = _strptime(buf, tptr->X_fmt, tm, GMTp, locale);
            if (buf == NULL)
                return (NULL);
            break;

        case 'x':
            buf = _strptime(buf, tptr->x_fmt, tm, GMTp, locale);
            if (buf == NULL)
                return (NULL);
            flags |= FLAG_MONTH | FLAG_MDAY | FLAG_YEAR;
            break;

        case 'j':
            if (!isdigit_l((unsigned char)*buf, locale))
                return (NULL);

            len = 3;
            for (i = 0; len && *buf != 0 &&
                 isdigit_l((unsigned char)*buf, locale); buf++){
                i *= 10;
                i += *buf - '0';
                len--;
            }
            if (i < 1 || i > 366)
                return (NULL);

            tm->tm_yday = i - 1;
            flags |= FLAG_YDAY;

            break;

        case 'f':
            /* CockroachDB extension: nanoseconds */
            if (!isdigit_l((unsigned char)*buf, locale))
                return (NULL);

            len = 9;
            for (i = 0; len && *buf != 0 &&
                 isdigit_l((unsigned char)*buf, locale); buf++){
                i *= 10;
                i += *buf - '0';
                len--;
            }
            while (len) {
                i *= 10;
                len--;
            }

            tm->tm_nsec = i;

            break;

        case 'M':
        case 'S':
            if (*buf == 0 ||
                isspace_l((unsigned char)*buf, locale))
                break;

            if (!isdigit_l((unsigned char)*buf, locale))
                return (NULL);

            len = 2;
            for (i = 0; len && *buf != 0 &&
                isdigit_l((unsigned char)*buf, locale); buf++){
                i *= 10;
                i += *buf - '0';
                len--;
            }

            if (c == 'M') {
                if (i > 59)
                    return (NULL);
                tm->tm_min = i;
            } else {
                if (i > 60)
                    return (NULL);
                tm->tm_sec = i;
            }

            break;

        case 'H':
        case 'I':
        case 'k':
        case 'l':
            /*
             * Of these, %l is the only specifier explicitly
             * documented as not being zero-padded.  However,
             * there is no harm in allowing zero-padding.
             *
             * XXX The %l specifier may gobble one too many
             * digits if used incorrectly.
             */
            if (!isdigit_l((unsigned char)*buf, locale))
                return (NULL);

            len = 2;
            for (i = 0; len && *buf != 0 &&
                 isdigit_l((unsigned char)*buf, locale); buf++) {
                i *= 10;
                i += *buf - '0';
                len--;
            }
            if (c == 'H' || c == 'k') {
                if (i > 23)
                    return (NULL);
            } else if (i > 12)
                return (NULL);

            tm->tm_hour = i;

            break;

        case 'p':
            /*
             * XXX This is bogus if parsed before hour-related
             * specifiers.
             */
            len = (int)strlen(tptr->am);
            if (strncasecmp_l(buf, tptr->am, len, locale) == 0) {
                if (tm->tm_hour > 12)
                    return (NULL);
                if (tm->tm_hour == 12)
                    tm->tm_hour = 0;
                buf += len;
                break;
            }

            len = (int)strlen(tptr->pm);
            if (strncasecmp_l(buf, tptr->pm, len, locale) == 0) {
                if (tm->tm_hour > 12)
                    return (NULL);
                if (tm->tm_hour != 12)
                    tm->tm_hour += 12;
                buf += len;
                break;
            }

            return (NULL);

        case 'A':
        case 'a':
            for (i = 0; i < asizeof(tptr->weekday); i++) {
                len = (int)strlen(tptr->weekday[i]);
                if (strncasecmp_l(buf, tptr->weekday[i],
                        len, locale) == 0)
                    break;
                len = (int)strlen(tptr->wday[i]);
                if (strncasecmp_l(buf, tptr->wday[i],
                        len, locale) == 0)
                    break;
            }
            if (i == asizeof(tptr->weekday))
                return (NULL);

            buf += len;
            tm->tm_wday = i;
            flags |= FLAG_WDAY;
            break;

        case 'U':
        case 'W':
            /*
             * XXX This is bogus, as we can not assume any valid
             * information present in the tm structure at this
             * point to calculate a real value, so just check the
             * range for now.
             */
            if (!isdigit_l((unsigned char)*buf, locale))
                return (NULL);

            len = 2;
            for (i = 0; len && *buf != 0 &&
                 isdigit_l((unsigned char)*buf, locale); buf++) {
                i *= 10;
                i += *buf - '0';
                len--;
            }
            if (i > 53)
                return (NULL);

            if (c == 'U')
                day_offset = TM_SUNDAY;
            else
                day_offset = TM_MONDAY;


            week_offset = i;

            break;

        case 'w':
            if (!isdigit_l((unsigned char)*buf, locale))
                return (NULL);

            i = *buf - '0';
            if (i > 6)
                return (NULL);

            tm->tm_wday = i;
            flags |= FLAG_WDAY;

            break;

        case 'u':
            // CockroachDB extension of the FreeBSD code
            if (!isdigit_l((unsigned char)*buf, locale))
                return (NULL);

            i = *buf - '0';
            if (i < 1 || i > 7)
                return (NULL);
            if (i == 7)
                i = 0;

            tm->tm_wday = i;
            flags |= FLAG_WDAY;

            break;

        case 'e':
            /*
             * With %e format, our strftime(3) adds a blank space
             * before single digits.
             */
            if (*buf != 0 &&
                isspace_l((unsigned char)*buf, locale))
                   buf++;
            /* FALLTHROUGH */
        case 'd':
            /*
             * The %e specifier was once explicitly documented as
             * not being zero-padded but was later changed to
             * equivalent to %d.  There is no harm in allowing
             * such padding.
             *
             * XXX The %e specifier may gobble one too many
             * digits if used incorrectly.
             */
            if (!isdigit_l((unsigned char)*buf, locale))
                return (NULL);

            len = 2;
            for (i = 0; len && *buf != 0 &&
                 isdigit_l((unsigned char)*buf, locale); buf++) {
                i *= 10;
                i += *buf - '0';
                len--;
            }
            if (i > 31)
                return (NULL);

            tm->tm_mday = i;
            flags |= FLAG_MDAY;

            break;

        case 'B':
        case 'b':
        case 'h':
            for (i = 0; i < asizeof(tptr->month); i++) {
                if (Oalternative) {
                    if (c == 'B') {
                        len = (int)strlen(tptr->alt_month[i]);
                        if (strncasecmp_l(buf,
                                tptr->alt_month[i],
                                len, locale) == 0)
                            break;
                    }
                } else {
                    len = (int)strlen(tptr->month[i]);
                    if (strncasecmp_l(buf, tptr->month[i],
                            len, locale) == 0)
                        break;
                }
            }
            /*
             * Try the abbreviated month name if the full name
             * wasn't found and Oalternative was not requested.
             */
            if (i == asizeof(tptr->month) && !Oalternative) {
                for (i = 0; i < asizeof(tptr->month); i++) {
                    len = (int)strlen(tptr->mon[i]);
                    if (strncasecmp_l(buf, tptr->mon[i],
                            len, locale) == 0)
                        break;
                }
            }
            if (i == asizeof(tptr->month))
                return (NULL);

            tm->tm_mon = i;
            buf += len;
            flags |= FLAG_MONTH;

            break;

        case 'm':
            if (!isdigit_l((unsigned char)*buf, locale))
                return (NULL);

            len = 2;
            for (i = 0; len && *buf != 0 &&
                 isdigit_l((unsigned char)*buf, locale); buf++) {
                i *= 10;
                i += *buf - '0';
                len--;
            }
            if (i < 1 || i > 12)
                return (NULL);

            tm->tm_mon = i - 1;
            flags |= FLAG_MONTH;

            break;

        case 's':
            {
            char *cp;
            int sverrno;
            long n;
            time_t t;

            sverrno = errno;
            errno = 0;
            n = strtol_l(buf, &cp, 10, locale);
            if (errno == ERANGE || (long)(t = n) != n) {
                errno = sverrno;
                return (NULL);
            }
            errno = sverrno;
            buf = cp;
            if (gmtime_r(&t, tm) == NULL)
                return (NULL);
            *GMTp = 1;
            flags |= FLAG_YDAY | FLAG_WDAY | FLAG_MONTH |
                FLAG_MDAY | FLAG_YEAR;
            }
            break;

        case 'Y':
        case 'y':
            if (*buf == 0 ||
                isspace_l((unsigned char)*buf, locale))
                break;

            if (!isdigit_l((unsigned char)*buf, locale))
                return (NULL);

            len = (c == 'Y') ? 4 : 2;
            for (i = 0; len && *buf != 0 &&
                 isdigit_l((unsigned char)*buf, locale); buf++) {
                i *= 10;
                i += *buf - '0';
                len--;
            }
            if (c == 'Y')
                i -= TM_YEAR_BASE;
            if (c == 'y' && i < 69)
                i += 100;
            if (i < 0)
                return (NULL);

            tm->tm_year = i;
            flags |= FLAG_YEAR;

            break;

        case 'Z':
            {
            const char *cp;
            char *zonestr;

            for (cp = buf; *cp &&
                 isupper_l((unsigned char)*cp, locale); ++cp) {
                /*empty*/}
            if (cp - buf) {
                zonestr = malloc(cp - buf + 1);
                strncpy(zonestr, buf, cp - buf);
                zonestr[cp - buf] = '\0';
                //tzset();
                if (0 == strcmp(zonestr, "GMT") ||
                    0 == strcmp(zonestr, "UTC")) {
                    *GMTp = 1;
                // } else if (0 == strcmp(zonestr, tzname[0])) {
                //     tm->tm_isdst = 0;
                // } else if (0 == strcmp(zonestr, tzname[1])) {
                //     tm->tm_isdst = 1;
                } else {
                    return (NULL);
                }
                buf += cp - buf;
            }
            }
            break;

        case 'z':
            {
            int sign = 1;

            if (*buf != '+') {
                if (*buf == '-')
                    sign = -1;
                else
                    return (NULL);
            }

            buf++;
            i = 0;
            for (len = 4; len > 0; len--) {
                if (isdigit_l((unsigned char)*buf, locale)) {
                    i *= 10;
                    i += *buf - '0';
                    buf++;
                } else
                    return (NULL);
            }

            tm->tm_hour -= sign * (i / 100);
            tm->tm_min  -= sign * (i % 100);
            *GMTp = 1;
            }
            break;

        case 'n':
        case 't':
            while (isspace_l((unsigned char)*buf, locale))
                buf++;
            break;

        default:
            return (NULL);
        }
    }

    if (!(flags & FLAG_YDAY) && (flags & FLAG_YEAR)) {
        if ((flags & (FLAG_MONTH | FLAG_MDAY)) ==
            (FLAG_MONTH | FLAG_MDAY)) {
            tm->tm_yday = start_of_month[isleap(tm->tm_year +
                TM_YEAR_BASE)][tm->tm_mon] + (tm->tm_mday - 1);
            flags |= FLAG_YDAY;
        } else if (day_offset != -1) {
            /* Set the date to the first Sunday (or Monday)
             * of the specified week of the year.
             */
            if (!(flags & FLAG_WDAY)) {
                tm->tm_wday = day_offset;
                flags |= FLAG_WDAY;
            }
            tm->tm_yday = (7 -
                first_wday_of(tm->tm_year + TM_YEAR_BASE) +
                day_offset) % 7 + (week_offset - 1) * 7 +
                tm->tm_wday - day_offset;
            flags |= FLAG_YDAY;
        }
    }

    if ((flags & (FLAG_YEAR | FLAG_YDAY)) == (FLAG_YEAR | FLAG_YDAY)) {
        if (!(flags & FLAG_MONTH)) {
            i = 0;
            while (tm->tm_yday >=
                start_of_month[isleap(tm->tm_year +
                TM_YEAR_BASE)][i])
                i++;
            if (i > 12) {
                i = 1;
                tm->tm_yday -=
                    start_of_month[isleap(tm->tm_year +
                    TM_YEAR_BASE)][12];
                tm->tm_year++;
            }
            tm->tm_mon = i - 1;
            flags |= FLAG_MONTH;
        }
        if (!(flags & FLAG_MDAY)) {
            tm->tm_mday = tm->tm_yday -
                start_of_month[isleap(tm->tm_year + TM_YEAR_BASE)]
                [tm->tm_mon] + 1;
            flags |= FLAG_MDAY;
        }
        if (!(flags & FLAG_WDAY)) {
            i = 0;
            wday_offset = first_wday_of(tm->tm_year);
            while (i++ <= tm->tm_yday) {
                if (wday_offset++ >= 6)
                    wday_offset = 0;
            }
            tm->tm_wday = wday_offset;
            flags |= FLAG_WDAY;
        }
    }

    return ((char *)buf);
}

char* estrptime(const char *buf, const char* fmt, etm* tm)
{
    int GMP = 0;

    memset(tm, 0, sizeof(*tm));

    return _strptime(buf, fmt, tm, &GMP, 0);
}
