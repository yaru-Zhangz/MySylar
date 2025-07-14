#ifndef __SYLAR_TIMER_H__
#define __SYLAR_TIMER_H__

#include <memory>
#include <mutex>
#include <shared_mutex>
#include <vector>
#include <set>
#include "thread.h"

namespace sylar {

class TimerManager;

class Timer : public std::enable_shared_from_this<Timer> {

friend class TimerManager;

public:
    using ptr = std::shared_ptr<Timer>;
    bool cancel();                              // 取消定时器                   
    bool refresh();                             // 刷新定时器
    bool reset(uint64_t ms, bool from_now);     // 重置定时器

private:
    Timer(uint64_t ms, std::function<void()> cb, bool recurring, TimerManager* manager);    // 通过TimerManager创建
    Timer(uint64_t next);  
private:
    bool m_recurring = false;           // 是否为循环定时器
    uint64_t m_ms = 0;                  // 执行周期（毫秒）
    uint64_t m_next = 0;                // 下次执行时间
    std::function<void()> m_cb;
    TimerManager* m_manager = nullptr;  // 所属管理器
private:

    struct Comparator {
        bool operator()(const Timer::ptr& lhs, const Timer::ptr& rhs) const;
    };
};


// 定时器管理器
class TimerManager {

friend class Timer;

public:
    TimerManager();
    virtual ~TimerManager();
    Timer::ptr addTimer(uint64_t ms, std::function<void()> cb, bool recurring = false);
    Timer::ptr addConditionTimer(uint64_t ms, std::function<void()> cb, 
                    std::weak_ptr<void> weak_cond, bool recurring = false);     // 只有当weak_cond不为空时才会执行回调，避免对象已销毁时的无效回调

    uint64_t getNextTimer();                    // 到最近一个定时器执行的时间间隔(毫秒)
    void listExpiredCb(std::vector<std::function<void()> >& cbs);
    bool hasTimer();

protected:
    virtual void onTimerInsertedAtFront() = 0;  // 当有新的定时器插入到定时器的首部,执行该函数
    void addTimer(Timer::ptr val, std::unique_lock<std::shared_mutex>& lock);

private:
    bool detectClockRollover(uint64_t now_ms);  // 检测服务器时间是否被调后，如果发现时间倒退，重置所有定时器
private:
    std::shared_mutex m_mutex;
    std::set<Timer::ptr, Timer::Comparator> m_timers;   // 定时器集合
    bool m_tickled = false;                             // 是否触发onTimerInsertedAtFront
    uint64_t m_previouseTime = 0;                       // 上次执行时间
};

}

#endif
