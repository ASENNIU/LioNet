/**
 * @file util.h
 * @brief 常用的工具函数
*/

#ifndef __LIONET_UTIL_H__
#define __LIONET_UTIL_H__

#include <cxxabi.h>
#include <pthread.h>
#include <stdint.h>
#include <iostream>
#include <string>
#include <vector>

namespace LioNet {

/**
 * @brief 返回当前线程ID
*/
pid_t GetThreadId();

/**
 * @brief 返回当前协程的ID
 */
uint32_t GetFiberId();

/**
 * @brief 获取当前时间的毫秒
 */
uint64_t GetCurrentMS();

/**
 * @brief 获取当前时间的微秒
 */
uint64_t GetCurrentUS();

class FSUtil {
 public:
  static void ListAllFile(std::vector<std::string>& files,
                          const std::string& path, const std::string& subfix);
  static bool Mkdir(const std::string& dirname);
  static bool IsRunningPidfile(const std::string& pidfile);
  static bool Rm(const std::string& path);
  static bool Mv(const std::string& from, const std::string& to);
  static bool Realpath(const std::string& path, std::string& rpath);
  static bool Symlink(const std::string& frm, const std::string& to);
  static bool Unlink(const std::string& filename, bool exist = false);
  static std::string Dirname(const std::string& filename);
  static std::string Basename(const std::string& filename);
  static bool OpenForRead(std::ifstream& ifs, const std::string& filename,
                          std::ios_base::openmode mode);
  static bool OpenForWrite(std::ofstream& ofs, const std::string& filename,
                           std::ios_base::openmode mode);
};

template <class T>
const char* TypeToName() {
  static const char* s_name =
      abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr);
  return s_name;
}
}  // namespace LioNet

#endif