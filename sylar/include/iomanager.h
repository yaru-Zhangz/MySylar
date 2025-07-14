#ifndef __IOMANAGER_H__
#define __IOMANAGER_H__


#include "scheduler.h"
#include "timer.h"

namespace sylar {

class IOManager : public Scheduler, public TimerManager {
public:
    using ptr = std::shared_ptr<IOManager>;
    
    enum Event {
        NONE = 0x0,
        READ = 0x1, // EPOLLIN
        WRITE = 0x4 // EPOLLOUT
    };

private:    
    // 文件描述符上下文信息
    struct FdContext {
        struct EventContext {
            Scheduler* scheduler = nullptr;    // 事件执行的scheduler
            Fiber::ptr fiber;                  // 事件协程
            std::function<void()> cb;          // 事件的回调函数
        };
        EventContext& getContext(Event event);  // 获取指定事件上下文 
        void resetContext(EventContext& ctx);   // 重置事件上下文
        void triggerEvent(Event event);         // 触发指定事件
        
        int fd = 0;                             // 文件描述符
        EventContext read;                      // 读事件上下文
        EventContext write;                     // 写事件上下文
        Event events = NONE;                    // 当前注册的事件
        std::mutex mutex;
    };

public:
    IOManager(size_t threads = 1, bool use_caller = true, const std::string& name = "");
    ~IOManager();

    // 1 success, 0 retry, -1 error
    int addEvent(int fd, Event event, std::function<void()> cb = nullptr);  // 添加事件监听
    bool delEvent(int fd, Event event);                                     // 删除事件监听
    bool cancelEvent(int fd, Event event);                                  // 取消事件监听

    bool cancelAll(int fd);                                                 // 取消该fd的所有事件

    static IOManager* GetThis();                                            // 获取当前线程的IOManager实例

protected:
    void tickle() override;                     // 唤醒空闲线程
    bool stopping() override;                   // 检查是否停止
    bool stopping(uint64_t& timeout);   
    void idle() override;                       // 空闲线程，执行事件循环
    void onTimerInsertedAtFront() override;     // 当有新定时器插入到最前面时回调
    void contextResize(size_t size);            // 调整fd上下文容器大小
private:
    int m_epfd = 0;   // epoll 文件描述
    int m_tickleFd;   // 用于唤醒epoll_wait的eventfd

    std::atomic<size_t> m_pendingEventCount = {0};  // 待处理事件数量
    std::shared_mutex m_mutex;
    std::vector<FdContext*> m_fdContexts;           // 存储所有fd上下文的数组
};

}

#endif