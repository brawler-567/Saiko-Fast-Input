 
 

 

#ifndef BASE_SYSTEM_H
#define BASE_SYSTEM_H

#include "dbg.h"
#include "detect.h"
#include "fs.h"
#include "mem.h"
#include "str.h"
#include "time.h"
#include "types.h"

#include <chrono>
#include <cinttypes>
#include <cstdarg>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <functional>
#include <mutex>
#include <optional>
#include <string>

#ifdef __MINGW32__
#undef PRId64
#undef PRIu64
#undef PRIX64
#define PRId64 "I64d"
#define PRIu64 "I64u"
#define PRIX64 "I64X"
#define PRIzu "Iu"
#else
#define PRIzu "zu"
#endif

#ifdef CONF_FAMILY_UNIX
#include <sys/un.h>
#endif

#ifdef CONF_PLATFORM_LINUX
#include <netinet/in.h>
#include <sys/socket.h>
#endif

 

 
enum
{
	 
	IOFLAG_READ = 1,
	 
	IOFLAG_WRITE = 2,
	 
	IOFLAG_APPEND = 4,
};

 
enum ESeekOrigin
{
	 
	IOSEEK_START = 0,
	 
	IOSEEK_CUR = 1,
	 
	IOSEEK_END = 2,
};

 
IOHANDLE io_open(const char *filename, int flags);

 
unsigned io_read(IOHANDLE io, void *buffer, unsigned size);

 
bool io_read_all(IOHANDLE io, void **result, unsigned *result_len);

 
char *io_read_all_str(IOHANDLE io);

 
int io_skip(IOHANDLE io, int64_t size);

 
int io_seek(IOHANDLE io, int64_t offset, ESeekOrigin origin);

 
int64_t io_tell(IOHANDLE io);

 
int64_t io_length(IOHANDLE io);

 
unsigned io_write(IOHANDLE io, const void *buffer, unsigned size);

 
bool io_write_newline(IOHANDLE io);

 
int io_close(IOHANDLE io);

 
int io_flush(IOHANDLE io);

 
int io_sync(IOHANDLE io);

 
int io_error(IOHANDLE io);

 
IOHANDLE io_stdin();

 
IOHANDLE io_stdout();

 
IOHANDLE io_stderr();

 
IOHANDLE io_current_exe();

 
typedef struct ASYNCIO ASYNCIO;

 
ASYNCIO *aio_new(IOHANDLE io);

 
void aio_lock(ASYNCIO *aio);

 
void aio_unlock(ASYNCIO *aio);

 
void aio_write(ASYNCIO *aio, const void *buffer, unsigned size);

 
void aio_write_newline(ASYNCIO *aio);

 
void aio_write_unlocked(ASYNCIO *aio, const void *buffer, unsigned size);

 
void aio_write_newline_unlocked(ASYNCIO *aio);

 
int aio_error(ASYNCIO *aio);

 
void aio_close(ASYNCIO *aio);

 
void aio_wait(ASYNCIO *aio);

 
void aio_free(ASYNCIO *aio);

 

 
void *thread_init(void (*threadfunc)(void *), void *user, const char *name);

 
void thread_wait(void *thread);

 
void thread_yield();

 
void thread_detach(void *thread);

 
void thread_init_and_detach(void (*threadfunc)(void *), void *user, const char *name);

 

#if defined(CONF_FAMILY_WINDOWS)
typedef void *SEMAPHORE;
#elif defined(CONF_PLATFORM_MACOS)
#include <semaphore.h>
typedef sem_t *SEMAPHORE;
#elif defined(CONF_FAMILY_UNIX)
#include <semaphore.h>
typedef sem_t SEMAPHORE;
#else
#error not implemented on this platform
#endif

 
void sphore_init(SEMAPHORE *sem);
 
void sphore_wait(SEMAPHORE *sem);
 
void sphore_signal(SEMAPHORE *sem);
 
void sphore_destroy(SEMAPHORE *sem);

 

 

 
extern const NETADDR NETADDR_ZEROED;

#ifdef CONF_FAMILY_UNIX
 
typedef int UNIXSOCKET;
 
typedef struct sockaddr_un UNIXSOCKETADDR;
#endif

 
void net_init();

 
int net_host_lookup(const char *hostname, NETADDR *addr, int types);

 
int net_addr_comp(const NETADDR *a, const NETADDR *b);

 
int net_addr_comp_noport(const NETADDR *a, const NETADDR *b);

 
void net_addr_str(const NETADDR *addr, char *string, int max_length, bool add_port);

 
int net_addr_from_url(NETADDR *addr, const char *string, char *host_buf, size_t host_buf_size);

 
bool net_addr_is_local(const NETADDR *addr);

 
int net_addr_from_str(NETADDR *addr, const char *string);

 
int net_set_non_blocking(NETSOCKET sock);

 
int net_set_blocking(NETSOCKET sock);

 
int net_errno();

 
std::string net_error_message();

 
int net_would_block();

 
int net_socket_read_wait(NETSOCKET sock, std::chrono::nanoseconds nanoseconds);

 

 
int net_socket_type(NETSOCKET sock);

 
NETSOCKET net_udp_create(NETADDR bindaddr);

 
int net_udp_send(NETSOCKET sock, const NETADDR *addr, const void *data, int size);

 
int net_udp_recv(NETSOCKET sock, NETADDR *addr, unsigned char **data);

 
void net_udp_close(NETSOCKET sock);

 

 
NETSOCKET net_tcp_create(NETADDR bindaddr);

 
int net_tcp_listen(NETSOCKET sock, int backlog);

 
int net_tcp_accept(NETSOCKET sock, NETSOCKET *new_sock, NETADDR *addr);

 
int net_tcp_connect(NETSOCKET sock, const NETADDR *addr);

 
int net_tcp_connect_non_blocking(NETSOCKET sock, NETADDR bindaddr);

 
int net_tcp_send(NETSOCKET sock, const void *data, int size);

 
int net_tcp_recv(NETSOCKET sock, void *data, int maxsize);

 
void net_tcp_close(NETSOCKET sock);

#if defined(CONF_FAMILY_UNIX)
 

 
UNIXSOCKET net_unix_create_unnamed();

 
int net_unix_send(UNIXSOCKET sock, UNIXSOCKETADDR *addr, void *data, int size);

 
void net_unix_set_addr(UNIXSOCKETADDR *addr, const char *path);

 
void net_unix_close(UNIXSOCKET sock);

#endif

 

 
[[gnu::format(printf, 3, 0)]] int str_format_v(char *buffer, int buffer_size, const char *format, va_list args);

 
[[gnu::format(printf, 3, 4)]] int str_format(char *buffer, int buffer_size, const char *format, ...);

#if !defined(CONF_DEBUG)
int str_format_int(char *buffer, size_t buffer_size, int value);

template<typename... Args>
int str_format_opt(char *buffer, int buffer_size, const char *format, Args... args)
{
	static_assert(sizeof...(args) > 0, "Use str_copy instead of str_format without format arguments");
	return str_format(buffer, buffer_size, format, args...);
}

template<>
inline int str_format_opt(char *buffer, int buffer_size, const char *format, int val)  
{
	if(strcmp(format, "%d") == 0)
	{
		return str_format_int(buffer, buffer_size, val);
	}
	else
	{
		return str_format(buffer, buffer_size, format, val);
	}
}

#define str_format str_format_opt
#endif

 
int str_utf8_dist(const char *a, const char *b);

 
int str_utf8_dist_buffer(const char *a, const char *b, int *buf, int buf_len);

 
int str_utf32_dist_buffer(const int *a, int a_len, const int *b, int b_len, int *buf, int buf_len);

 

 
void fs_listdir(const char *dir, FS_LISTDIR_CALLBACK cb, int type, void *user);

 
void fs_listdir_fileinfo(const char *dir, FS_LISTDIR_CALLBACK_FILEINFO cb, int type, void *user);

 
int fs_makedir_rec_for(const char *path);

 
int fs_storage_path(const char *appname, char *path, int max);

 
int fs_executable_path(char *buffer, int buffer_size);

 
int fs_is_file(const char *path);

 
int fs_is_dir(const char *path);

 
int fs_is_relative_path(const char *path);

 
int fs_chdir(const char *path);

 
char *fs_getcwd(char *buffer, int buffer_size);

 
const char *fs_filename(const char *path);

 
void fs_split_file_extension(const char *filename, char *name, size_t name_size, char *extension = nullptr, size_t extension_size = 0);

 
void fs_normalize_path(char *path);

 
int fs_parent_dir(char *path);

 
int fs_remove(const char *filename);

 
int fs_rename(const char *oldname, const char *newname);

 
int fs_file_time(const char *name, time_t *created, time_t *modified);

 
void swap_endian(void *data, unsigned elem_size, unsigned num);

void net_stats(NETSTATS *stats);

int str_utf8_to_skeleton(const char *str, int *buf, int buf_len);

 
int str_utf8_comp_confusable(const char *str1, const char *str2);

 
int str_utf8_tolower_codepoint(int code);

 
unsigned bytes_be_to_uint(const unsigned char *bytes);

 
void uint_to_bytes_be(unsigned char *bytes, unsigned value);

 

 
int pid();

 
void cmdline_fix(int *argc, const char ***argv);

 
void cmdline_free(int argc, const char **argv);

#if !defined(CONF_PLATFORM_ANDROID)
 
enum class EShellExecuteWindowState
{
	 
	FOREGROUND,

	 
	BACKGROUND,
};

 
PROCESS shell_execute(const char *file, EShellExecuteWindowState window_state, const char **arguments = nullptr, size_t num_arguments = 0);

 
int kill_process(PROCESS process);

 
bool is_process_alive(PROCESS process);

 
int open_link(const char *link);

 
int open_file(const char *path);
#endif  

 

 
void generate_password(char *buffer, unsigned length, const unsigned short *random, unsigned random_length);

 
void secure_random_password(char *buffer, unsigned length, unsigned pw_length);

 
void secure_random_fill(void *bytes, unsigned length);

 
int secure_rand_below(int below);

 
bool os_version_str(char *version, size_t length);

 
void os_locale_str(char *locale, size_t length);

 

 
void crashdump_init_if_available(const char *log_file_path);

 
class CCmdlineFix
{
	int m_Argc;
	const char **m_ppArgv;

public:
	CCmdlineFix(int *pArgc, const char ***pppArgv)
	{
		cmdline_fix(pArgc, pppArgv);
		m_Argc = *pArgc;
		m_ppArgv = *pppArgv;
	}
	~CCmdlineFix()
	{
		cmdline_free(m_Argc, m_ppArgv);
	}
	CCmdlineFix(const CCmdlineFix &) = delete;
};

#endif
