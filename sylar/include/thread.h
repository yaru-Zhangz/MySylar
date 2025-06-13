#ifndef __SYLAR_THREAD_H__
#define __SYLAR_THREAD_H__
#include <thread>
#include <pthread.h>
#include <functional>
#include <memory>
#include <shared_mutex>
#include <semaphore>
namespace sylar {

template<class T>
struct ScopedLockImpl {
public:
    ScopedLockImpl(T& mutex)
        :m_mutex(mutex) {
        m_mutex.lock();
        m_locked = true;
    }

    ~ScopedLockImpl() {
        unlock();
    }

    void lock() {
        if(!m_locked) {
            m_mutex.lock();
            m_locked = true;
        }
    }

    void unlock() {
        if(m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }
private:
    T& m_mutex;
    bool m_locked;
};

class Spinlock {
public:
    using Lock = ScopedLockImpl<Spinlock>;
    Spinlock() {
        pthread_spin_init(&m_mutex, 0);
    }

    ~Spinlock() {
        pthread_spin_destroy(&m_mutex);
    }

    void lock() {
        pthread_spin_lock(&m_mutex);
    }

    void unlock() {
        pthread_spin_lock(&m_mutex);
    }
private:
    pthread_spinlock_t m_mutex;
};

// 封装线程
class Thread {
public:
    using ptr = std::shared_ptr<Thread>;
    Thread(std::function<void()> cb, const std::string& name);
    ~Thread();

    pid_t getId() const {return m_id;}
    const std::string& getName() const {return m_name;}

    void join();
    static Thread* GetThis();
    static const std::string GetName();
    static void SetName (const std::string& name);
    static void* run(void* arg);
private:
    Thread(const Thread&) = delete;
    Thread(const Thread&&) = delete;
    Thread& operator=(const Thread&) = delete;
private:
    pid_t m_id = -1;            // 线程ID：是操作系统内核分配给线程的唯一数字标识符
    pthread_t m_thread = 0;     // 线程句柄：是操作系统内部线程对象的一个引用/指针
    std::function<void()> m_cb; // 线程执行函数
    std::string m_name;         // 线程名
    std::counting_semaphore<1> m_sem{0};
};
}
#endif