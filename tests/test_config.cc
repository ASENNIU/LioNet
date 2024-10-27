#include <yaml-cpp/yaml.h>
#include <iostream>
#include <string>
#include "LioNet/config.h"
#include "LioNet/log.h"
#include "LioNet/util.h"

#if 0

LioNet::ConfigVar<int>::ptr g_int_value_config =
    LioNet::Config::Lookup("system.port", (int)8080, "system port");

LioNet::ConfigVar<float>::ptr g_int_valuex_config =
    LioNet::Config::Lookup("system.port", (float)8080, "system port");

LioNet::ConfigVar<float>::ptr g_float_value_config =
    LioNet::Config::Lookup("system.value", (float)10.2f, "system port");

LioNet::ConfigVar<std::vector<int>>::ptr g_int_vec_value_config =
    LioNet::Config::Lookup("system.int_vec", std::vector<int>{1, 2},
                           "system int vec");

LioNet::ConfigVar<std::list<int>>::ptr g_int_list_value_config =
    LioNet::Config::Lookup("system.int_list", std::list<int>{1, 2},
                           "system list");

LioNet::ConfigVar<std::set<int>>::ptr g_int_set_value_config =
    LioNet::Config::Lookup("system.int_set", std::set<int>{1, 2},
                           "system int set");

LioNet::ConfigVar<std::unordered_set<int>>::ptr g_int_uset_value_config =
    LioNet::Config::Lookup("system.int_uset", std::unordered_set<int>{1, 2},
                           "system int uset");

LioNet::ConfigVar<std::map<std::string, int>>::ptr g_str_int_map_value_config =
    LioNet::Config::Lookup("system.str_int_map",
                           std::map<std::string, int>{{"k1", 2}},
                           "system str int map");

LioNet::ConfigVar<std::unordered_map<std::string, int>>::ptr
    g_str_int_umap_value_config = LioNet::Config::Lookup(
        "system.str_int_vec_umap",
        std::unordered_map<std::string, int>{{"", 2}}, "system str int umap");

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

void test_config() {
  LIONET_INFO(LIONET_LOG_ROOT())
      << "before: " << g_int_value_config->getValue();
  LIONET_INFO(LIONET_LOG_ROOT())
      << "before: " << g_float_value_config->getValue();

#define XX(g_var, name, prefix)                                      \
  {                                                                  \
    auto& v = g_var->getValue();                                     \
    for (auto& i : v) {                                              \
      LIONET_INFO(LIONET_LOG_ROOT()) << #prefix " " #name ": " << i; \
    }                                                                \
    LIONET_INFO(LIONET_LOG_ROOT())                                   \
        << #prefix " " #name " yaml: " << g_var->toString();         \
  }

#define XX_M(g_var, name, prefix)                                            \
  {                                                                          \
    auto& v = g_var->getValue();                                             \
    for (auto& i : v) {                                                      \
      LIONET_INFO(LIONET_LOG_ROOT())                                         \
          << #prefix " " #name ": {" << i.first << " - " << i.second << "}"; \
    }                                                                        \
    LIONET_INFO(LIONET_LOG_ROOT())                                           \
        << #prefix " " #name " yaml: " << g_var->toString();                 \
  }

  XX(g_int_vec_value_config, int_vec, before);
  XX(g_int_list_value_config, int_list, before);
  XX(g_int_set_value_config, int_set, before);
  XX(g_int_uset_value_config, int_uset, before);
  XX_M(g_str_int_map_value_config, str_int_map, before);
  XX_M(g_str_int_umap_value_config, str_int_umap, before);

  YAML::Node root =
      YAML::LoadFile("/home/leon/workspace/cpp/LioNet/bin/conf/log.yml");
  LioNet::Config::LoadFromYaml(root);

  LIONET_INFO(LIONET_LOG_ROOT()) << "after: " << g_int_value_config->getValue();
  LIONET_INFO(LIONET_LOG_ROOT())
      << "after: " << g_float_value_config->getValue();

  XX(g_int_vec_value_config, int_vec, after);
  XX(g_int_list_value_config, int_list, after);
  XX(g_int_set_value_config, int_set, after);
  XX(g_int_uset_value_config, int_uset, after);
  XX_M(g_str_int_map_value_config, str_int_map, after);
  XX_M(g_str_int_umap_value_config, str_int_umap, after);
}

#endif

class Person {
 public:
  Person(){};
  std::string m_name;
  int m_age = 0;
  bool m_sex = 0;

  std::string toString() const {
    std::stringstream ss;
    ss << "[Person name=" << m_name << " age=" << m_age << " sex=" << m_sex
       << "]";
    return ss.str();
  }

  bool operator==(const Person& oth) const {
    return m_name == oth.m_name && m_age == oth.m_age && m_sex == oth.m_sex;
  }
};

namespace LioNet {

template <>
class LexicalCast<std::string, Person> {
 public:
  Person operator()(const std::string& v) {
    YAML::Node node = YAML::Load(v);
    Person p;
    p.m_name = node["name"].as<std::string>();
    p.m_age = node["age"].as<int>();
    p.m_sex = node["sex"].as<bool>();
    return p;
  }
};

template <>
class LexicalCast<Person, std::string> {
 public:
  std::string operator()(const Person& p) {
    YAML::Node node;
    node["name"] = p.m_name;
    node["age"] = p.m_age;
    node["sex"] = p.m_sex;
    std::stringstream ss;
    ss << node;
    return ss.str();
  }
};
}  // namespace LioNet

LioNet::ConfigVar<Person>::ptr g_person =
    LioNet::Config::Lookup("class.person", Person(), "system person");

LioNet::ConfigVar<std::map<std::string, Person>>::ptr g_person_map =
    LioNet::Config::Lookup("class.map", std::map<std::string, Person>(),
                           "system person");

LioNet::ConfigVar<std::map<std::string, std::vector<Person>>>::ptr
    g_person_vec_map = LioNet::Config::Lookup(
        "class.vec_map", std::map<std::string, std::vector<Person>>(),
        "system person");

void test_class() {
  LIONET_INFO(LIONET_LOG_ROOT())
      << "before: " << g_person->getValue().toString() << " - "
      << g_person->toString();

#define XX_PM(g_var, prefix)                                            \
  {                                                                     \
    auto m = g_person_map->getValue();                                  \
    for (auto& i : m) {                                                 \
      LIONET_INFO(LIONET_LOG_ROOT())                                    \
          << prefix << ": " << i.first << " - " << i.second.toString(); \
    }                                                                   \
    LIONET_INFO(LIONET_LOG_ROOT()) << prefix << ": size=" << m.size();  \
  }

  g_person->addListener([](const Person& old_value, const Person& new_value) {
    LIONET_INFO(LIONET_LOG_ROOT()) << "old_value=" << old_value.toString()
                                   << " new_value=" << new_value.toString();
  });

  XX_PM(g_person_map, "class.map before");
  LIONET_INFO(LIONET_LOG_ROOT()) << "before: " << g_person_vec_map->toString();

  YAML::Node root =
      YAML::LoadFile("/home/leon/workspace/cpp/LioNet/bin/conf/log.yml");
  LioNet::Config::LoadFromYaml(root);

  LIONET_INFO(LIONET_LOG_ROOT()) << "after: " << g_person->getValue().toString()
                                 << " - " << g_person->toString();
  XX_PM(g_person_map, "class.map after");
  LIONET_INFO(LIONET_LOG_ROOT()) << "after: " << g_person_vec_map->toString();
}

int main(int argc, char** argv) {
  // test_yaml();
  // test_config();
  test_class();
  return 0;
}