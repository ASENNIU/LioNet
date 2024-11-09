#include <benchmark/benchmark.h>
#include <atomic>
#include <vector>
#include "lionet.h"

static LioNet::Logger::ptr g_logger = LIONET_LOG_NAME("system");
static std::atomic<size_t> s_fiber_count{0};

void fiber_func() {
  for (int i = 0; i < 1000; ++i) {
    LioNet::Fiber::YieldToHold();
  }
  ++s_fiber_count;
}

class FiberFixture : public benchmark::Fixture {
 public:
  void SetUp(const ::benchmark::State& state) {
    fiber_count = state.range(0);
    thread_count = state.range(1);
    s_fiber_count = 0;
    g_logger->setLevel(LioNet::LogLevel::ERROR);
    // LioNet::Fiber::GetThis();  // 确保主协程被初始化
  }

  // void TearDown(const ::benchmark::State& state) {
  //   // 清理任何剩余的协程
  //   LioNet::Fiber::GetThis().reset();
  // }

  size_t fiber_count;
  size_t thread_count;
};

BENCHMARK_DEFINE_F(FiberFixture, BM_FiberCreation)(benchmark::State& state) {
  for (auto _ : state) {
    std::vector<LioNet::Fiber::ptr> fibers;
    for (size_t i = 0; i < fiber_count; ++i) {
      fibers.push_back(LioNet::Fiber::ptr(new LioNet::Fiber(fiber_func)));
    }
    benchmark::DoNotOptimize(fibers);
    benchmark::ClobberMemory();
  }
  state.SetItemsProcessed(state.iterations() * fiber_count);
}

BENCHMARK_DEFINE_F(FiberFixture, BM_FiberSwitch)(benchmark::State& state) {
  for (auto _ : state) {
    state.PauseTiming();
    std::vector<LioNet::Fiber::ptr> fibers;
    for (size_t i = 0; i < fiber_count; ++i) {
      fibers.push_back(LioNet::Fiber::ptr(new LioNet::Fiber(fiber_func)));
    }
    LioNet::Scheduler sched(thread_count, false, "test");
    sched.start();
    s_fiber_count = 0;
    state.ResumeTiming();

    sched.schedule(fibers.begin(), fibers.end());
    while (s_fiber_count < fiber_count) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    state.PauseTiming();
    sched.stop();
    // 确保所有协程都完成
    for (auto& fiber : fibers) {
      while (fiber->getState() != LioNet::Fiber::TERM) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
      }
    }
    fibers.clear();  // 显式清理 fibers
    state.ResumeTiming();
  }
  state.SetItemsProcessed(state.iterations() * fiber_count * 1000);
}

BENCHMARK_REGISTER_F(FiberFixture, BM_FiberCreation)
    ->Args({1000, 1})
    ->Args({1000, 2})
    ->Args({1000, 4})
    ->Args({1000, 8})
    ->Args({1000, 16})
    ->Args({3000, 1})
    ->Args({3000, 2})
    ->Args({3000, 4})
    ->Args({3000, 8})
    ->Args({3000, 16})
    ->Unit(benchmark::kNanosecond);

BENCHMARK_REGISTER_F(FiberFixture, BM_FiberSwitch)
    ->Args({1000, 1})
    ->Args({1000, 2})
    ->Args({1000, 4})
    ->Args({1000, 8})
    ->Args({1000, 16})
    ->Args({3000, 1})
    ->Args({3000, 2})
    ->Args({3000, 4})
    ->Args({3000, 8})
    ->Args({3000, 16})
    ->Unit(benchmark::kNanosecond);

BENCHMARK_MAIN();
