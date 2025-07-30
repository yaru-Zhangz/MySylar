#ifndef __SYLAR_ADDRESS_H__
#define __SYLAR_ADDRESS_H__

#include <memory>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <map>


namespace sylar {

class IPAddress;

class Address {
public:
    using ptr = std::shared_ptr<Address>;

    static Address::ptr Create(const sockaddr* addr, socklen_t addrlen);            // 通过sockaddr创建地址

    // 通过host地址返回满足条件的Address, 例如：www.baidu.com[:80]
    static bool Lookup(std::vector<Address::ptr>& result, const std::string& host,  
                            int family = AF_UNSPEC, int type = 0, int protocol = 0);
    // 通过host地址返回任一满足条件的IP Address
    static Address::ptr LookupAny(const std::string& host, int family = AF_INET, 
                            int type = 0, int protocol = 0);
    // 通过host地址返回任一满足条件的IP Address
    static std::shared_ptr<IPAddress> LookupAnyIPAddress(const std::string& host,
            int family = AF_INET, int type = 0, int protocol = 0);

    // 返回本机所有网卡的<网卡名, 地址, 子网掩码位数>
    static bool GetInterfaceAddresses(std::multimap<std::string,std::pair<Address::ptr, uint32_t> >& result, int family = AF_INET);
    // 获取指定网卡的地址和子网掩码位数
    static bool GetInterfaceAddresses(std::vector<std::pair<Address::ptr, uint32_t> >&result
                    , const std::string& iface, int family = AF_INET);

    
    virtual ~Address() = default;

    int getFamily() const;      // 返回地址所属的协议族：AF_INET、AF_INET6、AF_UNIX、AF_UNSPEC

    virtual const sockaddr* getAddr() const = 0;
    virtual sockaddr* getAddr() = 0;
    virtual socklen_t getAddrLen() const = 0;

    virtual std::ostream& insert(std::ostream& os) const = 0;
    std::string toString();

    bool operator<(const Address& rhs) const;
    bool operator==(const Address& rhs) const;
    bool operator!= (const Address& rhs) const;
};

class IPAddress : public Address {
public:
    using ptr = std::shared_ptr<IPAddress>;

    static IPAddress::ptr Create(const char* address, uint16_t port = 0);

    virtual IPAddress::ptr broadcastAddress(uint32_t prefix_len) = 0;
    virtual IPAddress::ptr networkAddress(uint32_t prefix_len) = 0;
    virtual IPAddress::ptr subnetMask(uint32_t prefix_len) = 0;

    virtual uint16_t getPort() const = 0;
    virtual void setPort(uint16_t v) = 0;
};

class IPv4Address : public IPAddress {
public:
    using ptr = std::shared_ptr<IPv4Address>;
    static IPv4Address::ptr Create(const char* address, uint16_t port = 0);
    IPv4Address(const sockaddr_in& address);
    IPv4Address(uint32_t address = INADDR_ANY, uint16_t port = 0);

    sockaddr* getAddr() override;
    const sockaddr* getAddr() const override;
    socklen_t getAddrLen() const override;
    std::ostream& insert(std::ostream& os) const override;

    IPAddress::ptr broadcastAddress(uint32_t prefix_len) override;
    IPAddress::ptr networkAddress(uint32_t prefix_len) override;
    IPAddress::ptr subnetMask(uint32_t prefix_len) override;
    uint16_t getPort() const override;
    virtual void setPort(uint16_t v) override;
private:
    sockaddr_in m_addr;
};

class IPv6Address : public IPAddress {
public:
    using ptr = std::shared_ptr<IPv6Address>;

    IPv6Address();
    IPv6Address(const sockaddr_in6& address);
    IPv6Address(const uint8_t address[16], uint16_t port = 0);
    static IPv6Address::ptr Create(const char* address, uint16_t port = 0);

    sockaddr* getAddr() override;
    const sockaddr* getAddr() const override;
    socklen_t getAddrLen() const override;
    std::ostream& insert(std::ostream& os) const override;

    IPAddress::ptr broadcastAddress(uint32_t prefix_len) override;
    IPAddress::ptr networkAddress(uint32_t prefix_len) override;
    IPAddress::ptr subnetMask(uint32_t prefix_len) override;
    uint16_t getPort() const override;
    virtual void setPort(uint16_t v) override;
private:
    sockaddr_in6 m_addr;   
};

class UnixAddress : public Address {
public:
    using ptr = std::shared_ptr<UnixAddress>;

    UnixAddress();
    UnixAddress(const std::string& path);

    void setAddrLen(uint32_t v);
    sockaddr* getAddr() override;
    const sockaddr* getAddr() const override;

    socklen_t getAddrLen() const override;
    std::string getPath() const;
    std::ostream& insert(std::ostream& os) const override;
private:
    struct sockaddr_un m_addr;
    socklen_t m_length;
};

class UnknownAddress : public Address {
public:
    using ptr = std::shared_ptr<UnknownAddress>;
    UnknownAddress(int family);
    UnknownAddress(const sockaddr& addr);
    sockaddr* getAddr() override;
    const sockaddr* getAddr() const override;
    socklen_t getAddrLen() const override;
    std::ostream& insert(std::ostream& os) const override;
private:
    sockaddr m_addr;
};

std::ostream& operator<<(std::ostream& os, const Address& addr);

}

#endif