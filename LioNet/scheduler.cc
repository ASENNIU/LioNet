#include "log.h"
#include "macro.h"
#include "scheduler.h"

namespace LioNet {

static LioNet::Logger::ptr g_logger = LIONET_LOG_NAME("system");

static thread_local Scheduler* t_scheduler = nullptr;
static thread_local Fiber* t_scheduler_fiber = nullptr;

Scheduler::Scheduler(size_t threads, bool use_caller, const std::string& name)
    : m_name(name) {
  LIONET_ASSERT(threads > 0);

  if (use_caller) {
    LioNet::Fiber::GetThis();
    --threads;

    LIONET_ASSERT(GetThis() == nullptr);
    t_scheduler = this;

    m_rootFiber.reset(new Fiber(std::bind(&Scheduler::run, this), 0, true));
    LioNet::Thread::SetName(m_name);

    t_scheduler_fiber = m_rootFiber.get();
    m_rootThread = LioNet::GetThreadId();
    m_threadIds.push_back(m_rootThread);
  } else {
    m_rootThread = -1;
  }

  m_threadCount = threads;
}

Scheduler::~Scheduler() {
  LIONET_ASSERT(m_stopping);
  if (GetThis() == this) {
    t_scheduler = nullptr;
  }
}

Scheduler* Scheduler::GetThis() {
  return t_scheduler;
}

Fiber* Scheduler::GetMainFiber() {
  return t_scheduler_fiber;
}

void Scheduler::start() {
  MutexType::Lock lock(m_mutex);

  if (!m_stopping) {
    return;
  }

  m_stopping = false;
  LIONET_ASSERT(m_threads.empty());

  m_threads.resize(m_threadCount);
  for (size_t i = 0; i < m_threadCount; ++i) {
    m_threads[i].reset(new Thread(std::bind(&Scheduler::run, this),
                                  m_name + "_" + std::to_string(i)));
    m_threadIds.push_back(m_threads[i]->getId());
  }
}

void Scheduler::stop() {
  m_autoStop = true;
  if (m_rootFiber && m_threadCount == 0 &&
      (m_rootFiber->getState() == Fiber::TERM ||
       m_rootFiber->getState() == Fiber::INIT)) {
    m_stopping = true;

    if (stopping()) {
      return;
    }
  }

  if (m_rootThread != -1) {
    LIONET_ASSERT(GetThis() == this);
  } else {
    LIONET_ASSERT(GetThis() != this);
  }

  m_stopping = true;
  for (size_t i = 0; i < m_threadCount; ++i) {
    tickle();
  }

  if (m_rootFiber) {
    tickle();
  }

  if (m_rootFiber) {
    if (!stopping()) {
      m_rootFiber->call();
    }
  }

  std::vector<Thread::ptr> thrs;
  {
    MutexType::Lock lock(m_mutex);
    thrs.swap(m_threads);
  }

  for (auto& i : thrs) {
    i->join();
  }
}

void Scheduler::setThis() {
  t_scheduler = this;
}

void Scheduler::run() {
  LIONET_DEBUG(g_logger) << m_name;

  setThis();
  // 设置当前线程的主协程
  if (LioNet::GetThreadId() != m_rootThread) {
    t_scheduler_fiber = Fiber::GetThis().get();
  }

  Fiber::ptr idle_fiber(new Fiber(std::bind(&Scheduler::idle, this)));
  Fiber::ptr func_fiber;

  FiberAndThread ft;
  while (true) {
    ft.reset();
    bool tickle_me = false;
    bool is_active = false;

    {
      MutexType::Lock lock(m_mutex);
      auto it = m_fibers.begin();
      while (it != m_fibers.end()) {
        if (it->thread != -1 && it->thread != LioNet::GetThreadId()) {
          ++it;
          tickle_me = true;
          continue;
        }
        LIONET_ASSERT(it->fiber || it->func);
        if (it->fiber && it->fiber->getState() == Fiber::EXEC) {
          ++it;
          continue;
        }

        ft = *it;
        it = m_fibers.erase(it);
        ++m_activeThreadCount;
        is_active = true;
        break;
      }
      tickle_me |= it != m_fibers.end();
    }

    if (tickle_me) {
      tickle();
    }

    if (ft.fiber && (ft.fiber->getState() != Fiber::TERM &&
                     ft.fiber->getState() != Fiber::EXCEPT)) {
      ft.fiber->swapIn();
      --m_activeThreadCount;

      if (ft.fiber->getState() == Fiber::READY) {
        schedule(ft.fiber);
      } else if (ft.fiber->getState() != Fiber::TERM &&
                 ft.fiber->getState() != Fiber::EXCEPT) {
        ft.fiber->m_state = Fiber::HOLD;
      }
      ft.reset();
    } else if (ft.func) {
      if (func_fiber) {
        func_fiber->reset(ft.func);
      } else {
        func_fiber.reset(new Fiber(ft.func));
      }
      ft.reset();

      func_fiber->swapIn();
      --m_activeThreadCount;
      if (func_fiber->getState() == Fiber::READY) {
        schedule(func_fiber);
        func_fiber.reset();
      } else if (func_fiber->getState() == Fiber::EXCEPT ||
                 func_fiber->getState() == Fiber::TERM) {
        func_fiber->reset(nullptr);
      } else {
        func_fiber->m_state = Fiber::HOLD;
        func_fiber.reset();
      }
    } else {
      if (is_active) {
        --m_activeThreadCount;
        continue;
      }

      if (idle_fiber->getState() == Fiber::TERM) {
        LIONET_INFO(g_logger) << "idle fiber term";
        break;
      }

      ++m_idleThreadCount;
      idle_fiber->swapIn();
      --m_idleThreadCount;
      if (idle_fiber->getState() != Fiber::TERM &&
          idle_fiber->getState() != Fiber::EXCEPT) {
        idle_fiber->m_state = Fiber::HOLD;
      }
    }
  }
}

void Scheduler::tickle() {
  LIONET_INFO(g_logger) << "tickle";
}

bool Scheduler::stopping() {
  MutexType::Lock lock(m_mutex);
  return m_autoStop && m_stopping && m_fibers.empty() &&
         m_activeThreadCount == 0;
}

void Scheduler::idle() {
  LIONET_INFO(g_logger) << "run idle task";
  while (!stopping()) {
    Fiber::YieldToHold();
  }
}

void Scheduler::switchTo(int thread) {
  LIONET_ASSERT(Scheduler::GetThis() != nullptr);
  if (Scheduler::GetThis() == this) {
    if (thread == -1 || thread == LioNet::GetThreadId()) {
      return;
    }
  }
  schedule(LioNet::Fiber::GetThis(), thread);
  LioNet::Fiber::YieldToHold();
}

std::ostream& Scheduler::dump(std::ostream& os) {
  os << "[Scheduler name=" << m_name << " size=" << m_threadCount
     << " active_count=" << m_activeThreadCount
     << " idle_count=" << m_idleThreadCount << " stopping=" << m_stopping
     << " ]" << std::endl
     << "    ";
  for (size_t i = 0; i < m_threadIds.size(); ++i) {
    if (i) {
      os << ", ";
    }
    os << m_threadIds[i];
  }
  return os;
}

SchedulerSwitcher::SchedulerSwitcher(Scheduler* target) {
  m_caller = Scheduler::GetThis();
  if (target) {
    target->switchTo();
  }
}

SchedulerSwitcher::~SchedulerSwitcher() {
  if (m_caller) {
    m_caller->switchTo();
  }
}

}  // namespace LioNet