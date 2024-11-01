#include <unistd.h>
#include "LioNet/lionet.h"

LioNet::Logger::ptr g_logger = LIONET_LOG_NAME("system");

int count = 0;
// LioNet::Mutex mutex;
// LioNet::Spinlock mutex;
LioNet::CASLock mutex;
bool isStop = false;

void func1() {
  LIONET_INFO(g_logger) << "name: " << LioNet::Thread::GetName()
                        << " this.name: "
                        << LioNet::Thread::GetThis()->getName()
                        << " id: " << LioNet::GetThreadId() << " this.id"
                        << LioNet::Thread::GetThis()->getId();
  for (int i = 0; i < 1000000000; ++i) {
    // LioNet::Mutex::Lock lock(mutex);
    LioNet::CASLock::Lock lock(mutex);
    ++count;
  }
}

void func2() {
  while (!isStop) {
    LIONET_INFO(g_logger) << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
  }
}

void func3() {
  while (!isStop) {
    LIONET_INFO(g_logger) << "==========================================";
  }
}

int main() {
  std::cout << "Thread test begin..." << std::endl;
  LIONET_INFO(g_logger) << "Thread test begin.";
  pid_t tid = LioNet::GetThreadId();
  printf("Thread ID: %d\n", tid);
  YAML::Node root =
      YAML::LoadFile("/home/leon/workspace/cpp/LioNet/bin/conf/log.yml");
  LioNet::Config::LoadFromYaml(root);

  std::vector<LioNet::Thread::ptr> thrs;
  for (int i = 0; i < 1; ++i) {
    LioNet::Thread::ptr thr(
        new LioNet::Thread(&func2, "name_" + std::to_string(i * 2)));
    LioNet::Thread::ptr thr2(
        new LioNet::Thread(&func3, "name_" + std::to_string(i * 2 + 1)));
    thrs.push_back(thr);
    thrs.push_back(thr2);
  }
  LioNet::Thread::ptr thr(new LioNet::Thread(&func1, "name_0"));
  // sleep(1);
  isStop = true;
  for (size_t i = 0; i < thrs.size(); ++i) {
    thrs[i]->join();
  }
  thr->join();

  LIONET_INFO(g_logger) << "thread test end";
  LIONET_INFO(g_logger) << "count=" << count;

  return 0;
}