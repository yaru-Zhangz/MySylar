#include "../include/thread.h"
#include "../include/log.h"
#include "../include/util.h"

namespace sylar {
    static thread_local Thread* t_thread = nullptr; // 指向当前线程对象的指针，每个线程有自己的副本
    static thread_local std::string t_thread_name = "UNKNOWN";  // 当前线程的名称，

    static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

    Thread* Thread::GetThis() {
        return t_thread;
    }

    const std::string Thread::GetName() {
        return t_thread_name;
    }

    void Thread::SetName (const std::string& name) {
        if(name.empty()) {
            return;
        }
        if(t_thread) {
            t_thread->m_name = name;
        }
        t_thread_name = name;
    }

    Thread::Thread(std::function<void()> cb, const std::string& name)
    : m_cb(cb), m_name(name) {
        if(name.empty()) {
            m_name = "UNKNOWN";
        }
        int rt = pthread_create(&m_thread, nullptr, &Thread::run, this); // On success, pthread_create() returns 0; 
        // pthread create error， print log
        if(rt) {
            SYLAR_LOG_ERROR(g_logger) << "pthread_create thread fail, rt = " << rt
                                        << " name=" << name;
            throw std::logic_error("pthread_create error");
        }
        m_sem.acquire(); 
    }

    Thread::~Thread() {
        if(m_thread) {
            pthread_detach(m_thread); // 如果线程仍在运行将其分离
        }
    }

    // 线程入口函数
    void* Thread::run(void* arg) {
        Thread* thread = (Thread*) arg;
        t_thread = thread;
        t_thread_name = thread->m_name;
        thread->m_id = sylar::GetThreadId();
        pthread_setname_np(pthread_self(), thread->m_name.substr(0,15).c_str());

        std::function<void()> cb;
        cb.swap(thread->m_cb);  // 将线程要执行的回调移动到局部变量中，将thread->m_cb置空

        thread->m_sem.release(); // 确保线程正确初始化后再执行回调
        cb(); 
        return 0;
    }

    void  Thread::join() {
        if(m_thread) {
            int rt = pthread_join(m_thread, nullptr);
            // error
            if(rt) {
                SYLAR_LOG_ERROR(g_logger) << "pthread_join thread fail, rt=" << rt
                    << " name=" << m_name;
                throw std::logic_error("pthread_join error");
            }
            m_thread = 0;
        }
    }

}