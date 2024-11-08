#include "lionet.h"

static LioNet::Logger::ptr g_logger = LIONET_LOG_NAME("system");

void run_in_fiber() {
  LIONET_INFO(g_logger) << "run_in_fiber begin";
  LioNet::Fiber::YieldToHold();
  LIONET_INFO(g_logger) << "run_in_fiber end";
  LioNet::Fiber::YieldToHold();
}

void test_fiber() {
  LIONET_INFO(g_logger) << "main begin -1";
  {
    LioNet::Fiber::GetThis();  // 确保当前线程主协程已经建立
    LIONET_INFO(g_logger) << "main begin";
    LioNet::Fiber::ptr fiber(new LioNet::Fiber(run_in_fiber, 0, true));
    // fiber->swapIn();
    fiber->call();
    LIONET_INFO(g_logger) << "main after swapIn";
    // fiber->swapIn();
    fiber->call();
    LIONET_INFO(g_logger) << "main after end";
    // fiber->swapIn();
    fiber->call();
  }
  LIONET_INFO(g_logger) << "main after end2";
}

int main() {
  LioNet::Thread::SetName("main");

  std::vector<LioNet::Thread::ptr> thrs;

  for (int i = 0; i < 3; ++i) {
    thrs.push_back(LioNet::Thread::ptr(
        new LioNet::Thread(&test_fiber, "name_" + std::to_string(i))));
  }

  for (auto i : thrs) {
    i->join();
  }

  return 0;
}