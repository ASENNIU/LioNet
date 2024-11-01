#include "env.h"
#include "config.h"
#include "log.h"

#include <stdlib.h>
#include <unistd.h>
#include <iomanip>
#include <iostream>

namespace LioNet {

static LioNet::Logger::ptr g_logger = LIONET_LOG_NAME("system");
/**
 * @brief 初始化程序执行参数
 * @details 获取当前可执行文件的路径并设置相关成员变量
 *          解析命令行参数，将参数键值对存储到环境配置中
 *          处理错误情况并记录日志
*/
bool Env::init(int argc, char** argv) {
  char link[1024] = {0};  // 符号链接路径
  char path[1024] = {0};  // 实际路劲
  sprintf(link, "/proc/%d/exe", getpid());

  // 读取符号链接并处理返回值
  // ssize_t readlink(const char*, char*, size_t)’ declared with attribute ‘warn_unused_result’
  ssize_t len = readlink(link, path, sizeof(path));

  if (len == -1) {
    LIONET_ERROR(g_logger) << "Failed to readlink: " << strerror(errno);
    return false;
  }
  // /path/xxx/exe
  m_exe = path;

  auto pos = m_exe.find_last_of("/");
  m_cwd = m_exe.substr(0, pos) + "/";

  m_program = argv[0];
  // -config /path/to/config -file xxxx -d
  const char* now_key = nullptr;
  for (int i = 1; i < argc; ++i) {
    if (argv[i][0] == '-') {
      if (strlen(argv[i]) > 1) {
        if (now_key) {
          add(now_key, "");
        }
        now_key = argv[i] + 1;
      } else {
        LIONET_ERROR(g_logger) << "invalid arg idx=" << i << " val=" << argv[i];
      }
    } else {
      if (now_key) {
        add(now_key, argv[i]);
        now_key = nullptr;
      } else {
        LIONET_ERROR(g_logger) << "invalid arg idx=" << i << " val=" << argv[i];
        return false;
      }
    }
  }
  if (now_key) {
    add(now_key, "");
  }
  return true;
}

void Env::add(const std::string& key, const std::string& val) {
  m_args[key] = val;
}

bool Env::has(const std::string& key) {
  auto it = m_args.find(key);
  return it != m_args.end();
}

void Env::del(const std::string& key) {
  m_args.erase(key);
}

std::string Env::get(const std::string& key, const std::string& default_value) {
  auto it = m_args.find(key);
  return it != m_args.end() ? it->second : default_value;
}

void Env::addHelp(const std::string& key, const std::string& desc) {
  removeHelp(key);
  m_helps.push_back(std::make_pair(key, desc));
}

void Env::removeHelp(const std::string& key) {
  for (auto it = m_helps.begin(); it != m_helps.end(); ++it) {
    if (it->first == key) {
      it = m_helps.erase(it);
    } else {
      ++it;
    }
  }
}

void Env::printHelp() {
  std::cout << "Usage: " << m_program << " [options]" << std::endl;
  for (auto& i : m_helps) {
    std::cout << std::setw(5) << " - " << i.first << " : " << i.second
              << std::endl;
  }
}

bool Env::setEnv(const std::string& key, const std::string& val) {
  return !setenv(key.c_str(), val.c_str(), 1);
}

std::string Env::getEnv(const std::string& key,
                        const std::string& default_value) {
  const char* v = getenv(key.c_str());
  if (v == nullptr) {
    return default_value;
  }
  return v;
}

std::string Env::getAbsolutePath(const std::string& path) const {
  if (path.empty()) {
    return "/";
  }
  if (path[0] == '/') {
    return path;
  }
  return m_cwd + path;
}

std::string Env::getAbsoluteWorkPath(const std::string& path) const {
  if (path.empty()) {
    return "/";
  }
  if (path[0] == '/') {
    return path;
  }

  static LioNet::ConfigVar<std::string>::ptr g_server_work_path =
      LioNet::Config::Lookup<std::string>("server.work_path");
  return g_server_work_path->getValue() + "/" + path;
}

std::string Env::getConfigPath() {
  return getAbsolutePath(get("c", "conf"));
}
}  // namespace LioNet
