#include "fiber.h"
#include "config.h"
#include "macro.h"
#include "scheduler.h"
#include "log.h"

#include <atomic>

namespace sylar {

static Logger::ptr g_logger = SYLAR_LOG_NAME("system");
static std::atomic<uint64_t> s_fiber_id{0};         // 用于生成协程id
static std::atomic<uint64_t> s_fiber_count{0};      // 统计当前的协程数

// 不同线程的协程互不影响
static thread_local Fiber* t_fiber = nullptr;               // 指向当前正在执行的协程
static thread_local Fiber::ptr t_threadFiber = nullptr;     // 保存线程的主协程

static ConfigVar<uint32_t>::ptr g_fiber_stack_size = Config::Lookup<uint32_t>("fiber.stack_size", 1024*1024, "fiber stack size");

// 栈分配器，用malloc和free管理协程栈内存
class MallocStackAllocator {
public:
    static void* Alloc(size_t size) {
        return malloc(size);
    }

    static void Dealloc(void* vp, size_t size) {
        return free(vp);
    }
};

using StackAllocator = MallocStackAllocator;

uint64_t Fiber::GetFiberId() {
    if(t_fiber) {
        return t_fiber->getId();
    }
    return 0;
}

Fiber::Fiber() {
    m_state = EXEC;
    SetThis(this);

    if(getcontext(&m_ctx)) {
        SYLAR_ASSERT2(false, "getcontext");
    }
    ++s_fiber_count;

    SYLAR_LOG_DEBUG(g_logger) << "Fiber::main";
}

Fiber::Fiber(std::function<void()> cb, size_t stacksize, bool use_caller) 
    :m_id(++s_fiber_id), m_cb(cb) {
    ++s_fiber_count;
    m_stacksize = stacksize ? stacksize : g_fiber_stack_size->getValue();

    m_stack = StackAllocator::Alloc(m_stacksize);   // 分配指定大小的栈空间
    if(getcontext(&m_ctx)) {                        // getcontext用于获取当前执行上下文并保存到指定的ucontext_t结构中
        SYLAR_ASSERT2(false, "getcontext");
    }
    m_ctx.uc_link = nullptr;                 // 链接上下文
    m_ctx.uc_stack.ss_sp = m_stack;          // 栈基址
    m_ctx.uc_stack.ss_size = m_stacksize;    // 栈大小

    if(!use_caller)
        makecontext(&m_ctx, &Fiber::MainFunc, 0);       // 用于修改一个已通过getcontext获取的上下文，主要是设置新的入口点和栈信息
    else
        makecontext(&m_ctx, &Fiber::CallerMainFunc, 0);
    SYLAR_LOG_DEBUG(g_logger) << "Fiber::Fiber id=" << m_id;
}

Fiber::~Fiber() {
    --s_fiber_count;
    if(m_stack) {
        SYLAR_ASSERT(m_state == TERM || m_state == INIT || m_state == EXCEP);
        StackAllocator::Dealloc(m_stack, m_stacksize);
    } else {
        SYLAR_ASSERT(!m_cb);
        SYLAR_ASSERT(m_state == EXEC);

        Fiber* cur = t_fiber;
        if(cur == this) {
            SetThis(nullptr);
        }
    }
    SYLAR_LOG_DEBUG(g_logger) << "Fiber::~Fiber id=" << m_id << " total=" << s_fiber_count;
}

void Fiber::reset(std::function<void()> cb) {
    SYLAR_ASSERT(m_stack);
    SYLAR_ASSERT(m_state == TERM || m_state == INIT || m_state == EXCEP);
    m_cb = cb;
    if(getcontext(&m_ctx)) {
        SYLAR_ASSERT2(false, "getcontext");
    }

    m_ctx.uc_link = nullptr;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stacksize;

    makecontext(&m_ctx, &Fiber::MainFunc, 0);
    m_state = INIT;
}

void Fiber::call() {
    SetThis(this);
    m_state = EXEC;
    if(swapcontext(&t_threadFiber->m_ctx, &m_ctx)) {
        SYLAR_ASSERT2(false, "swapcontext");
    }
}

void Fiber::back() {
    SetThis(t_threadFiber.get());
    m_state = EXEC;
    if(swapcontext(&m_ctx, &t_threadFiber->m_ctx)) {
        SYLAR_ASSERT2(false, "swapcontext");
    }
}
// 切换到当前协程执行
void Fiber::swapIn() {
    SetThis(this);
    SYLAR_ASSERT(m_state != EXEC);
    m_state = EXEC;
    /*
        保存当前CPU寄存器到第一个参数，从第二个参数加载之前保存的寄存器值，跳转到目标上下文继续执行
    */
    if(swapcontext(&Scheduler::GetMainFiber()->m_ctx, &m_ctx)) {    // 恢复保存的上下文，保存当前上下文到第一个参数，恢复第二参数执行的上下文
        SYLAR_ASSERT2(false, "swapcontext");
    }
}

// 切换到后台执行
void Fiber::swapOut() {
    SetThis(Scheduler::GetMainFiber());

    if(swapcontext(&m_ctx, &Scheduler::GetMainFiber()->m_ctx)) {
        SYLAR_ASSERT2(false, "swapcontext");
    }
}

// 设置当前协程
void Fiber::SetThis(Fiber* f) {
    t_fiber= f;
}

// 获取当前协程
Fiber::ptr Fiber::GetThis() {
    if(t_fiber) {
        return t_fiber->shared_from_this(); 
    }
    // 如果当前线程还未创建协程，则创建线程的第一个协程
    Fiber::ptr main_fiber(new Fiber);
    SYLAR_ASSERT(t_fiber == main_fiber.get());
    t_threadFiber = main_fiber;
    return t_fiber->shared_from_this();
}

// 协程切换到后台，并且设置状态为Ready状态
void Fiber::YieldToReady() {
    Fiber::ptr cur = GetThis();
    SYLAR_ASSERT(cur->m_state == EXEC);
    cur->m_state = READY;
    cur->swapOut();
}

// 协程切换到后台，并且设置状态为Hold状态
void Fiber::YieldToHold() {
    Fiber::ptr cur = GetThis();
    SYLAR_ASSERT(cur->m_state == EXEC);
    cur->m_state = HOLD;
    cur->swapOut();
}
// 总协程数
uint64_t Fiber::TotalFibers() {
    return s_fiber_count;
}

void Fiber::MainFunc() {
    Fiber::ptr cur = GetThis();
    SYLAR_ASSERT(cur);
    try {
        cur->m_cb();
        cur->m_cb = nullptr;
        cur->m_state = TERM;
    } catch (std::exception& ex) {
        cur->m_state = EXCEP;
        SYLAR_LOG_ERROR(g_logger) << "Fiber Except: " << ex.what() << " fiber_id=" << cur->getId() << std::endl << sylar::BacktraceToString();
    } catch (...) {
        cur->m_state = EXCEP;
        SYLAR_LOG_ERROR(g_logger) << "Fiber Except: " << " fiber_id=" << cur->getId() << std::endl << sylar::BacktraceToString();
    }
    auto raw_ptr = cur.get();
    cur.reset();
    raw_ptr->swapOut();

    SYLAR_ASSERT2(false, "never reach fiber_id=" + std::to_string(raw_ptr->getId()));
}

void Fiber::CallerMainFunc() {
    Fiber::ptr cur = GetThis();
    SYLAR_ASSERT(cur);
    try {
        cur->m_cb();
        cur->m_cb = nullptr;
        cur->m_state = TERM;
    } catch (std::exception& ex) {
        cur->m_state = EXCEP;
        SYLAR_LOG_ERROR(g_logger) << "Fiber Except: " << ex.what() << " fiber_id=" << cur->getId() << std::endl << sylar::BacktraceToString();
            
    } catch (...) {
        cur->m_state = EXCEP;
        SYLAR_LOG_ERROR(g_logger) << "Fiber Except" << " fiber_id=" << cur->getId() << std::endl << sylar::BacktraceToString();
          
    }

    auto raw_ptr = cur.get();
    cur.reset();
    raw_ptr->back();
    SYLAR_ASSERT2(false, "never reach fiber_id=" + std::to_string(raw_ptr->getId()));

}

}


