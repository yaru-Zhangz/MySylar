#include "../sylar/include/sylar.h"
#include "../sylar/include/iomanager.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>

sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

int sock = 0;

void test_fiber() {
    SYLAR_LOG_INFO(g_logger) << "test_fiber sock=" << sock;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    /*
        fcntl()用于对已打开的文件描述符进行各种控制操作.
        F_DUPFD: 查找大于或等于制定证书的最小可用文件描述符，并将其复制为fd的一个副本
        F_SETFL: 设置文件状态标志
        O_NONBLOCK: 设置文件状态标志位“非阻塞”
    */
    fcntl(sock, F_SETFL, O_NONBLOCK);   // 

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET, "115.239.210.27", &addr.sin_addr.s_addr);

    // 在非阻塞模式下，connect()会立即返回，如果连接可以建立返回0，大多数情况下返回-1并设置errno为EINPROGRESS
    if(!connect(sock, (const sockaddr*)&addr, sizeof(addr))) {
    } else if(errno == EINPROGRESS) {   // 表示操作已开始但尚未完成
        SYLAR_LOG_INFO(g_logger) << "add event errno=" << errno << " " << strerror(errno);
        // 添加读回调
        sylar::IOManager::GetThis()->addEvent(sock, sylar::IOManager::READ, [](){
            SYLAR_LOG_INFO(g_logger) << "read callback";
            sylar::IOManager::GetThis()->cancelEvent(sock, sylar::IOManager::READ);
            sylar::IOManager::GetThis()->cancelEvent(sock, sylar::IOManager::WRITE);
            close(sock);
        });
        // 添加
        sylar::IOManager::GetThis()->addEvent(sock, sylar::IOManager::WRITE, [](){
            SYLAR_LOG_INFO(g_logger) << "write callback"; 
            sylar::IOManager::GetThis()->cancelAll(sock);
            close(sock);
        });
    } else {
        SYLAR_LOG_INFO(g_logger) << "else " << errno << " " << strerror(errno);
        close(sock);
    }
}

void test1() {
    std::cout << "EPOLLIN=" << EPOLLIN
              << " EPOLLOUT=" << EPOLLOUT << std::endl;
    sylar::IOManager iom(2, false);
    iom.schedule(&test_fiber);
}
sylar::Timer::ptr s_timer;
void test_timer() {
    sylar::IOManager iom(2);
    s_timer = iom.addTimer(1000, [](){
        SYLAR_LOG_INFO(g_logger) << "hello timer";
        static int i = 0;
        if(++i == 3) {
            s_timer->reset(2000, true); // 执行3次后重置定时器为2s间隔
            // s_timer->cancel();
        }
    }, true);
}
int main(int argc, char** argv) {
    
    // test1();
    test_timer();
    return 0;
}

