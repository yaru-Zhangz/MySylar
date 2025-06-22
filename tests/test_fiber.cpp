#include "../sylar/include/sylar.h"

sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void run_in_fiber() {
    SYLAR_LOG_INFO(g_logger) << "run_in_fiber begin";
    sylar::Fiber::YieldToHold();
    SYLAR_LOG_INFO(g_logger) << "run_in_fiber end";
    sylar::Fiber::YieldToHold();
}
void test_fiber()
{
    SYLAR_LOG_INFO(g_logger) << "main begin -1";
    {
        sylar::Fiber::GetThis();                                    // 创建线程的主协程, 初始化ucontext_t结构，不分配额外的栈空间
        SYLAR_LOG_INFO(g_logger) << "main begin";                   
        sylar::Fiber::ptr fiber(new sylar::Fiber(run_in_fiber));    // 创建工作协程，分配栈空间
        fiber->swapIn();                                            // 保存主协程，切换到子协程
        SYLAR_LOG_INFO(g_logger) << "main after swapIn";
        fiber->swapIn();
        SYLAR_LOG_INFO(g_logger) << "main after end";
        fiber->swapIn();
    }
    SYLAR_LOG_INFO(g_logger) << "main after end2";
}
int main(int argc, char** argv) {
    sylar::Thread::SetName("main");
    std::vector<sylar::Thread::ptr> thrs;
    for(int i = 0; i < 3; i++)
    {
        thrs.push_back(sylar::Thread::ptr(new sylar::Thread(&test_fiber, "name_" + std::to_string(i))));
    }
    for(auto i : thrs) {
        i->join();
    }
    return 0;
}