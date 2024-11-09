/**
 * @file scheduler.h
 * @brief 协程调度器封装
 */

#ifndef __LIONET_SCHEDULER_H__
#define __LIONET_SCHEDULER_H__

#include <iostream>
#include <list>
#include <memory>
#include <vector>

#include "fiber.h"
#include "thread.h"

namespace LioNet {

/**
 * @brief 协程调度器
 * @details 封装M: N的协程调度器
            内部维护线程池，支持协程在其中切换
 */
class Scheduler {
 public:
  typedef Mutex MutexType;
  typedef std::shared_ptr<Scheduler> ptr;

  /**
   * @brief 构造函数
   * @param[in] threads 线程数量
   * @param[in] use_caller 是否使用当前调用线程
   * @param[in] name 协程调度器名称
   */
  Scheduler(size_t threads = 1, bool use_caller = true,
            const std::string& name = "");

  /**
   * @brief 析构函数（虚函数，子类有不同实现）
   */
  virtual ~Scheduler();

  /**
   * @brief 返回协程调度器名称
   */
  const std::string& getName() const { return m_name; }

  /**
   * @brief 返回当前协程调度器
   */
  static Scheduler* GetThis();

  /**
   * @brief 返回当前协程调度器的调度协程
   */
  static Fiber* GetMainFiber();

  /**
   * @brief 启动协程调度器
   */
  void start();

  /**
   * @brief 停止协程调度器
   */
  void stop();

  /**
   * @brief 调度协程
   * @param[in] func 协程或者函数
   * @param[in] thread 协程执行的线程id， -1标识任意线程
   */
  template <class FiberOrFunc>
  void schedule(FiberOrFunc func, int thread = -1) {
    bool need_tickle = false;
    {
      MutexType::Lock lock(m_mutex);
      need_tickle = scheduleNonLock(func, thread);
    }
    if (need_tickle) {
      tickle();
    }
  }

  /**
   * @brief 批量调度协程
   * @param[in] begin 协程数组的开始
   * @param[in] end 协程数组的结束
   */
  template <class InputIterater>
  void schedule(InputIterater begin, InputIterater end) {
    bool need_tickle = false;
    {
      MutexType::Lock lock(m_mutex);
      while (begin != end) {
        need_tickle = scheduleNonLock(*begin, -1) || need_tickle;
        ++begin;
      }
    }

    if (need_tickle) {
      tickle();
    }
  }

  void switchTo(int thread = -1);
  std::ostream& dump(std::ostream& os);

 protected:
  /**
   * @brief 通知协程调度器有任务了
   */
  virtual void tickle();

  /**
   * @brief 协程调度函数
   */
  void run();

  /**
   * @brief 返回是否可以停止
   */
  virtual bool stopping();

  /**
   * @brief 协程无任务时可调度执行idle协程
   */
  virtual void idle();

  /**
   * @brief 设置当前的协程调度器
   */
  void setThis();

  /**
   * @brief 是否有空闲线程
   */
  bool hasIdleThreads() { return m_idleThreadCount > 0; }

 private:
  /**
   * @brief 协程调度启动（无锁）
   */
  template <class FiberOrFunc>
  bool scheduleNonLock(FiberOrFunc func, int thread) {
    bool need_tickle = m_fibers.empty();
    FiberAndThread ft(func, thread);
    if (ft.fiber || ft.func) {
      m_fibers.push_back(ft);
    }
    return need_tickle;
  }

 private:
  /**
   * @brief 协程/函数/线程组
   */
  struct FiberAndThread {
    Fiber::ptr fiber;            // 协程
    std::function<void()> func;  // 协程执行函数
    int thread;                  // 线程id

    /**
     * @brief 构造函数
     * @param[in] f 协程指针
     * @param[in] thr 线程id
     */
    FiberAndThread(Fiber::ptr f, int thr) : fiber(f), thread(thr) {}

    /**
     * @brief 构造函数
     * @param[in] f 协程执行函数
     * @param[in] thr 线程id
     */
    FiberAndThread(std::function<void()> f, int thr) : func(f), thread(thr) {}

    /**
     * @brief 构造函数
     * @param[in] f 协程执行函数指针
     * @param[in] thr 线程id
     * @post *f = nullptr
     */
    FiberAndThread(std::function<void()>* f, int thr) : thread(thr) {
      func.swap(*f);
    }

    /**
     * @brief 无参构造函数
     */
    FiberAndThread() : thread(-1) {}

    /**
     * @brief 重置数据
     */
    void reset() {
      fiber = nullptr;
      func = nullptr;
      thread = -1;
    }
  };

 private:
  MutexType m_mutex;
  std::vector<Thread::ptr> m_threads;  // 线程池
  std::list<FiberAndThread> m_fibers;  // 待执行的协程队列
  Fiber::ptr m_rootFiber;  // use_caller为true时有效，调度协程
  std::string m_name;      // 协程调度器名称

 protected:
  std::vector<int> m_threadIds;                // 协程下的线程id数组
  size_t m_threadCount = 0;                    // 线程数量
  std::atomic<size_t> m_activeThreadCount{0};  // 工作线程数量
  std::atomic<size_t> m_idleThreadCount{0};    // 空闲线程数量
  bool m_stopping;                             // 是否正在停止
  bool m_autoStop;                             // 是否主动停止
  int m_rootThread = 0;                        // 主线程id（use_caller）
};

class SchedulerSwitcher : public Noncopyable {
 public:
  SchedulerSwitcher(Scheduler* target = nullptr);
  ~SchedulerSwitcher();

 private:
  Scheduler* m_caller;
};
}  // namespace LioNet

#endif