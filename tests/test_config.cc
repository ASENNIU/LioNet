#include <yaml-cpp/yaml.h>
#include <iostream>
#include <string>
#include "LioNet/log.h"
#include "LioNet/util.h"

void print_yaml(const YAML::Node& node, int level) {
  if (node.IsScalar()) {
    LIONET_INFO(LIONET_LOG_ROOT())
        << std::string(level * 4, ' ') << node.Scalar() << " - " << node.Type()
        << " - " << level;
  } else if (node.IsNull()) {
    LIONET_INFO(LIONET_LOG_ROOT()) << std::string(level * 4, ' ') << "NULL - "
                                   << node.Type() << " - " << level;
  } else if (node.IsMap()) {
    for (auto it = node.begin(); it != node.end(); ++it) {
      LIONET_INFO(LIONET_LOG_ROOT())
          << std::string(level * 4, ' ') << it->first << " - "
          << it->second.Type() << " - " << level;
      print_yaml(it->second, level + 1);
    }
  } else if (node.IsSequence()) {
    for (size_t i = 0; i < node.size(); ++i) {
      LIONET_INFO(LIONET_LOG_ROOT())
          << std::string(level * 4, ' ') << i << " - " << node[i].Type()
          << " - " << level;
      print_yaml(node[i], level + 1);
    }
  }
}

void test_yaml() {
  YAML::Node root =
      YAML::LoadFile("/home/leon/workspace/cpp/LioNet/bin/conf/log.yml");
  LIONET_INFO(LIONET_LOG_ROOT()) << root["test"].IsDefined();
  LIONET_INFO(LIONET_LOG_ROOT()) << root["logs"].IsDefined();
  LIONET_INFO(LIONET_LOG_ROOT()) << root;
  print_yaml(root, 0);
}

int main(int argc, char** argv) {
  test_yaml();
  return 0;
}