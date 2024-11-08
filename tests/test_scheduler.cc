#include "lionet.h"

static LioNet::Logger::ptr g_logger = LIONET_LOG_NAME("system");

void test_fiber() {
  static int s_count = 5;
  LIONET_INFO(g_logger) << "test in fiber s_count = " << s_count;

  sleep(1);
  if (--s_count >= 0) {
    LioNet::Scheduler::GetThis()->schedule(&test_fiber, LioNet::GetThreadId());
  }
}

int main() {
  LIONET_ASSERT2(g_logger->getName() == "system", "logger name");
  LIONET_INFO(g_logger) << "main";
  LioNet::Scheduler sched(3, true, "test");
  sched.start();
  sleep(2);

  LIONET_INFO(g_logger) << "schedule";
  sched.schedule(&test_fiber);
  sched.stop();
  LIONET_INFO(g_logger) << "over";

  return 0;
}