#ifndef __SYLAR_UTIL_H__
#define __SYLAR_UTIL_H__
#include <pthread.h>
#include <thread>
#include <vector>
#include <string>
#include <stdint.h>
#include <execinfo.h>
namespace sylar {

    pid_t GetThreadId();
    uint32_t GetFiberId();

    void Backtrace(std::vector<std::string>& bt, int size, int skip); 
    std::string BacktraceToString(int size = 64, int skip = 2, const std::string& prefix = "");
}

#endif
