#include "minigit.h"

const char *weekday_names[] = {
  "Sunday",  "Mondays",  "Tuesdays",  "Wednesdays",  "Thursdays",  "Fridays",  "Saturdays"
};

const char *month_names[] = {
  "January",  "February",  "March",  "April",  "May",  "June",  "July",  "August",  "September",  "October",  "November",  "December"
};

/**
 * git:date.cのtime_to_tmおよびgm_time_tから借用
 */
struct tm *time_to_tm(unsigned long time, int tz)
{
    int minutes;

    minutes = tz < 0 ? -tz : tz;
    minutes = (minutes / 100)*60 + (minutes % 100);
    minutes = tz < 0 ? -minutes : minutes;
    time_t t = (time_t) time + minutes * 60;
    return gmtime(&t);
}
