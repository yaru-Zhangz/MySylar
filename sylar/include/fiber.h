#ifndef __SYLAR_FIBER_H__
#define __SYLAR_FIBER_H__
#include <ucontext.h>
#include <memory>
#include <functional>
#include "thread.h"

namespace sylar {
class Fiber : public std::enable_shared_from_this<Fiber> {
public:
    using ptr = std::shared_ptr<Fiber>;

    enum State {
        INIT,
        HLOD,
        EXEC,
        TERM,
        READY
    };
private:
    Fiber();
    
public:
    Fiber(std::function<void()> cb, size_t stacksize = 0);
    ~Fiber();
    // 重置协程函数，并重置状态
    // INIT, TERM
    void reset(std::function<void()> cb);
    // 切换到当前协程执行
    void swapIn();
    // 切换到后台执行
    void swapOut();
public:
    static void SetThis(Fiber* f);
    // 返回当前协程
    static Fiber::ptr GetThis();
    // 协程切换到后台，并且设置状态为Ready状态
    static void YieldToReady();
    // 协程切换到后台，并且设置状态为Hold状态
    static void YieldToHold();
    // 总协程数
    static uint64_t TotalFibers();

    static MainFunc();
private:
    uint64_t m_id = 0;
    uint32_t m_stacksize = 0;
    State m_state = INIT;

    ucontext_t m_ctx;
    void* m_stack = nullptr;

    std::function<void()> m_cb;

};



}

#endif