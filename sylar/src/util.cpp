#include "util.h"

namespace sylar {
    pid_t GetThreadId() {
        return static_cast<uint32_t>(std::hash<std::thread::id>()(std::this_thread::get_id()));
    }

    uint32_t GetFiberId() {
        return 0;
    }
}