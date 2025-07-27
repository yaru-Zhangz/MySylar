#ifndef __SYLAR_ENDIAN_H__
#define __SYLAR_ENDIAN_H__

#define SYLAR_LITTLE_ENDIAN 1
#define SYLAR_BIG_ENDIAN 2


#include <byteswap.h>
#include <stdint.h>
/*
相比于系统函数，自定义函数支持uint64_t类型的
大端序：内存地址从低到高​​，数据的 ​​最高有效字节​ 存储在最低地址。（符合人类阅读习惯）
小端序：内存地址从低到高​​，数据的 ​​最低有效字节存储在最低地址。
网络字节序：一定是大端序
主机字节序：跟平台有关可能是大端序，可能是小端序

*/

namespace sylar {

template<class T>
T byteswap(T value) {
    if constexpr (sizeof(T) == sizeof(uint64_t)) {
        return static_cast<T>(bswap_64(static_cast<uint64_t>(value)));
    } else if constexpr (sizeof(T) == sizeof(uint32_t)) {
        return static_cast<T>(bswap_32(static_cast<uint32_t>(value)));
    } else if constexpr (sizeof(T) == sizeof(uint16_t)) {
        return static_cast<T>(bswap_16(static_cast<uint16_t>(value)));
    } else {
        static_assert(sizeof(T) == 0, "Unsupported type size for byteswap");
        return value; 
    }
}

#if BYTE_ORDER == BIG_ENDIAN
#define SYLAR_BYTE_ORDER SYLAR_BIG_ENDIAN
#else
#define SYLAR_BYTE_ORDER SYLAR_LITTLE_ENDIAN
#endif

#if SYLAR_BYTE_ORDER == SYLAR_BIG_ENDIAN
template<class T>
T toBigEndian(T t) { // 表示转成网络字节序，转成大端
    return t;
}
template<class T>
T toLittleEndian(T t) {   // 转成小端 
    return byteswap(t);
}
#else
template<class T>
T toBigEndian(T t) {
    return byteswap(t);
}
template<class T>
T toLittleEndian(T t) {
    return t;
}
#endif
}

#endif
