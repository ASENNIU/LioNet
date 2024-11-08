#include "fiber.h"
#include <atomic>
#include "config.h"
#include "log.h"
#include "macro.h"
#include "scheduler.h"

namespace LioNet {

static Logger::ptr g_logger = LIONET_LOG_NAME("system");

static std::atomic<uint64_t> s_fiber_id{0};
static std::atomic<uint64_t> s_fiber_count{0};

// 外部并不会直接访问这些静态变量，等价于已经用函数封装了，不会出现静态变量初始化顺序问题
// std::atomic<uint64_t>& fetchFiberId() {
//   static std::atomic<uint64_t> s_fiber_id{0};
//   return s_fiber_id;
// }

// std::atomic<uint64_t>& fetchFiberCount() {
//   static std::atomic<uint64_t> s_fiber_count{0};
//   return s_fiber_count;
// }

static thread_local Fiber* t_fiber = nullptr;            // 指向当前协程
static thread_local Fiber::ptr t_threadFiber = nullptr;  // 指向主协程

static ConfigVar<uint32_t>::ptr g_fiber_stack_size = Config::Lookup<uint32_t>(
    "fiber.stack_size", 128 * 1024, "fiber stack size");

class MallocStackAllocator {
 public:
  static void* Alloc(size_t size) { return malloc(size); }

  static void Dealloc(void* vp, size_t size) {
    free(vp);
    return;
  }
};

using StackAllocator = MallocStackAllocator;

uint64_t Fiber::GetFiberId() {
  if (t_fiber) {
    return t_fiber->getId();
  }
  return 0;
}

Fiber::Fiber() {
  m_state = EXEC;
  SetThis(this);

  if (getcontext(&m_ctx)) {
    LIONET_ASSERT2(false, "getcontext");
  }

  m_id = ++s_fiber_id;
  ++s_fiber_count;

  LIONET_DEBUG(g_logger) << "Fiber::Fiber Main";
}

Fiber::Fiber(std::function<void()> func, size_t stacksize, bool use_caller)
    : m_id(++s_fiber_id), m_func(func) {
  ++s_fiber_count;
  m_stacksize = stacksize ? stacksize : g_fiber_stack_size->getValue();

  m_stack = StackAllocator::Alloc(m_stacksize);
  if (getcontext(&m_ctx)) {
    LIONET_ASSERT2(false, "getcontext");
  }
  m_ctx.uc_link = nullptr;
  m_ctx.uc_stack.ss_sp = m_stack;
  m_ctx.uc_stack.ss_size = m_stacksize;

  // 统一函数入口及后处理
  if (!use_caller) {
    makecontext(&m_ctx, &Fiber::MainFunc, 0);
  } else {
    makecontext(&m_ctx, &Fiber::CallerMainFunc, 0);
  }

  LIONET_DEBUG(g_logger) << "Fiber::Fiber id=" << m_id;
}

Fiber::~Fiber() {
  --s_fiber_count;
  if (m_stack) {  // 子协程析构
    LIONET_ASSERT(m_state == TERM || m_state == EXCEPT || m_state == INIT);
    StackAllocator::Dealloc(m_stack, m_stacksize);
  } else {  // 主协程析构
    LIONET_ASSERT(!m_func);
    LIONET_ASSERT(m_state == EXEC);

    Fiber* cur = t_fiber;
    if (cur == this) {
      SetThis(nullptr);
    }
  }
  LIONET_DEBUG(g_logger) << "Fiber::~Fiber id=" << m_id
                         << ", total=" << s_fiber_count;
}

void Fiber::reset(std::function<void()> func) {
  LIONET_ASSERT(m_stack);
  LIONET_ASSERT(m_state == TERM || m_state == EXCEPT || m_state == INIT);

  m_func = func;
  if (getcontext(&m_ctx)) {
    LIONET_ASSERT2(false, "getcontext");
  }

  m_ctx.uc_link = nullptr;
  m_ctx.uc_stack.ss_sp = m_stack;
  m_ctx.uc_stack.ss_size = m_stacksize;

  makecontext(&m_ctx, &Fiber::MainFunc, 0);
  m_state = INIT;
}

void Fiber::call() {
  SetThis(this);
  m_state = EXEC;
  if (swapcontext(&t_threadFiber->m_ctx, &m_ctx)) {
    LIONET_ASSERT2(false, "swapcontext");
  }
}

void Fiber::back() {
  SetThis(t_threadFiber.get());
  if (swapcontext(&m_ctx, &t_threadFiber->m_ctx)) {
    LIONET_ASSERT2(false, "swapcontext");
  }
}

void Fiber::swapIn() {
  SetThis(this);
  LIONET_ASSERT(this);
  m_state = EXEC;
  if (swapcontext(&Scheduler::GetMainFiber()->m_ctx, &m_ctx)) {
    LIONET_ASSERT2(false, "swapcontext");
  }
}

void Fiber::swapOut() {
  SetThis(Scheduler::GetMainFiber());
  if (swapcontext(&m_ctx, &Scheduler::GetMainFiber()->m_ctx)) {
    LIONET_ASSERT2(false, "swapcontext");
  }
}

void Fiber::SetThis(Fiber* f) {
  t_fiber = f;
}

Fiber::ptr Fiber::GetThis() {
  if (t_fiber) {
    return t_fiber->shared_from_this();
  }

  Fiber::ptr main_fiber(new Fiber);
  LIONET_ASSERT(t_fiber == main_fiber.get());
  t_threadFiber = main_fiber;
  return t_fiber->shared_from_this();
}

void Fiber::YieldToReady() {
  Fiber::ptr cur = GetThis();
  LIONET_ASSERT(cur->m_state == EXEC);
  cur->m_state = READY;
  cur->back();
}

void Fiber::YieldToHold() {
  Fiber::ptr cur = GetThis();
  LIONET_ASSERT(cur->m_state == EXEC);
  cur->back();
}

uint64_t Fiber::TotalFibers() {
  return s_fiber_count;
}

void Fiber::MainFunc() {
  Fiber::ptr cur = GetThis();
  LIONET_ASSERT(cur);

  try {
    cur->m_func();
    cur->m_func = nullptr;
    cur->m_state = TERM;
  } catch (std::exception& e) {
    cur->m_state = EXCEPT;
    LIONET_ERROR(g_logger) << "Fiber Except: " << e.what()
                           << ", fiber_id=" << cur->getId() << std::endl
                           << LioNet::BacktraceToString();
  } catch (...) {
    cur->m_state = EXCEPT;
    cur->m_state = EXCEPT;
    LIONET_ERROR(g_logger) << "Fiber Except"
                           << ", fiber_id=" << cur->getId() << std::endl
                           << LioNet::BacktraceToString();
  }

  auto raw_ptr = cur.get();
  cur.reset();
  raw_ptr->swapOut();

  LIONET_ASSERT2(false,
                 "never reach fiber_id=" + std::to_string(raw_ptr->getId()));
}

void Fiber::CallerMainFunc() {
  Fiber::ptr cur = GetThis();
  LIONET_ASSERT(cur);
  try {
    cur->m_func();
    cur->m_func = nullptr;
    cur->m_state = TERM;
  } catch (std::exception& e) {
    cur->m_state = EXCEPT;
    LIONET_ERROR(g_logger) << "Fiber Except: " << e.what()
                           << ", fiber_id=" << cur->getId() << std::endl
                           << LioNet::BacktraceToString();
  } catch (...) {
    cur->m_state = EXCEPT;
    LIONET_ERROR(g_logger) << "Fiber Except"
                           << ", fiber_id=" << cur->getId() << std::endl
                           << LioNet::BacktraceToString();
  }

  auto raw_ptr = cur.get();
  cur.reset();
  raw_ptr->back();

  LIONET_ASSERT2(false,
                 "never reach fiber_id=" + std::to_string(raw_ptr->getId()));
}

}  // namespace LioNet