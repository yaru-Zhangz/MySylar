#include<iostream>
#include<thread>
#include "../sylar/include/log.h"
#include "../sylar/include/util.h"
int main(int argc, char** argv) {
    sylar::Logger::ptr logger(new sylar::Logger);
    logger->addAppender(sylar::LogAppender::ptr(new sylar::StdoutLogAppender));
    sylar::FileLogAppender::ptr file_appender(new sylar::FileLogAppender("./log.txt"));
    logger->addAppender(file_appender);

    SYLAR_LOG_INFO(logger) << "test macro";
    SYLAR_LOG_ERROR(logger) << "test macro error";
    SYLAR_LOG_FATAL(logger) << "test macro fatal";

    SYLAR_LOG_FMT_ERROR(logger, "test macro fmt error %s", "aa");
    auto l = sylar::LoggerMgr::GetInstance()->getLogger("xx");

    SYLAR_LOG_INFO(SYLAR_LOG_NAME("dragonborn")) << "hello";
    SYLAR_LOG_INFO(l) << "xxxx";
    return 0;

/*
1、创建一个根日志输出器，默认日志级别为DEBUG, new LogFormatter用默认格式初始化并调用init解析
2、添加一个控制台输出地，将logger的formatter设置给appender
3、创建日志事件
4、检查日志级别是否大于等于m_level，若低于设置级别则丢弃
5、调用StdoutLogAppender的log方法，调用具体的appender的log方法，在log方法里调用formatter的format方法 -> 调用每一个item的format方法，
将结果输出到stdout/file
*/
}