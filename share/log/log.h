#ifndef WUKONG_LOG_H
#define WUKONG_LOG_H

#include <cstdio>
#include <cstring>

class Log
{
public:
//    void Info(const char *__restrict __format, ...);
};

#ifdef WIN32
#define __OS_PATH_SEP__ '\\'
#else
#define __OS_PATH_SEP__ '/'
#endif

#ifdef LOG_FULL_FILE_NAME
#define __FILENAME__ __FILE__
#else
#define __FILENAME__ (strrchr(__FILE__, __OS_PATH_SEP__) + 1)
#endif

#define LogDebug(format, ...)  printf("%s(%s:%d) " format "\n", __FUNCTION__, __FILENAME__, __LINE__, ## __VA_ARGS__)

#define LogInfo(format, ...)  printf("%s(%s:%d) " format "\n", __FUNCTION__, __FILENAME__, __LINE__, ## __VA_ARGS__)

#define LogWarn(format, ...)  printf("%s(%s:%d) " format "\n", __FUNCTION__, __FILENAME__, __LINE__, ## __VA_ARGS__)

#define LogError(format, ...)  printf("%s(%s:%d) " format "\n", __FUNCTION__, __FILENAME__, __LINE__, ## __VA_ARGS__)

#define LogFatal(format, ...)  printf("%s(%s:%d) " format "\n", __FUNCTION__, __FILENAME__, __LINE__, ## __VA_ARGS__)

#endif //WUKONG_LOG_H
