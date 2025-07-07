#include "scheduler.h"
#include "log.h"
#include "macro.h"
#include "hook.h"
namespace sylar {
static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

static thread_local Scheduler* t_scheduler = nullptr;   // 存储当前线程的调度器实例
static thread_local Fiber* t_scheduler_fiber = nullptr;           // 存储当前线程的主协程

Scheduler::Scheduler(size_t threads, bool use_caller, const std::string& name)
    :m_name(name) {
    SYLAR_ASSERT(threads > 0);

    if(use_caller) {                // 是否将调用线程也作为工作线程
        sylar::Fiber::GetThis();    // 创建当前线程的主协程
        --threads;                  // 使用调用线程作为工作线程，减少需要创建的线程数

        SYLAR_ASSERT(GetThis() == nullptr);
        t_scheduler = this;         // 设置当前线程的调度器

        m_rootFiber.reset(new Fiber(std::bind(&Scheduler::run, this), 0, true)); // 创建调度器主协程，绑定run方法作为入口函数
        sylar::Thread::SetName(m_name);

        t_scheduler_fiber = m_rootFiber.get();// 返回智能指针内部保存的原始裸指针
        m_rootThread = sylar::GetThreadId();
        m_threadIds.push_back(m_rootThread);
    } else {     // 调用线程不作为工作线程，m_rootThread = -1
        m_rootThread = -1;
    }
    m_threadCount = threads;
}

Scheduler::~Scheduler() {
    SYLAR_ASSERT(m_stopping);
    if(GetThis() == this) {
        t_scheduler = nullptr;
    }
}


Scheduler* Scheduler::GetThis() {
    return t_scheduler;
}

Fiber* Scheduler::GetMainFiber() {
    return t_scheduler_fiber;
}

void Scheduler::start() {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if(!m_stopping) {
            return;     // 防止重复启动
        }
        m_stopping = false;
        SYLAR_ASSERT(m_threads.empty());

        m_threads.resize(m_threadCount);

        for(size_t i = 0; i < m_threadCount; i++) {
            // 创建工作线程，绑定run方法
            m_threads[i].reset(new Thread(std::bind(&Scheduler::run, this), m_name + "_" + std::to_string(i)));
            m_threadIds.push_back(m_threads[i]->getId());
        }
    }
    // if(m_rootFiber) {   // 调用线程作为工作线程，调用线程切换成工作线程开始执行任务，主线程也开始执行run
    //     m_rootFiber->swapIn();
    // }
}

void Scheduler::stop() {
    m_autoStop = true;
    if(m_rootFiber
            && m_threadCount == 0
            && (m_rootFiber->getState() == Fiber::TERM
                || m_rootFiber->getState() == Fiber::INIT)) {
        SYLAR_LOG_INFO(g_logger) << this << " stopped";
        m_stopping = true;
        if(stopping()) {
            return;
        }
    }

    //bool exit_on_this_fiber = false;
    if(m_rootThread != -1) {
        SYLAR_ASSERT(GetThis() == this);
    } else {
        SYLAR_ASSERT(GetThis() != this);
    }

    m_stopping = true;
    for(size_t i = 0; i < m_threadCount; ++i) {
        tickle();
    }

    if(m_rootFiber) {
        tickle();
    }

    if(m_rootFiber) {
        if(!stopping()) {
            m_rootFiber->call();
        }
    }

    std::vector<Thread::ptr> thrs;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        thrs.swap(m_threads);
    }

    for(auto& i : thrs) {
        i->join();
    }
}

void Scheduler::setThis() {
    t_scheduler = this;
}

// 工作线程的主协程在run里面通过GetThis创建
void Scheduler::run() {
    SYLAR_LOG_DEBUG(g_logger) << m_name << " run";
    // set_hook_enable(true);
    setThis(); // 设置当前线程的调度器实例
    
    // 工作线程需要初始化自己的主协程
    if (sylar::GetThreadId() != m_rootThread) {
        t_scheduler_fiber = Fiber::GetThis().get();
    }

    // 准备空闲协程和回调协程容器
    Fiber::ptr idle_fiber(new Fiber(std::bind(&Scheduler::idle, this)));    // 当任务队列为空时，调度器线程会切换到这个协程
    Fiber::ptr cb_fiber;    
    FiberAndThread ft;
    // ==================== 主调度循环 ====================
    while (true) {
        // ----------- 任务获取阶段（加锁临界区）-----------
        ft.reset();
        bool tickle_me = false;
        bool is_active = false;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            auto it = m_fibers.begin();

            // 遍历任务队列寻找可执行任务
            while (it != m_fibers.end()) {
                // 跳过指定到其他线程的任务
                if (it->thread != -1 && it->thread != sylar::GetThreadId()) {
                    ++it;
                    tickle_me = true;
                    continue;
                }
                SYLAR_ASSERT(it->fiber || it->cb);
                // 跳过正在执行的协程
                if (it->fiber && it->fiber->getState() == Fiber::EXEC) {
                    ++it;
                    continue;
                }

                // 找到有效任务
                ft = *it;
                m_fibers.erase(it++);
                ++m_activeThreadCount;
                is_active = true;
                break;
            }
            tickle_me |= it != m_fibers.end(); // 检查是否有剩余任务
        }
        
        // ----------- 协作式调度通知 -----------
        if (tickle_me) {
            tickle(); // 唤醒其他可能空闲的线程
        }
        
        // ----------- 任务执行阶段（无锁环境）-----------
                if(ft.fiber && (ft.fiber->getState() != Fiber::TERM
                        && ft.fiber->getState() != Fiber::EXCEP)) {
            ft.fiber->swapIn();
            --m_activeThreadCount;

            if(ft.fiber->getState() == Fiber::READY) {
                schedule(ft.fiber);
            } else if(ft.fiber->getState() != Fiber::TERM
                    && ft.fiber->getState() != Fiber::EXCEP) {
                ft.fiber->m_state = Fiber::HOLD;
            }
            ft.reset();
        } else if(ft.cb) {
            if(cb_fiber) {
                cb_fiber->reset(ft.cb);
            } else {
                cb_fiber.reset(new Fiber(ft.cb));
            }
            ft.reset();
            cb_fiber->swapIn();
            --m_activeThreadCount;
            if(cb_fiber->getState() == Fiber::READY) {
                schedule(cb_fiber);
                cb_fiber.reset();
            } else if(cb_fiber->getState() == Fiber::EXCEP
                    || cb_fiber->getState() == Fiber::TERM) {
                cb_fiber->reset(nullptr);
            } else {//if(cb_fiber->getState() != Fiber::TERM) {
                cb_fiber->m_state = Fiber::HOLD;
                cb_fiber.reset();
            }
        } else {
            if(is_active) {
                --m_activeThreadCount;
                continue;
            }
            if(idle_fiber->getState() == Fiber::TERM) {
                SYLAR_LOG_INFO(g_logger) << "idle fiber term";
                break;
            }

            ++m_idleThreadCount;
            idle_fiber->swapIn();
            --m_idleThreadCount;
            if(idle_fiber->getState() != Fiber::TERM
                    && idle_fiber->getState() != Fiber::EXCEP) {
                idle_fiber->m_state = Fiber::HOLD;
            }
        }
    } // end while
}

void Scheduler::tickle() {
    SYLAR_LOG_INFO(g_logger) << "tickle";
}

bool Scheduler::stopping() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_autoStop && m_stopping && m_fibers.empty() && m_activeThreadCount == 0;
}

void Scheduler::idle() {
    SYLAR_LOG_INFO(g_logger) << "idle";
    while(!stopping()) {
        sylar::Fiber::YieldToHold();
    }
}

void Scheduler::switchTo(int thread) {
    SYLAR_ASSERT(Scheduler::GetThis() != nullptr);
    if(Scheduler::GetThis() == this) {
        if(thread == -1 || thread == sylar::GetThreadId()) {
            return;
        }
    }
    schedule(Fiber::GetThis(), thread);
    Fiber::YieldToHold();
}

std::ostream& Scheduler::dump(std::ostream& os) {
    os << "[Scheduler name=" << m_name
       << " size=" << m_threadCount
       << " active_count=" << m_activeThreadCount
       << " idle_count=" << m_idleThreadCount
       << " stopping=" << m_stopping
       << " ]" << std::endl << "    ";
    for(size_t i = 0; i < m_threadIds.size(); ++i) {
        if(i) {
            os << ", ";
        }
        os << m_threadIds[i];
    }
    return os;
}

SchedulerSwitcher::SchedulerSwitcher(Scheduler* target) {
    m_caller = Scheduler::GetThis();
    if(target) {
        target->switchTo();
    }
}

SchedulerSwitcher::~SchedulerSwitcher() {
    if(m_caller) {
        m_caller->switchTo();
    }
}

}