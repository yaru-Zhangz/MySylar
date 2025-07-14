# MySylar

## 1、日志模块

#### 模块介绍
用于格式化输出程序日志，方便从日志中定位程序运行过程中出现的问题。这里的日志除了日志内容本身之外，还应该包括文件名/行号，时间戳，线程/协程号，模块名称，日志级别等额外信息，甚至在打印致命的日志时，还应该附加程序的栈回溯信息，以便于分析和排查问题。
#### 相关的类
`LogFormatter`: 

`LogAppender`: 继承自ConfigVarBase，使用模板实现类型安全的配置存储，通过模板特化支持YAML与各种STL容器的相互转换

`Logger`: 提供类型转换的基础设施，默认使用boost::lexical_cast进行转换

`LogEvent`: 全局配置管理入口，负责管理全部的`ConfigVar`对象，提供：

## 2、配置模块

#### 模块介绍

基于YAML实现了一个类型安全、线程安全的配置管理系统，通过YAML格式支持多种数据类型（包括基础类型和STL容器）的序列化与反序列化，使用模板偏特化处理YAML字符串与STL容器的相互转换，同时结合读写锁保证线程安全，实现了配置变更通知机制，并提供了从YAML文件加载配置、动态修改配置以及配置项查找等功能。

#### 相关的类
`ConfigVarBase`: 作为所有配置项的基类，定义了配置项的通用接口

`ConfigVar`: 继承自ConfigVarBase，使用模板实现类型安全的配置存储，通过模板特化支持YAML与各种STL容器的相互转换

`LecicalCast`: 提供类型转换的基础设施，默认使用boost::lexical_cast进行转换

`Config`: 全局配置管理入口，负责管理全部的`ConfigVar`对象，提供：

- 配置项的查找与创建：提供`Lookup`方法，根据配置名称查询配置项，如果查询时提供了默认值和配置项的描述信息就会在未找到配置时，自动创建一个对应的配置项。
    
- 从YAML加载配置

- 线程安全的配置访问

- 配置项遍历功能
  
## 3、协程模块
进程、线程、协程的区别？

优化：原作者自定义了信号量和读写锁，该实现直接使用C++20自带的特性

#### 模块介绍
1. 协程库封装


2. 调度器
定义协程接口

ucontext_t
```
Fiber::GetThis()
Thread->main_fiber <------------> sub_fiber
            ^
            |
        sub_fiber
```
协程调度模块scheduler
```
        1 - N            1 - M
scheduler -->  thread  --> fiber
1. 线程池, 分配一组线程
2. 协程调度器，将协程指定到相应的线程上去执行

N:M
m_threads
<function<void>()> fiber, threadId m_fibers


```
优化：事件通知机制由pipe优化为eventfd

- pipe是一个单向字节流通道，由两个文件描述符组成，pipefd[0]是读端，pipefd[1]是写端，内部维护一个环形缓冲区，由于只使用pipe来唤醒idle协程并不传输实际数据，所以更适合用
- eventfd是专门用来通知时间的，它内部维护一个64位无符号整数计数器，只需要一个fd, write操作增加计数器，read读取并清零计数器，计数器是原子操作，比pipe更轻量级


#### 相关的类

## 4、IO协程调度模块
每个IOManager对象独占一个epoll fd，该epoll实例管理所有注册的fd事件，单个fd可以同时注册读和写事件。
### 定时器
- Timer类表示单个计时器，支持一次性或循环定时，可取消、刷新或重置，只能通过TimerManager创建
- TimerManager类管理所有定时器，可以添加和删除定时器，计算最近到期时间，收集过期回调，
TODO: 高性能定时器，红黑树 + 多级时间轮
Timer -> addTimer() --> cancel()
获取当前的定时器触发离现在的时间差
返回当前需要触发的时间差


```
        [Fiber]                 [Timer]
           ^ N                     ^
           |                       |
           | 1                     |
        [Thread]             [TimerManager]
           ^ M                     ^
           |                       |
           | 1                     |
      [Scheduler] <------- [IOManager(epoll)]

```

## 5、 Socket IO HOOK
hook: 拦截和修改底层socket系统调用技术
```
sleep
usleep



```




