 
 

#ifndef BASE_WINDOWS_H
#define BASE_WINDOWS_H

#include "detect.h"

#if defined(CONF_FAMILY_WINDOWS)

#include <cstdint>
#include <optional>
#include <string>

 

 
std::string windows_format_system_message(unsigned long error);

 
std::wstring windows_args_to_wide(const char **arguments, size_t num_arguments);

 
std::wstring windows_utf8_to_wide(const char *str);

 
std::optional<std::string> windows_wide_to_utf8(const wchar_t *wide_str);

 
class CWindowsComLifecycle
{
public:
	CWindowsComLifecycle(bool HasWindow);
	~CWindowsComLifecycle();
	CWindowsComLifecycle(const CWindowsComLifecycle &) = delete;
};

 
bool windows_shell_register_protocol(const char *protocol_name, const char *executable, bool *updated);

 
bool windows_shell_register_extension(const char *extension, const char *description, const char *executable_name, const char *executable, bool *updated);

 
bool windows_shell_register_application(const char *name, const char *executable, bool *updated);

 
bool windows_shell_unregister_class(const char *shell_class, bool *updated);

 
bool windows_shell_unregister_application(const char *executable, bool *updated);

 
void windows_shell_update();

#endif

#endif
