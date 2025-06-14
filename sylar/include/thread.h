#ifndef __SYLAR_THREAD_H__
#define __SYLAR_THREAD_H__
#include <thread>
#include <pthread.h>
#include <functional>
#include <memory>
#include <shared_mutex>
#include <semaphore>
namespace sylar {

// 利用RAII特性在构造时加锁，析构时解锁
template<class T>
class ScopedLockImpl {
public:
    explicit ScopedLockImpl(T& mutex) : m_mutex(mutex) {
        m_mutex.lock();
    }

    ~ScopedLockImpl() {
        m_mutex.unlock();
    }

    // 禁止拷贝和移动
    ScopedLockImpl(const ScopedLockImpl&) = delete;
    ScopedLockImpl& operator=(const ScopedLockImpl&) = delete;

private:
    T& m_mutex;  // 引用方式持有锁
};

// 封装自旋锁
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
        pthread_spin_unlock(&m_mutex);
    }
private:
    pthread_spinlock_t m_mutex;
};

// 封装基于原子操作的自旋锁: 当锁的竞争事件较短时，性能优于传统的互斥锁
class CASLock {
public:
    using Lock = ScopedLockImpl<CASLock>;

    CASLock(): m_mutex(ATOMIC_FLAG_INIT) {}     // 初始化m_mutex为未锁定状态
    ~CASLock() = default;
    /*
        test_and_set() 函数检查 std::atomic_flag 标志，如果 std::atomic_flag 之前没有被设置过，则设置 std::atomic_flag 的标志，
        并返回先前该 std::atomic_flag 对象是否被设置过，如果之前 std::atomic_flag 对象已被设置，则返回 true，否则返回 false。
        std::memory_order_acquire: 确保当前线程在获取锁后，能正确读取其他线程释放锁前的写入
    */

   // 尝试获取锁，如果锁已被占用，则​忙等​，直到锁可用。
    void lock() {
        while(m_mutex.test_and_set(std::memory_order_acquire));
    }

    // std::memory_order_release: 确保当前线程释放锁前的所有写入对其他线程可见
    void unlock() {
        m_mutex.clear(std::memory_order_release);
    }
private:
    std::atomic_flag m_mutex;
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