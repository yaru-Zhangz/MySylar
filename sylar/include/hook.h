#ifndef __SYLAR_HOOK_H__
#define __SYLAR__HOOK_H__

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>

namespace sylar {
    bool is_hook_enable();              // 检查当前线程是否启用hook
    void set_hook_enable(bool flag);    // 设置hook状态

}

// 定义函数指针类型并保存原始函数指针
extern "C" {
    // sleep
    typedef unsigned int (*sleep_fun)(unsigned int seconds);
    extern sleep_fun sleep_f;           // 使进程挂起指定的秒数

    typedef int (*usleep_fun)(useconds_t usec);
    extern usleep_fun usleep_f;         // 使进程挂起指定的微秒数

    typedef int (*nanosleep_fun)(const struct timespec *req, struct timespec *rem);
    extern nanosleep_fun nanosleep_f;   // 使进程挂起指定的纳秒数

    // socket
    typedef int (*socket_fun)(int domain, int type, int protocol);
    extern socket_fun socket_f;         // 创建socket

    typedef int (*connect_fun)(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
    extern connect_fun connect_f;       // 客户端建立连接

    typedef int (*accept_fun)(int s, struct sockaddr *addr, socklen_t *addrlen);
    extern accept_fun accept_f;         // 服务端接受连接

    // read
    typedef ssize_t (*read_fun)(int fd, void *buf, size_t count);
    extern read_fun read_f;             // 从任何文件描述符读取

    typedef ssize_t (*readv_fun)(int fd, const struct iovec *iov, int iovcnt);
    extern readv_fun readv_f;           // 分散读，可以一次读到多个缓冲区

    typedef ssize_t (*recv_fun)(int sockfd, void *buf, size_t len, int flags);
    extern recv_fun recv_f;             // 专用于sockfd的读取

    typedef ssize_t (*recvfrom_fun)(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen);
    extern recvfrom_fun recvfrom_f;     // 接收数据并获取发送方地址，主要用于UDP

    typedef ssize_t (*recvmsg_fun)(int sockfd, struct msghdr *msg, int flags);
    extern recvmsg_fun recvmsg_f;       // 支持多缓冲区，可获取辅助数据，可获取发送方地址

    // write
    typedef ssize_t (*write_fun)(int fd, const void *buf, size_t count);
    extern write_fun write_f;           // 向任何文件描述符写入

    typedef ssize_t (*writev_fun)(int fd, const struct iovec *iov, int iovcnt);
    extern writev_fun writev_f;         // 聚集写，可以一次写多个缓冲区

    typedef ssize_t (*send_fun)(int s, const void *msg, size_t len, int flags);
    extern send_fun send_f;             // 专用于socket的发送

    typedef ssize_t (*sendto_fun)(int s, const void *msg, size_t len, int flags, const struct sockaddr *to, socklen_t tolen);
    extern sendto_fun sendto_f;         // 发送数据到指定地址，主要用于UDP

    typedef ssize_t (*sendmsg_fun)(int s, const struct msghdr *msg, int flags);
    extern sendmsg_fun sendmsg_f;       // 支持多缓冲区，可发送辅助数据，可指定目标地址

    // close
    typedef int (*close_fun)(int fd);
    extern close_fun close_f;           // 关闭文件描述符

    // socket operation
    typedef int (*fcntl_fun)(int fd, int cmd, ... /* arg */ );
    extern fcntl_fun fcntl_f;           // 控制文件描述符

    typedef int (*ioctl_fun)(int d, unsigned long int request, ...);
    extern ioctl_fun ioctl_f;           // 用于设备控制

    typedef int (*getsockopt_fun)(int sockfd, int level, int optname, void *optval, socklen_t *optlen);
    extern getsockopt_fun getsockopt_f; // 获取socket选项

    typedef int (*setsockopt_fun)(int sockfd, int level, int optname, const void *optval, socklen_t optlen);
    extern setsockopt_fun setsockopt_f; // 设置socket选项

    // 带超时的connect
    extern int connect_with_timeout(int fd, const struct sockaddr* addr, socklen_t addrlen, uint64_t timeout_ms);

}

#endif