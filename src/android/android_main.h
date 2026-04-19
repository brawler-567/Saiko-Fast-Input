#ifndef ANDROID_ANDROID_MAIN_H
#define ANDROID_ANDROID_MAIN_H

#include <base/detect.h>
#if !defined(CONF_PLATFORM_ANDROID)
#error "This header should only be included when compiling for Android"
#endif

#include <cstddef>

 

 
const char *InitAndroid();

 
void RestartAndroidApp();

 
bool StartAndroidServer(const char **ppArguments, size_t NumArguments);

 
void ExecuteAndroidServerCommand(const char *pCommand);

 
bool IsAndroidServerRunning();

#endif  
