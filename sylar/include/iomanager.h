#ifndef __IOMANAGER_H__
#define __IOMANAGER_H__


#include "scheduler.h"

namespace sylar {

class IOManager : public Scheduler {
public:
    using ptr = std::shared_ptr<IOManager>;
    
    enum Event {
        NONE = 0x0,
        READ = 0x1,
        WRITE = 0x2
    };

private:    
    struct FdContext {
        struct EventContext {
            Scheduler* scheduler = nullptr;    // 事件执行的scheduler
            Fiber::ptr fiber;                  // 事件协程
            std::function<void()> cb;          // 事件的回调函数
        };
        EventContext& getContext(Event event);
        void resetContext(EventContext& ctx);
        void triggerEvent(Event event);
        
        int fd = 0;
        EventContext read;                      // 读事件
        EventContext write;                     // 写事件
        Event events = NONE;                  // 事件关联的句柄
        std::mutex mutex;
    };

public:
    IOManager(size_t threads = 1, bool use_caller = true, const std::string& name = "");
    ~IOManager();

    // 1 success, 0 retry, -1 error
    int addEvent(int fd, Event event, std::function<void()> cb = nullptr);
    bool delEvent(int fd, Event event);
    bool cancelEvent(int fd, Event event);

    bool cancelAll(int fd);

    static IOManager* GetThis();

protected:
    void tickle() override;
    bool stopping() override;
    void idle() override;

    void contextResize(size_t size);
private:
    int m_epfd = 0;
    int m_tickleFds[2];

    std::atomic<size_t> m_pendingEventCount = {0};
    std::shared_mutex m_mutex;
    std::vector<FdContext*> m_fdContexts;
};

}

#endif