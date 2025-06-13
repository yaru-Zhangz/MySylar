#include "util.h"
#include <sys/syscall.h>
#include <unistd.h>
namespace sylar {
    pid_t GetThreadId() {
        return syscall(SYS_gettid);
    }

    uint32_t GetFiberId() {
        return 0;
    }
}