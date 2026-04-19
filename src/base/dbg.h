 
 

#ifndef BASE_DBG_H
#define BASE_DBG_H

#include <functional>

 

 
#define dbg_assert(test, fmt, ...) \
	do \
	{ \
		if(!(test)) \
		{ \
			dbg_assert_imp(__FILE__, __LINE__, fmt, ##__VA_ARGS__); \
		} \
	} while(false)

 
#define dbg_assert_failed(fmt, ...) dbg_assert_imp(__FILE__, __LINE__, fmt, ##__VA_ARGS__)

 
[[gnu::format(printf, 3, 4)]] [[noreturn]] void
dbg_assert_imp(const char *filename, int line, const char *fmt, ...);

 
bool dbg_assert_has_failed();

 
[[noreturn]] void dbg_break();

 
typedef std::function<void(const char *message)> DBG_ASSERT_HANDLER;

 
void dbg_assert_set_handler(DBG_ASSERT_HANDLER handler);

 
[[gnu::format(printf, 2, 3)]] void dbg_msg(const char *sys, const char *fmt, ...);

#endif
