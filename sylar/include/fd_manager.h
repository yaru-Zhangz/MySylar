#ifndef __FD_MANAGER_H__
#define __FD_MANAGER_H__

#include <memory>
#include <vector>
#include <shared_mutex>
#include <mutex>
#include "thread.h"
#include "singleton.h"

namespace sylar {
/*
文件描述符上下文类：封装单个文件描述符的状态信息，
*/
class FdCtx : public std::enable_shared_from_this<FdCtx> {
public:
    using ptr = std::shared_ptr<FdCtx>;
    FdCtx(int fd);
    ~FdCtx();

    bool isInit() const { return m_isInit;}                
    bool isSocket() const { return m_isSocket;}
    bool isClose() const { return m_isClosed;}

    void setUserNonblock(bool v) { m_userNonblock = v;}
    bool getUserNonblock() const { return m_userNonblock;}

    void setSysNonblock(bool v) { m_sysNonblock = v;}
    bool getSysNonblock() const { return m_sysNonblock;}

    void setTimeout(int type, uint64_t v);  // 设置超时时间
    uint64_t getTimeout(int type);          // 获取超时时间

private:
    bool init();
private:
    // :1指定bool类型只占1bit
    bool m_isInit: 1;        // 是否初始化
    bool m_isSocket: 1;      // 是否是socket类型，区分是socket还是文件类型
    bool m_sysNonblock: 1;   // 是否hook非阻塞
    bool m_userNonblock: 1;  // 是否用户主动设置非阻塞
    bool m_isClosed: 1;      // 是否关闭
    int m_fd;                // 文件句柄
    uint64_t m_recvTimeout;  // 读超时时间毫秒
    uint64_t m_sendTimeout;  // 写超时时间毫秒
};

/*
文件描述符管理类：管理所有的FdCtx对象的生命周期
*/
class FdManager {
public:
    FdManager();

    FdCtx::ptr get(int fd, bool auto_create = false);   // 获取FD上下文
    void del(int fd);
private:
    std::shared_mutex m_mutex;
    std::vector<FdCtx::ptr> m_datas;
};

// 文件句柄单例
using FdMgr = Singleton<FdManager>;

}

#endif
