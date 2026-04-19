 
 

#ifndef BASE_TIME_H
#define BASE_TIME_H

#include <chrono>
#include <cstdint>
#include <ctime>

 

 

 
void set_new_tick();

 
std::chrono::nanoseconds time_get_nanoseconds();

 
int64_t time_get_impl();

 
int64_t time_get();

 
int64_t time_freq();

 
int64_t time_timestamp();

 
int time_houroftheday();

 
enum ETimeSeason
{
	SEASON_SPRING = 0,
	SEASON_SUMMER,
	SEASON_AUTUMN,
	SEASON_WINTER,
	SEASON_EASTER,
	SEASON_HALLOWEEN,
	SEASON_XMAS,
	SEASON_NEWYEAR
};

 
ETimeSeason time_season();

 
void str_timestamp(char *buffer, int buffer_size);

 
[[gnu::format(strftime, 3, 0)]] void str_timestamp_format(char *buffer, int buffer_size, const char *format);

 
[[gnu::format(strftime, 4, 0)]] void str_timestamp_ex(time_t time, char *buffer, int buffer_size, const char *format);

 
[[gnu::format(strftime, 2, 0)]] bool timestamp_from_str(const char *string, const char *format, time_t *timestamp);

#define FORMAT_TIME "%H:%M:%S"
#define FORMAT_SPACE "%Y-%m-%d %H:%M:%S"
#define FORMAT_NOSPACE "%Y-%m-%d_%H-%M-%S"

enum
{
	TIME_DAYS,
	TIME_HOURS,
	TIME_MINS,
	TIME_HOURS_CENTISECS,
	TIME_MINS_CENTISECS,
	TIME_SECS_CENTISECS,
};

 
int str_time(int64_t centisecs, int format, char *buffer, int buffer_size);

 
int str_time_float(float secs, int format, char *buffer, int buffer_size);

#endif
