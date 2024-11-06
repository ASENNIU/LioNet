#include <benchmark/benchmark.h>
#include <vector>
#include "lionet.h"

LioNet::Logger::ptr g_logger;

// 关闭日志
LioNet::Logger::ptr getLogger() {
  if (!g_logger) {
    g_logger = LIONET_LOG_NAME("system");
    g_logger->setLevel(LioNet::LogLevel::ERROR);
  }
  return g_logger;
}

void simple_fiber_func() {
  LIONET_INFO(getLogger()) << "Fiber function running";
  LioNet::Fiber::YieldToHold();
  LIONET_INFO(getLogger()) << "Fiber function resumed";
}

// 1. 创建和销毁时间
static void BM_FiberCreateDestroy(benchmark::State& state) {
  LioNet::Fiber::GetThis();
  for (auto _ : state) {
    LIONET_INFO(getLogger()) << "Creating fiber";
    LioNet::Fiber::ptr fiber(new LioNet::Fiber(simple_fiber_func));
    LIONET_INFO(getLogger()) << "Swapping in fiber";
    fiber->swapIn();
    LIONET_INFO(getLogger()) << "Swapping in fiber again";
    fiber->swapIn();
    LIONET_INFO(getLogger()) << "Fiber completed";
  }
}

// 2. 切换性能
static void BM_FiberSwitch(benchmark::State& state) {
  LioNet::Fiber::GetThis();
  for (auto _ : state) {
    LIONET_INFO(getLogger()) << "Creating fiber for switch test";
    LioNet::Fiber::ptr fiber(new LioNet::Fiber(simple_fiber_func));
    fiber->swapIn();
    fiber->swapIn();
  }
  state.SetItemsProcessed(state.iterations() * 2);
}

// 3. 内存使用（通过创建大量Fiber来间接测量）
static void BM_FiberMemoryUsage(benchmark::State& state) {
  LioNet::Fiber::GetThis();
  const int fiber_count = state.range(0);
  for (auto _ : state) {
    state.PauseTiming();
    std::vector<LioNet::Fiber::ptr> fibers;
    fibers.reserve(fiber_count);
    state.ResumeTiming();

    for (int i = 0; i < fiber_count; ++i) {
      LIONET_INFO(getLogger()) << "Creating fiber " << i;
      auto fiber = LioNet::Fiber::ptr(new LioNet::Fiber(simple_fiber_func));
      fiber->swapIn();
      fiber->swapIn();
      fibers.push_back(std::move(fiber));
    }

    benchmark::DoNotOptimize(fibers);
    state.PauseTiming();
    LIONET_INFO(getLogger()) << "Clearing fibers";
    fibers.clear();
    state.ResumeTiming();
  }
  state.SetItemsProcessed(state.iterations() * fiber_count);
}

// 4. 上下文切换开销（与函数调用对比）
void dummy_function() {
  LIONET_INFO(getLogger()) << "Dummy function called";
}

static void BM_FunctionCall(benchmark::State& state) {
  LioNet::Fiber::GetThis();
  for (auto _ : state) {
    dummy_function();
  }
}

static void BM_FiberContextSwitch(benchmark::State& state) {
  LioNet::Fiber::GetThis();

  for (auto _ : state) {
    LioNet::Fiber::ptr fiber(
        new LioNet::Fiber([] {}));  // 空函数，只测试上下文切换
    fiber->swapIn();
  }
  state.SetItemsProcessed(state.iterations());
}

BENCHMARK(BM_FiberCreateDestroy);
BENCHMARK(BM_FiberSwitch);
BENCHMARK(BM_FiberMemoryUsage)->RangeMultiplier(2)->Range(1, 1 << 12);
BENCHMARK(BM_FunctionCall);
BENCHMARK(BM_FiberContextSwitch);

BENCHMARK_MAIN();
