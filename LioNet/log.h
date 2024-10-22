/**
 * @file  log.h
 * @brief 日志模块
*/

#ifndef __LIONET_LOG_H__
#define __LIONET_LOG_H__

#include <stdint.h>
#include <cstdint>
#include <fstream>
#include <list>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace LioNet {

class Logger;
class LoggerManager;

/**
 * @brief 日志级别
*/
class LogLevel {
 public:
  enum Level {
    UNKNOW = 0,
    DEBUG = 1,
    INFO = 2,
    WARN = 3,
    ERROR = 4,
    FATAL = 5
  };

  /**
   * @brief 将日志级别转换为文本输出 
  */
  static const char* ToString(LogLevel::Level level);

  /**
   * @brief 将文本转换为日志级别
  */
  static LogLevel::Level FromString(const std::string& str);
};

/**
 * @brief 日志事件
*/
class LogEvent {
 public:
  typedef std::shared_ptr<LogEvent> ptr;
  /**
   * @brief 构造函数
   * @param[in] logger 日志器
   * @param[in] level 日志级别
   * @param[in] file 文件名
   * @param[in] line 文件行号
   * @param[in] elapse 程序启动依赖的耗时(毫秒)
   * @param[in] thread_id 线程id
   * @param[in] fiber_id 协程id
   * @param[in] time 日志事件(秒)
   * @param[in] thread_name 线程名称
   */
  LogEvent(std::shared_ptr<Logger> logger, LogLevel::Level level,
           const char* file, int32_t line, uint32_t elapse, uint32_t thread_id,
           uint32_t fiber_id, uint64_t time, const std::string& thread_name);

  const char* getFile() const { return m_file; }

  int32_t getLine() const { return m_line; }

  int32_t getElapse() const { return m_elapse; }

  int32_t getThreadId() const { return m_threadId; }

  uint32_t getFiberId() const { return m_fiberId; }

  uint64_t getTime() const { return m_time; }

  const std::string& getThreadName() const { return m_threadName; }

  std::string getContent() const { return m_ss.str(); }

  std::shared_ptr<Logger> getLogger() const { return m_logger; }

  LogLevel::Level getLevel() const { return m_level; }

  std::stringstream& getSS() { return m_ss; }

  void format(const char* fmt, ...);
  void format(const char* fmt, va_list al);

 private:
  const char* m_file = nullptr;      // 文件名
  int32_t m_line = 0;                // 行号
  uint32_t m_elapse = 0;             // 程序启动开始到现在的毫秒数
  uint32_t m_threadId = 0;           // 线程id
  uint32_t m_fiberId = 0;            // 协程id
  uint64_t m_time = 0;               // 时间戳
  std::stringstream m_ss;            // 日志内容流
  std::string m_threadName;          // 线程名
  std::shared_ptr<Logger> m_logger;  // 日志器
  LogLevel::Level m_level;           // 日志等级
};

/**
 * @brief 日志事件包装器
*/
class LogEventWrap {
 public:
  LogEventWrap(LogEvent::ptr e);
  ~LogEventWrap();

  LogEvent::ptr getEvent() const { return m_event; }
  std::stringstream& getSS();

 private:
  LogEvent::ptr m_event;
};

class LogFormatter {
 public:
  typedef std::shared_ptr<LogFormatter> ptr;
  /**
  * @brief 构造函数
  * @param[in] pattern 格式模板
  * @details 
  *  %m 消息
  *  %p 日志级别
  *  %r 累计毫秒数
  *  %c 日志名称
  *  %t 线程id
  *  %n 换行
  *  %d 时间
  *  %f 文件名
  *  %l 行号
  *  %T 制表符
  *  %F 协程id
  *  %N 线程名称
  *
  *  默认格式 "%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"
  */
  LogFormatter(const std::string& pattern);

  std::string format(std::shared_ptr<Logger> looger, LogLevel::Level level,
                     LogEvent::ptr event);
  std::ostream& format(std::ostream& os, std::shared_ptr<Logger> logger,
                       LogLevel::Level level, LogEvent::ptr event);

 public:
  /**
   * @brief 日志内容格式化
  */
  class FormatItem {
   public:
    typedef std::shared_ptr<FormatItem> ptr;
    virtual ~FormatItem() {}

    virtual void format(std::ostream& os, std::shared_ptr<Logger> logger,
                        LogLevel::Level level, LogEvent::ptr event) = 0;
  };

  /**
   * @brief 初始化日志解析模板
  */
  void init();

  bool isError() const { return m_error; }

  const std::string getPattern() const { return m_pattern; }

 private:
  std::string m_pattern;                 // 日志格式化模板
  std::vector<FormatItem::ptr> m_items;  // 日志解析后格式
  bool m_error = false;                  // 是否有错误
};

/**
 * @brief 日志输出目标
*/
class LogAppender {
  friend class Logger;

 public:
  typedef std::shared_ptr<LogAppender> ptr;
  virtual ~LogAppender() {}

  virtual void log(std::shared_ptr<Logger> Logger, LogLevel::Level level,
                   LogEvent::ptr event);

  void setFormatter(LogFormatter::ptr formatter);
  LogFormatter::ptr getFormatter() const { return m_formatter; }

  void setLevel(LogLevel::Level level) { m_level = level; }
  LogLevel::Level getLevel() const { return m_level; }

 protected:
  LogLevel::Level m_level = LogLevel::DEBUG;  // 日志级别
  LogFormatter::ptr m_formatter;              // 日志格式化器
  bool m_hasFormatter = false;  // 日否有自己的日志格式器
};

/**
 * @brief 日志器
*/
class Logger : public std::enable_shared_from_this<Logger> {
  friend class LoggerManager;

 public:
  typedef std::shared_ptr<Logger> ptr;

  Logger(const std::string& name = "root");

  /**
   * @brief 写日志
  */
  void log(LogLevel::Level level, const LogEvent::ptr event);
  void debug(LogEvent::ptr event);
  void info(LogEvent::ptr event);
  void warn(LogEvent::ptr event);
  void error(LogEvent::ptr event);
  void fatal(LogEvent::ptr event);

  /**
   * @brief 处理日志输出目标
  */
  void addAppender(LogAppender::ptr appender);
  void delAppender(LogAppender::ptr appender);
  void clearAppenders();

  LogLevel::Level getLevel() const { return m_level; }
  void setLevel(LogLevel::Level val) { m_level = val; }

  const std::string& getName() const { return m_name; }

  /**
   * @brief 处理日志格式化器
  */
  void setFormatter(LogFormatter::ptr val);
  void setFormatter(std::string& val);
  LogFormatter::ptr getFormatter();

 private:
  std::string m_name;                       // 日志名称
  LogLevel::Level m_level;                  // 其中级别
  std::list<LogAppender::ptr> m_appenders;  // 日志输出目标集合
  LogFormatter::ptr m_formatter;            // 日志格式化器
  Logger::ptr root;                         // z主日志器
};

/**
 * @brief 输出到控制台的Appender
*/
class StdoutLogAppender : public LogAppender {
 public:
  typedef std::shared_ptr<StdoutLogAppender> ptr;

  void log(std::shared_ptr<Logger> logger, LogLevel::Level level,
           LogEvent::ptr event) override;
};

/**
 * @brief 输出到文件的Appender
*/
class FileLogAppender : public LogAppender {
 public:
  typedef std::shared_ptr<FileLogAppender> ptr;

  FileLogAppender(const std::string& filename);
  void log(std::shared_ptr<Logger> logger, LogLevel::Level level,
           LogEvent::ptr event) override;
  bool reopen();

 private:
  std::string m_filename;      // 文件路径
  std::ofstream m_filestream;  // 文件输出流
  uint64_t m_lastTime = 0;     // 上次重新打开时间
};

/**
 * @brief 日志器管理类
*/
class LoggerManager {
 public:
  LoggerManager();

  Logger::ptr getLogger();

  void init();

  Logger::ptr getRoot() const { return m_root; }

 private:
  std::map<std::string, Logger::ptr> m_loggers;  // 日志器容器
  Logger::ptr m_root;                            // 主日志器
};

}  // namespace LioNet

#endif