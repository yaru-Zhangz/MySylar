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

#### 相关的类
## 4、


