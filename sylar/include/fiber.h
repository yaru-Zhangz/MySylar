#ifndef __SYLAR_FIBER_H__
#define __SYLAR_FIBER_H__
#include <ucontext.h>
#include <memory>
#include <functional>

namespace sylar {

class Scheduler;

class Fiber : public std::enable_shared_from_this<Fiber> {
friend class Scheduler;
public:
    using ptr = std::shared_ptr<Fiber>;

    enum State {
        INIT,   // 初始状态
        HOLD,   // 暂停状态
        EXEC,   // 正在执行
        TERM,   // 已终止
        READY,  // 准备就绪，可执行
        EXCEP   // 发生异常
    };
private:
    // 主协程构造函数，用于创建线程的主协程，不分配栈空间，直接使用线程栈，只能由GetThis方法调用
    Fiber();    
    
public:
    // 子协程构造函数 (协程执行的函数， 协程栈的大小, 是否在MainFiber上调度)
    Fiber(std::function<void()> cb, size_t stacksize = 0, bool use_caller = false);
    ~Fiber();
    
    void reset(std::function<void()> cb);   // 重置协程函数，并设置状态
    void swapIn();      // 切换到当前协程执行
    void swapOut();     // 切换到后台执行

    void call();        // 将当前线程切换到执行状态，且执行的为当前线程的主协程
    void back();        // 将当前线程切换到后台，执行的为该线程的主协程

    uint64_t getId() const {return m_id;}
    State getState() const { return m_state;}
public:
    static void SetThis(Fiber* f);      // 设置当前线程正在执行的协程
    static Fiber::ptr GetThis();        // 返回当前协程
    static void YieldToReady();         // 协程切换到后台，并且设置状态为Ready状态
    static void YieldToHold();          // 协程切换到后台，并且设置状态为Hold状态
    static uint64_t TotalFibers();      // 获取总协程数
    static void MainFunc();             // 协程的主执行函数，执行完成返回线程主协程
    static void CallerMainFunc();       // 协程执行函数，执行完成返回到线程调度协程
    static uint64_t GetFiberId();
private:
    uint64_t m_id = 0;          // 协程id
    uint32_t m_stacksize = 0;   // 协程运行栈大小
    State m_state = INIT;       // 协程状态
    ucontext_t m_ctx;           // 协程上下文
    void* m_stack = nullptr;    // 协程运行栈指针
    std::function<void()> m_cb; // 协程执行的函数对象
};

}

#endif