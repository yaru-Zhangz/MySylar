#include "../sylar/include/sylar.h"

static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void test_fiber() {
    static int s_count = 5;
    SYLAR_LOG_INFO(g_logger) << "test in fiber s_count=" << s_count;

    sleep(1);
    if(--s_count >= 0) {    // 打印日志，休眠1秒，s_count--，重新调度自己
        sylar::Scheduler::GetThis()->schedule(&test_fiber, sylar::GetThreadId());
    }
}

int main(int argc, char** argv) {
    SYLAR_LOG_INFO(g_logger) << "main";
    sylar::Scheduler sc(3, false, "test");  // 1. 创建调度器，指定3个工作线程，不适用调用线程作为工作线程
    sc.start();                             // 2. 启动调度器，创建3个工作线程，每个线程执行run()方法
    sleep(2);
    SYLAR_LOG_INFO(g_logger) << "schedule"; 
    sc.schedule(&test_fiber);               // 3. 将test_fiber函数作为任务加入调度队列，其中一个工作协程从队列中去除病执行这个任务
    sc.stop();                              // 4. 等待所有任务完成(包括正在执行的test_fiber及其后续调度),停止所有工作线程
    SYLAR_LOG_INFO(g_logger) << "over";
    return 0;
}

/*


*/