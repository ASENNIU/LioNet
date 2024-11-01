/**
 * @file thread.h
 * @brief 线程相关的封装
 */

#ifndef __LIONET_THREAD_H__
#define __LIONET_THREAD_H__

#include <functional>
#include <list>
#include <memory>
#include <string>
#include "mutex.h"

namespace LioNet {

/**
 * @brief 线程类
 */
class Thread : Noncopyable {
 public:
  typedef std::shared_ptr<Thread> ptr;

  /**
   * @brief 构造函数
   * @param[in] func func 线程执行函数
   * @param[in] name 线程名称
   */
  Thread(std::function<void()> func, const std::string& name);

  /**
   * @brief 析构函数
   */
  ~Thread();

  /**
   * @brief 线程ID
   */
  pid_t getId() const { return m_id; }

  /**
   * @brief 线程名称
   */
  const std::string& getName() const { return m_name; }

  /**
   * @brief 等待线程执行完成
   */
  void join();

  /**
   * @brief 获取当前的线程指针
   */
  static Thread* GetThis();

  /**
   * @brief 获取当前的线程名称
   */
  static const std::string& GetName();

  /**
   * @brief 设置当前线程名称
   * @param[in] name 线程名称
   */
  static void SetName(const std::string& name);

 private:
  /**
   * @brief 线程执行函数
   */
  static void* run(void* arg);

 private:
  pid_t m_id = -1;               // 线程id
  pthread_t m_thread = 0;        // 线程结构
  std::function<void()> m_func;  // 线程执行函数
  std::string m_name;            // 线程名称
  Semaphore m_semaphore;         // 信号量，同步线程的启动过程
};

}  // namespace LioNet

#endif