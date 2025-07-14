#include "util.h"
#include "log.h"
#include "fiber.h"
#include <execinfo.h>
#include <sys/syscall.h>
#include<sys/time.h>
#include <unistd.h>
#include <chrono>
namespace sylar {

sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

    pid_t GetThreadId() {
        return syscall(SYS_gettid);
    }

    uint32_t GetFiberId() {
        return sylar::Fiber::GetFiberId();
    }

    void Backtrace(std::vector<std::string>& bt, int size, int skip) {
        void** array = (void**)malloc((sizeof(void*) * size));
        size_t s = ::backtrace(array, size);

        char** strings = backtrace_symbols(array, s);
        if(strings == nullptr) {
            SYLAR_LOG_ERROR(g_logger) << "backtrace_symbols error";
            return;
        }

        for(size_t i = skip; i < s; i++) {
            bt.push_back(strings[i]);
        }

        free(strings);
        free(array);

    }


    std::string BacktraceToString(int size, int skip, const std::string& prefix) {
        std::vector<std::string> bt;
        Backtrace(bt, size, skip);
        std::stringstream ss;
        for(size_t i = 0; i < bt.size(); i++) {
            ss << prefix << bt[i] << std::endl;
        }
        return ss.str();
    }

    uint64_t GetCurrentMS() {
        using namespace std::chrono;
        return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    }
    uint64_t GetCurrentUS() {
        using namespace std::chrono;
        return duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
    }
}