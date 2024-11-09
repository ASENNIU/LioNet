#include <atomic>
#include <chrono>
#include <iomanip>
#include <vector>
#include "lionet.h"

static LioNet::Logger::ptr g_logger = LIONET_LOG_NAME("system");
static std::atomic<size_t> s_fiber_count{0};

void fiber_func() {
  for (int i = 0; i < 1000; ++i) {
    // LioNet::Fiber::YieldToReady();
    LioNet::Fiber::YieldToHold();
  }
  ++s_fiber_count;
}

void test_fibers(size_t fiber_count, size_t thread_count) {
  LIONET_FATAL(g_logger) << "Testing with " << fiber_count << " fibers and "
                         << thread_count << " threads";

  s_fiber_count = 0;
  std::vector<LioNet::Fiber::ptr> fibers;

  // 测量创建时间
  auto start = std::chrono::high_resolution_clock::now();
  for (size_t i = 0; i < fiber_count; ++i) {
    fibers.push_back(LioNet::Fiber::ptr(new LioNet::Fiber(fiber_func)));
  }
  auto end = std::chrono::high_resolution_clock::now();
  auto creation_time =
      std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

  LioNet::Scheduler sched(thread_count, false, "test");
  sched.start();

  // 测量切换时间
  start = std::chrono::high_resolution_clock::now();
  // for (auto& fiber : fibers) {
  //   sched.schedule(fiber);
  // }
  sched.schedule(fibers.begin(), fibers.end());
  // 等待所有协程执行完毕
  while (s_fiber_count < fiber_count) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    // LIONET_FATAL(g_logger) << "finish coroutine: " << s_fiber_count;
  }

  end = std::chrono::high_resolution_clock::now();
  auto switch_time =
      std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

  sched.stop();

  double avg_creation_time = static_cast<double>(creation_time) / fiber_count;
  double avg_switch_time =
      static_cast<double>(switch_time) / (fiber_count * 1000);

  LIONET_FATAL(g_logger) << "Fiber count: " << fiber_count
                         << ", Thread count: " << thread_count
                         << ", Avg creation time: " << std::fixed
                         << std::setprecision(2) << avg_creation_time << " ns"
                         << ", Avg switch time: " << std::fixed
                         << std::setprecision(2) << avg_switch_time << " ns";
}

int main() {
  g_logger->setLevel(LioNet::LogLevel::ERROR);
  std::vector<size_t> fiber_counts = {1000, 3000};
  std::vector<size_t> thread_counts = {1, 2, 4, 8, 16};

  for (auto fiber_count : fiber_counts) {
    for (auto thread_count : thread_counts) {
      test_fibers(fiber_count, thread_count);
      // 在测试之间添加一些延迟，确保资源完全释放
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  }

  return 0;
}
