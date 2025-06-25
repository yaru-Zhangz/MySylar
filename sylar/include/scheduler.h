#ifndef __SYLAR_SCHEDULER_H__
#define __SYLAR_SCHEDULER_H__
#include <memory>
#include <mutex>
#include <atomic>
#include <vector>
#include <list>

#include "fiber.h"
#include "thread.h"

namespace sylar {

/*
    N-M的协程调度器，内部有一个线程池，支持协程在线程池里面切换
    1. N个工作线程调度M个协程任务
    2. 使用std::vector<Thread::ptr> m_threads管理工作线程
    3. 使用std::list<FiberAndThread> m_fibers管理待执行任务
    4. 支持指定线程执行任务
*/
class Scheduler {
public:
    using ptr = std::shared_ptr<Scheduler>;

    // <工作线程的数量，是否使用调用线程作为工作线程，调度器的名称>
    Scheduler(size_t threads = 1, bool use_caller = true, const std::string& name = "");
    virtual ~Scheduler();

    const std::string& getName() const {return m_name;}
    static Scheduler* GetThis();        // 获取当前线程的调度器实例
    static Fiber* GetMainFiber();

    void start();   // 启动调度器
    void stop();    // 停止调度器

    // 单个任务调度
    template<class FiberOrCb>
    void schedule(FiberOrCb fc, int thread = -1) {
        bool need_tickle = false;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            need_tickle = schedulerNoLock(fc, thread);
        }

        if(need_tickle) {
            tickle();
        }
    }

    // 批量任务调度
    template<class InputIterator>
    void schedule(InputIterator begin, InputIterator end) {
        bool need_tickle = false;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            while(begin != end) {
                need_tickle = schedulerNoLock(&*begin, -1) || need_tickle;
            }
        }
        if(need_tickle) {
            tickle();
        }
    }
    void switchTo(int thread = -1);
    std::ostream& dump(std::ostream& os);
protected:
    // 唤醒空闲线程
    virtual void tickle();
    void run();
    virtual bool stopping();
    virtual void idle();

    void setThis();
    bool hasIdleThread() {return m_idleThreadCount > 0;}
private:

    // 把任务封装为FiberAndThread添加到队列，返回是否需要唤醒工作线程
    template<class FiberOrCb>
    bool schedulerNoLock(FiberOrCb fc, int thread) {
        bool need_tickle = m_fibers.empty();
        FiberAndThread ft(fc, thread);
        if(ft.fiber || ft.cb) {
            m_fibers.push_back(ft);
        }
        return need_tickle;
    }
private:
    // 封装任务
    struct FiberAndThread {
        Fiber::ptr fiber;
        std::function<void()> cb;
        int thread;

        // 多种构造方式
        // 统一封装调度任务，支持两种任务形式：协程对象和回调函数，并可制定目标执行线程
        FiberAndThread(Fiber::ptr f, int thr):fiber(f), thread(thr) {}
        FiberAndThread(Fiber::ptr* f, int thr):thread(thr) { fiber.swap(*f); }
        FiberAndThread(std::function<void()> f, int thr):cb(f), thread(thr) {}
        FiberAndThread(std::function<void()>* f, int thr):thread(thr) { cb.swap(*f); }
        FiberAndThread():thread(-1) {}

        void reset() {
            fiber = nullptr;
            cb = nullptr;
            thread = -1;
        }
    };
private:
    std::vector<Thread::ptr> m_threads;     // 工作线程池
    std::list<FiberAndThread> m_fibers;     // 任务队列
    Fiber::ptr m_rootFiber;
    std::mutex m_mutex;                     
    std::string m_name;

protected:
    std::vector<int> m_threadIds;
    size_t m_threadCount = 0;
    std::atomic<size_t> m_activeThreadCount = {0};
    std::atomic<size_t> m_idleThreadCount = {0};
    bool m_stopping = true;
    bool m_autoStop = false;
    int m_rootThread = 0;
};

class SchedulerSwitcher : public Noncopyable {
public:
    SchedulerSwitcher(Scheduler* target = nullptr);
    ~SchedulerSwitcher();
private:
    Scheduler* m_caller;
};

}

#endif