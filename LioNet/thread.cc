#include "LioNet/thread.h"
#include "LioNet/log.h"
#include "LioNet/util.h"

namespace LioNet {

// 线程局部变量，方便在任何地方获取当前线程的信息
static thread_local Thread* t_thread = nullptr;
static thread_local std::string t_thread_name = "UNKNOW";

static LioNet::Logger::ptr g_logger = LIONET_LOG_NAME("system");

Thread* Thread::GetThis() {
  return t_thread;
}

const std::string& Thread::GetName() {
  return t_thread_name;
}

void Thread::SetName(const std::string& name) {
  if (name.empty()) {
    return;
  }
  if (t_thread) {
    t_thread->m_name = name;
  }
  t_thread_name = name;
}

Thread::Thread(std::function<void()> func, const std::string& name)
    : m_func(func), m_name(name) {
  if (name.empty()) {
    m_name = "UNKNOW";
  }

  int rt = pthread_create(&m_thread, nullptr, &Thread::run, this);
  if (rt) {
    LIONET_ERROR(g_logger) << "pthread_create thread fail, rt=" << rt
                           << " name=" << name;
    throw std::logic_error("pthread_create error");
  }

  m_semaphore.wait();  // 等待子线程启动
}

Thread::~Thread() {
  if (m_thread) {
    pthread_detach(m_thread);
  }
}

void Thread::join() {
  if (m_thread) {
    int rt = pthread_join(m_thread, nullptr);
    if (rt) {
      LIONET_ERROR(g_logger)
          << "pthread_join thread fail, rt=" << rt << " name=" << m_name;
      throw std::logic_error("pthread_join error");
    }
    m_thread = 0;
  }
}

void* Thread::run(void* arg) {
  Thread* thread = (Thread*)arg;
  t_thread = thread;
  t_thread_name = thread->m_name;
  thread->m_id = LioNet::GetThreadId();
  pthread_setname_np(pthread_self(), thread->m_name.substr(0, 15).c_str());

  std::function<void()> task;
  // 使用 swap 的主要目的是为了高效地转移任务函数的所有权，避免不必要的引用计数开销，并确保任务函数只被执行一次。
  task.swap(thread->m_func);
  thread->m_semaphore.notify();  // 通知主线程子线程已经准备好运行任务

  task();
  return 0;
}

}  // namespace LioNet