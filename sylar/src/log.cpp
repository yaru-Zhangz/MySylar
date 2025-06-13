#include<map>
#include<iostream>
#include<functional>
#include<time.h>
#include<string.h>
#include<regex>

#include"config.h"
#include "log.h"

namespace sylar {

const char* LogLevel::ToString(LogLevel::Level level) {
    switch(level) {
#define XX(name) \
    case LogLevel::name: \
        return #name; \
        break;
        XX(DEBUG);
        XX(INFO);
        XX(WARN);
        XX(ERROR);
        XX(FATAL);
    #undef XX
        default:
            return "UNKNOWN";
    }
    return "UNKNOWN";
}
LogEventWrap::LogEventWrap(LogEvent::ptr e)
    :m_event(e) {

}

// 在析构时触发日志的最终提交
LogEventWrap::~LogEventWrap() {
    m_event->getLogger()->log(m_event->getLevel(), m_event);
}

void LogEvent::format(const char* fmt, ...) {
    va_list al;         // 定义可变参数列表指针
    va_start(al, fmt);  // 初始化al,指向fmt后的第一个可变参数
    format(fmt, al);    // 调用重载函数处理实际格式化
    va_end(al);         // 清理va_list
}

void LogEvent::format(const char* fmt, va_list al) {
    char* buf = nullptr;
    int len = vasprintf(&buf, fmt, al);
    if(len != -1) {
        m_ss << std::string(buf, len); // 将 buf 指向的内存中前 len 个字符拷贝到新构造的 std::string 对象中。
        free(buf);
    }
}

std::stringstream& LogEventWrap::getSS() {
    return m_event->getSS();
}

void LogAppender::setFormatter(LogFormatter::ptr val) {
    std::lock_guard<std::mutex> guard(m_mutex);
    m_formatter = val;
    if(m_formatter) {
        m_hasFormatter = true;
    } else {
        m_hasFormatter = false;
    }
}

LogFormatter::ptr LogAppender::getFormatter() {
    std::lock_guard<std::mutex> guard(m_mutex);
    return m_formatter;
}

class MessageFormatItem : public LogFormatter::FormatItem {
public:
    MessageFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getContent();
    }
};
    
class LevelFormatItem : public LogFormatter::FormatItem {
public:
    LevelFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << LogLevel::ToString(level);
    }
};

class ElapseFormatItem : public LogFormatter::FormatItem {
public:
    ElapseFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getElapse();
    }
};

class NameFormatItem : public LogFormatter::FormatItem {
public:
    NameFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getLogger()->getName();
    }
};

class ThreadIdFormatItem : public LogFormatter::FormatItem {
public:
    ThreadIdFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getThreadId();
    }
};

class FiberIdFormatItem : public LogFormatter::FormatItem {
public:
    FiberIdFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getFiberId();
    }
};

class DateTimeFormatItem : public LogFormatter::FormatItem {
public:
    DateTimeFormatItem(const std::string& format = "%Y-%m-%d %H:%M:%S")
        :m_format(format) {
        if(m_format.empty()) {
            m_format = "%Y-%m-%d %H:%M:%S";
        }
        }
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        struct tm tm;
        time_t time = event->getTime();
        localtime_r(&time, &tm);
        char buf[64];
        strftime(buf, sizeof(buf), m_format.c_str(), &tm);
        os << buf;
    }
private:
    std::string m_format;
};

class FilenameFormatItem : public LogFormatter::FormatItem {
public:
    FilenameFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getFile();
    }
};

class LineFormatItem : public LogFormatter::FormatItem {
public:
    LineFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getLine();
    }
};

class NewLineFormatItem : public LogFormatter::FormatItem {
public:
    NewLineFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << std::endl;
    }
};

class TabFormatItem : public LogFormatter::FormatItem {
public:
    TabFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << "\t";
    }
private:
    std::string m_string;
};

class StringFormatItem : public LogFormatter::FormatItem {
public:
    StringFormatItem(const std::string& str)
        : m_string(str) {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
        os << m_string;
    }
private:
    std::string m_string;
};

LogEvent::LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level
        , const char* file, uint32_t line, uint32_t elapse
        , uint32_t thread_id, uint32_t fiber_id, uint64_t time)
    : m_file(file)
    , m_line(line)
    , m_elapse(elapse)
    , m_threadId(thread_id)
    , m_fiberId(fiber_id)
    , m_time(time)
    , m_logger(logger)
    , m_level(level) { 
}

Logger::Logger(const std::string& name)
    :m_name(name)
    , m_level(LogLevel::DEBUG) {
        m_formatter.reset(new LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%F%T[%p]%T[%c]%T%T%f:%l%T%m%n")); // 默认格式
}

// void Logger::setFormatter(LogFormatter::ptr val) {
//     m_formatter = val;

//     for(auto& i : m_appenders) {
//         if(!i->m_hasFormatter) {
//             i->m_formatter = m_formatter;
//         }
//     }
// }

// void Logger::setFormatter(const std::string& val) {

// }
// LogFormatter::ptr getFormatter() {

// }

void Logger::addAppender(LogAppender::ptr appender) {
    std::lock_guard<std::mutex> guard(m_mutex);
    if(!appender->getFormatter()) {
        appender->setFormatter(m_formatter);
    }
    m_appenders.push_back(appender);
}

void Logger::delAppender(LogAppender::ptr appender) {
    std::lock_guard<std::mutex> guard(m_mutex);
    for(auto it = m_appenders.begin(); it != m_appenders.end(); it++)
    {
        std::lock_guard<std::mutex> gua(appender->m_mutex);
        if(*it == appender) {
            m_appenders.erase(it);
            break;
        }
    }
}

void Logger::log(LogLevel::Level level, LogEvent::ptr event) {
    if(level >= m_level) {
        auto self = shared_from_this();
        std::lock_guard<std::mutex> guard(m_mutex);
        if(!m_appenders.empty()) {
            for(auto& i : m_appenders) {
                i->log(self, level, event);
            }
        } else if(m_root) {
            m_root->log(level, event);
        }
    }
}

void Logger::debug(LogEvent::ptr event) {
    log(LogLevel::DEBUG, event);
}
void Logger::info(LogEvent::ptr event) {
    log(LogLevel::INFO, event);
}
void Logger::warn(LogEvent::ptr event) {
    log(LogLevel::WARN, event);
}
void Logger::error(LogEvent::ptr event) {
    log(LogLevel::ERROR, event);
}
void Logger::fatal(LogEvent::ptr event) {
    log(LogLevel::FATAL, event);
}

FileLogAppender::FileLogAppender(const std::string& filename) 
    :m_filename(filename) {
    reopen();
}
void FileLogAppender::log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) {
    if(level >= m_level) {
        std::lock_guard<std::mutex> guard(m_mutex);
        m_filestream << m_formatter->format(logger, level, event);
    }
}

bool FileLogAppender::reopen() {
    std::lock_guard<std::mutex> guard(m_mutex);
    if(m_filestream) {
        m_filestream.close();
    }
    m_filestream.open(m_filename);
    return !!m_filestream;
}

void StdoutLogAppender::log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) {
    if(level >= m_level) {
        std::lock_guard<std::mutex> guard(m_mutex);
        m_formatter->format(std::cout, logger, level, event);
    }
}

LogFormatter::LogFormatter(const std::string& pattern) 
    :m_pattern(pattern) {
        init();
}

std::string LogFormatter::format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) {
    std::stringstream ss;
    for(auto& i : m_items) {
        i->format(ss, logger, level, event);
    }
    return ss.str();
}

std::ostream& LogFormatter::format(std::ostream& ofs, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) {
    for(auto& i : m_items) {
        i->format(ofs, logger, level, event);
    }
    return ofs;
}

void LogFormatter::init() {

    // 定义正则表达式：匹配 %xxx{...} 或 %xxx
    static const std::regex pattern_regex(R"(%([a-zA-Z%])(?:\{(.*?)\})?)");
    std::vector<std::tuple<std::string, std::string, int>> vec;  // <文本/占位符名称, 格式, 类型：0 = 普通文本，1 = 占位符>
    std::string::const_iterator start = m_pattern.begin();
    std::string::const_iterator end = m_pattern.end();
    std::smatch matches;
    std::string nstr; // 缓存普通文本

    // 遍历所有匹配项
    while (std::regex_search(start, end, matches, pattern_regex)) {
        // 处理匹配前的普通文本
        if (matches.prefix().length() > 0) {
            vec.emplace_back(matches.prefix().str(), "", 0);
        }

        // 处理占位符
        std::string placeholder = matches[1].str(); // 占位符名称
        std::string format = matches[2].str();      // 格式块内容（如 %%Y-%%m-%%d）, 没有为空串
        vec.emplace_back(placeholder, format, 1);

        start = matches[0].second; // 继续匹配剩余部分
    }

    // 处理末尾的普通文本
    if (start != end) {
        vec.emplace_back(std::string(start, end), "", 0);
    }

    static std::map<std::string, std::function<FormatItem::ptr(const std::string& str)>> s_format_items = {
#define XX(str, C) \
            {#str, [](const std::string& fmt) { return FormatItem::ptr(new C(fmt));}}
    XX(m, MessageFormatItem),
    XX(p, LevelFormatItem),
    XX(r, ElapseFormatItem),
    XX(c, NameFormatItem),
    XX(t, ThreadIdFormatItem),
    XX(n, NewLineFormatItem),
    XX(d, DateTimeFormatItem),
    XX(f, FilenameFormatItem),
    XX(l, LineFormatItem),
    XX(T, TabFormatItem),
    XX(F, FiberIdFormatItem),
#undef XX
    };

    for(auto& i : vec) {
        if(std::get<2>(i) == 0) {   // 普通文本
            m_items.push_back(FormatItem::ptr(new StringFormatItem(std::get<0>(i))));
        } else {    // 占位符
            auto it = s_format_items.find(std::get<0>(i));
            if(it == s_format_items.end()) {
                m_items.push_back(FormatItem::ptr(new StringFormatItem("<<error_format %" + std::get<0>(i) + ">>")));
            } else {
                m_items.push_back(it->second(std::get<1>(i)));
            }
        }

        // std::cout << "(" << std::get<0>(i) << ") - (" << std::get<1>(i) << ") - (" << std::get<2>(i) << ")" << std::endl;
    }
    // std::cout << m_items.size() << std::endl;
}
// %m -- 消息体
// %p -- 日志级别
// %r -- 启动后的时间
// %c -- 日志名称
// %t -- 线程id
// %F -- 协程id
// %n -- 回车换行
// %d -- 时间
// %f -- 文件名
// %l -- 行号
LoggerManager::LoggerManager() {
    m_root.reset(new Logger);
    m_root->addAppender(LogAppender::ptr(new StdoutLogAppender));

    m_loggers[m_root->m_name] = m_root;
    init();
}
Logger::ptr LoggerManager::getLogger(const std::string& name) {
    std::lock_guard<std::mutex> guard(m_mutex);
    auto it = m_loggers.find(name);
    if(it != m_loggers.end()) return it->second;
    Logger::ptr logger(new Logger(name));
    logger->m_root = m_root;
    m_loggers[name] = logger;
    return logger;
}
struct LogAppenderDefine {
    int type = 0;
    LogLevel::Level level = LogLevel::UNKNOWN;
    std::string formatter;
    std::string file;
    bool operator==(const LogAppenderDefine& other) const {
        return type == other.type && level == other.level && formatter == other.formatter && file == other.file;
    }
};

struct LogDefine {
    std::string name;
    LogLevel::Level level = LogLevel::UNKNOWN;
    std::string formatter;
    std::vector<LogAppenderDefine> appenders;

    bool operator==(const LogDefine& other) const {
        return name == other.name && level == other.level && formatter == other.formatter && appenders == other.appenders;
    }

    bool operator<(const LogDefine& other) const {
        return name < other.name;
    } 
};

void LoggerManager::init() {
}

}