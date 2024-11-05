#include <benchmark/benchmark.h>
#include <vector>
#include "lionet.h"

LioNet::Logger::ptr g_logger = LIONET_LOG_NAME("system");

void simple_fiber_func() {
  LioNet::Fiber::YieldToHold();
}

static void BM_MultipleFibers(benchmark::State& state) {
  const int fiber_count = state.range(0);

  for (auto _ : state) {
    state.PauseTiming();
    std::vector<LioNet::Fiber::ptr> fibers;
    fibers.reserve(fiber_count);
    LioNet::Fiber::GetThis();  // 确保主协程已初始化
    state.ResumeTiming();

    // 创建多个Fiber
    for (int i = 0; i < fiber_count; ++i) {
      fibers.emplace_back(new LioNet::Fiber(simple_fiber_func));
    }

    // 第一轮切换：主协程 -> 每个Fiber
    for (auto& fiber : fibers) {
      fiber->swapIn();
    }

    // 第二轮切换：再次切换到每个Fiber，使其完成执行
    for (auto& fiber : fibers) {
      fiber->swapIn();
    }

    state.PauseTiming();
    fibers.clear();  // 清理Fiber
    state.ResumeTiming();
  }

  state.SetItemsProcessed(state.iterations() * fiber_count *
                          2);  // 每个Fiber切换两次
  state.SetComplexityN(fiber_count);
}

// 运行不同数量的Fiber
BENCHMARK(BM_MultipleFibers)
    ->RangeMultiplier(2)
    ->Range(1, 1 << 12)
    ->Complexity(benchmark::oN);

BENCHMARK_MAIN();
