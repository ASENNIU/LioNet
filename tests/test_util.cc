#include "lionet.h"

LioNet::Logger::ptr g_logger = LIONET_LOG_ROOT();

void test_assert() {
  LIONET_INFO(g_logger) << LioNet::BacktraceToString(10);
  LIONET_ASSERT(0 == 1);
  LIONET_ASSERT2(5 == 6, "qwer xxx");
}

int main(int argc, char** argv) {
  test_assert();
  return 0;
}