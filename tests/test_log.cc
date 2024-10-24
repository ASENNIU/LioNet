#include <iostream>
#include "LioNet/log.h"
#include "LioNet/util.h"

int main(int argc, char** argv) {
  LioNet::Logger::ptr logger(new LioNet::Logger);
  logger->addAppender(LioNet::LogAppender::ptr(new LioNet::StdoutLogAppender));

  LioNet::FileLogAppender::ptr fil_appdender(
      new LioNet::FileLogAppender("./log.txt"));
  LioNet::LogFormatter::ptr fmt(new LioNet::LogFormatter("%d%T%p%T%m%n"));
  fil_appdender->setFormatter(fmt);
  fil_appdender->setLevel(LioNet::LogLevel::WARN);

  logger->addAppender(fil_appdender);

  std::cout << "Hello LioNet log." << std::endl;
  LIONET_INFO(logger) << "Test Macro";
  LIONET_FMT_ERROR(logger, "Test macro fmt error %s", "lio");
  LIONET_ERROR(logger) << "Test Macro OF FILEAPPENDER";

  auto l = LioNet::LoggerMgr::GetInstance()->getLogger("xx");
  LIONET_INFO(l) << "xxx";
  return 0;
}