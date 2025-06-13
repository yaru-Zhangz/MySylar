#include "../sylar/include/sylar.h"
#include<unistd.h>
sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

int count = 0;
std::shared_mutex mutex_ ;

void func1() {
    SYLAR_LOG_INFO(g_logger) << "name: " << sylar::Thread::GetName()
                             << " this.name: " << sylar::Thread::GetThis()->getName()
                             << " id: " << sylar::GetThreadId()
                             << " this.id: " << sylar::Thread::GetThis()->getId();
    for(int i = 0; i < 1000; i++)
    {
        std::unique_lock<std::shared_mutex> lck(mutex_);
        count++;
    }
}

void func2() {
    while(true)
        SYLAR_LOG_INFO(g_logger) << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
   
}


void func3() {
    while(true)
        SYLAR_LOG_INFO(g_logger) << "=======================================";
}

int main(int argc, char** argv) {
    std::vector<sylar::Thread::ptr> thrs;
    for(int i = 0; i < 5; i++) {
        sylar::Thread::ptr thr(new sylar::Thread(&func3, "name_" + std::to_string(i)));
        sylar::Thread::ptr thr2(new sylar::Thread(&func2, "name_" + std::to_string(i)));
        thrs.push_back(thr);
        thrs.push_back(thr2);
    }

    for(size_t i = 0; i < thrs.size(); i++) {
        thrs[i]->join();
    }

    SYLAR_LOG_INFO(g_logger) << "thread test end";
    SYLAR_LOG_INFO(g_logger) << "count = " << count;
}