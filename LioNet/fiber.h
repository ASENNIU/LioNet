/**
 * @file fiber.h
 * @brief 协程封装
 */

#ifndef __LIONET_FIBER_H__
#define __LIONET_FIBER_H__

#include <ucontext.h>
#include <functional>
#include <memory>

namespace LioNet {

class Scheduler;

/**
 * @brief 协程类
 */
class Fiber : public std::enable_shared_from_this<Fiber> {
 public:
  typedef std::shared_ptr<Fiber> ptr;

  /**
   * @brief 协程状态
   */
  enum State {
    INIT,   // 初始化
    HOLD,   // 暂停
    EXEC,   // 执行中
    TERM,   // 结束状态
    READY,  // 可执行
    EXCEPT  // 异常
  };

 private:
  /**
   * @brief 无参构造函数
   * @brief 每个线程第一个协程的构造
   */
  Fiber();

 public:
  /**
   * @brief 构造函数
   * @param[in] func 协程执行函数
   * @param[in] stacksize 协程栈大小
   * @param[in] use_caller 是否在MainFiber上调度
   */
  Fiber(std::function<void()> func, size_t stacksize = 0,
        bool use_caller = false);

  ~Fiber();

  /**
   * @brief 重置协程执行函数，并设置状态
   * @pre getState() 为INIT, TERM, EXCEPT
   * @post getState() = INIT
   */
  void reset(std::function<void()> func);

  /**
   * @brief 将当前协程切换到运行状态
   * @pre getState() != EXEC
   * @post getSatte() = INIT
   */
  void swapIn();

  /**
   * @brief 将当前协程切换到后台
   */
  void swapOut();

  /**
   * @brief 将当前协程切换到执行状态
   * @pre 执行的为当前线程的主协程
   */
  void call();

  /**
   * @brief 将当前协程切换到后台
   * @pre 执行的为该协程
   * @post 返回到当前线程的主协程
   */
  void back();

  /**
   * @brief 返回协程的id
   */
  uint64_t getId() const { return m_id; }

  /**
   * @brief 返回协程状态
   */
  State getState() const { return m_state; }

 public:
  /**
   * @brief 设置当前前程的运行协程
   * @param[in] f 运行协程
   */
  static void SetThis(Fiber* f);

  /**
   * @brief 返回当前所在协程
   */
  static Fiber::ptr GetThis();

  /**
   * @brief 将当前协程切换到后台，其设置为READY状态
   * @post getState() = READY
   */
  static void YieldToReady();

  /**
   * @brief 将当前协程切换到后台，其设置为HOLD状态
   * @post getState() = HOLD
   */
  static void YieldToHold();

  /**
   * @brief 返回当前协程总数
   */
  static uint64_t TotalFibers();

  /**
   * @brief 协程执行函数
   * @post 执行完成返回到线程主协程
   */
  static void MainFunc();

  /**
   * @brief 协程执行函数
   * @post 执行完成返回线程调度协程
   */
  static void CallerMainFunc();

  /**
   * @brief 获取协程id
   */
  static uint64_t GetFiberId();

 private:
  uint64_t m_id;                 // 协程id
  uint32_t m_stacksize;          // 协程运行栈大小
  State m_state;                 // 协程状态
  ucontext_t m_ctx;              // 协程上下文
  void* m_stack;                 // 协程运行栈指针
  std::function<void()> m_func;  // 协程运行函数
};
}  // namespace LioNet

#endif