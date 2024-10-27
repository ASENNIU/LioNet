/***
 * @file config.h
 * @brief 配置模块
*/

#ifndef __LIONET_CONFIG_H__
#define __LIONET_CONFIG_H__

#include <yaml-cpp/yaml.h>
#include <algorithm>
#include <boost/lexical_cast.hpp>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "log.h"
#include "util.h"

namespace LioNet {

/**
 * @brief 配置变量基类
*/
class ConfigVarBase {
 public:
  typedef std::shared_ptr<ConfigVarBase> ptr;

  /**
   * @brief 构造函数
   * @param[in] name 配置参数[0-9a-z_.]
   * @param[in] desc 配置参数描述
  */
  ConfigVarBase(const std::string& name, const std::string& desc = "")
      : m_name(name), m_description(desc) {
    std::transform(m_name.begin(), m_name.end(), m_name.begin(), ::tolower);
  }
  virtual ~ConfigVarBase() = default;

  const std::string& getName() const { return m_name; }
  const std::string& getDescription() const { return m_description; }

  virtual std::string toString() = 0;

  virtual bool fromString(const std::string& val) = 0;

  virtual std::string getTypeName() const = 0;

 protected:
  std::string m_name;         // 配置参数的名称
  std::string m_description;  // 配置参数的描述
};

/**
 * @brief 类型转换模板类（F 源类型， T 类型目标）
 */
template <class F, class T>
class LexicalCast {
 public:
  /**
   * @brief 类型转换
   * @param[in] v， 源类型值
   * @param[in] 返回v转换后的目标类型
   * @exception 当类型不可转换时抛出异常
   */
  T operator()(const F& v) { return boost::lexical_cast<T>(v); }
};

/**
 * @brief 类型转换模板类偏特化（YAML String 转换为 std::vector<T>）
 */
template <class T>
class LexicalCast<std::string, std::vector<T>> {
 public:
  std::vector<T> operator()(const std::string& v) {
    YAML::Node node = YAML::Load(v);
    // typename 明确指出 std::vector<T> 是一个类型
    typename std::vector<T> vec;
    std::stringstream ss;
    for (size_t i = 0; i < node.size(); ++i) {
      ss.str("");
      ss << node[i];
      // LexicalCast<std::string, T>()调用默认构造函数，创建临时对象
      vec.push_back(LexicalCast<std::string, T>()(ss.str()));
    }
    return vec;
  }
};

/**
 * @brief 类型转换模板类偏特化（std::vector<T>转换为 YAML String）
 */
template <class T>
class LexicalCast<std::vector<T>, std::string> {
 public:
  std::string operator()(const std::vector<T>& v) {
    YAML::Node node(YAML::NodeType::Sequence);
    for (auto& it : v) {
      node.push_back(YAML::Load(LexicalCast<T, std::string>()(it)));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
  }
};

/**
 * @brief 类型转换模板类片特化(YAML String 转换成 std::list<T>)
 */
template <class T>
class LexicalCast<std::string, std::list<T>> {
 public:
  std::list<T> operator()(const std::string& v) {
    YAML::Node node = YAML::Load(v);
    typename std::list<T> vec;
    std::stringstream ss;
    for (size_t i = 0; i < node.size(); ++i) {
      ss.str("");
      ss << node[i];
      vec.push_back(LexicalCast<std::string, T>()(ss.str()));
    }
    return vec;
  }
};

/**
 * @brief 类型转换模板类片特化(std::list<T> 转换成 YAML String)
 */
template <class T>
class LexicalCast<std::list<T>, std::string> {
 public:
  std::string operator()(const std::list<T>& v) {
    YAML::Node node(YAML::NodeType::Sequence);
    for (auto& i : v) {
      node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
  }
};

/**
 * @brief 类型转换模板类片特化(YAML String 转换成 std::set<T>)
 */
template <class T>
class LexicalCast<std::string, std::set<T>> {
 public:
  std::set<T> operator()(const std::string& v) {
    YAML::Node node = YAML::Load(v);
    typename std::set<T> vec;
    std::stringstream ss;
    for (size_t i = 0; i < node.size(); ++i) {
      ss.str("");
      ss << node[i];
      vec.insert(LexicalCast<std::string, T>()(ss.str()));
    }
    return vec;
  }
};

/**
 * @brief 类型转换模板类片特化(std::set<T> 转换成 YAML String)
 */
template <class T>
class LexicalCast<std::set<T>, std::string> {
 public:
  std::string operator()(const std::set<T>& v) {
    YAML::Node node(YAML::NodeType::Sequence);
    for (auto& i : v) {
      node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
  }
};

/**
 * @brief 类型转换模板类片特化(YAML String 转换成 std::unordered_set<T>)
 */
template <class T>
class LexicalCast<std::string, std::unordered_set<T>> {
 public:
  std::unordered_set<T> operator()(const std::string& v) {
    YAML::Node node = YAML::Load(v);
    typename std::unordered_set<T> vec;
    std::stringstream ss;
    for (size_t i = 0; i < node.size(); ++i) {
      ss.str("");
      ss << node[i];
      vec.insert(LexicalCast<std::string, T>()(ss.str()));
    }
    return vec;
  }
};

/**
 * @brief 类型转换模板类片特化(std::unordered_set<T> 转换成 YAML String)
 */
template <class T>
class LexicalCast<std::unordered_set<T>, std::string> {
 public:
  std::string operator()(const std::unordered_set<T>& v) {
    YAML::Node node(YAML::NodeType::Sequence);
    for (auto& i : v) {
      node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
  }
};

/**
 * @brief 类型转换模板类片特化(YAML String 转换成 std::map<std::string, T>)
 */
template <class T>
class LexicalCast<std::string, std::map<std::string, T>> {
 public:
  std::map<std::string, T> operator()(const std::string& v) {
    YAML::Node node = YAML::Load(v);
    typename std::map<std::string, T> vec;
    std::stringstream ss;
    for (auto it = node.begin(); it != node.end(); ++it) {
      ss.str("");
      ss << it->second;
      vec.insert(std::make_pair(it->first.Scalar(),
                                LexicalCast<std::string, T>()(ss.str())));
    }
    return vec;
  }
};

/**
 * @brief 类型转换模板类片特化(std::map<std::string, T> 转换成 YAML String)
 */
template <class T>
class LexicalCast<std::map<std::string, T>, std::string> {
 public:
  std::string operator()(const std::map<std::string, T>& v) {
    YAML::Node node(YAML::NodeType::Map);
    for (auto& i : v) {
      node[i.first] = YAML::Load(LexicalCast<T, std::string>()(i.second));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
  }
};

/**
 * @brief 类型转换模板类片特化(YAML String 转换成 std::unordered_map<std::string, T>)
 */
template <class T>
class LexicalCast<std::string, std::unordered_map<std::string, T>> {
 public:
  std::unordered_map<std::string, T> operator()(const std::string& v) {
    YAML::Node node = YAML::Load(v);
    typename std::unordered_map<std::string, T> vec;
    std::stringstream ss;
    for (auto it = node.begin(); it != node.end(); ++it) {
      ss.str("");
      ss << it->second;
      vec.insert(std::make_pair(it->first.Scalar(),
                                LexicalCast<std::string, T>()(ss.str())));
    }
    return vec;
  }
};

/**
 * @brief 类型转换模板类片特化(std::unordered_map<std::string, T> 转换成 YAML String)
 */
template <class T>
class LexicalCast<std::unordered_map<std::string, T>, std::string> {
 public:
  std::string operator()(const std::unordered_map<std::string, T>& v) {
    YAML::Node node(YAML::NodeType::Map);
    for (auto& i : v) {
      node[i.first] = YAML::Load(LexicalCast<T, std::string>()(i.second));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
  }
};

/**
 * @brief 配置参数模板子类,保存对应类型的参数值
 * @details T 参数的具体类型
 *          FromStr 从std::string转换成T类型的仿函数
 *          ToStr 从T转换成std::string的仿函数
 *          std::string 为YAML格式的字符串
 */
template <class T, class FromStr = LexicalCast<std::string, T>,
          class ToStr = LexicalCast<T, std::string>>
class ConfigVar : public ConfigVarBase {
 public:
  typedef std::shared_ptr<ConfigVar> ptr;
  typedef std::function<void(const T& old_value, const T& new_value)>
      on_change_cb;

  /**
   * @brief 通过参数名,参数值,描述构造ConfigVar
   * @param[in] name 参数名称有效字符为[0-9a-z_.]
   * @param[in] default_value 参数的默认值
   * @param[in] desc 参数的描述
   */
  ConfigVar(const std::string& name, const T& default_value,
            const std::string& desc = "")
      : ConfigVarBase(name, desc), m_val(default_value) {}

  /**
   * @brief 将参数值转换为YAML String
   * @exception 当转换失败抛出异常
   */
  std::string toString() override {
    try {
      return ToStr()(m_val);
    } catch (std::exception& e) {
      LIONET_ERROR(LIONET_LOG_ROOT())
          << "ConfigVar::toString exception" << e.what() << " convert"
          << TypeToName<T>() << " to string"
          << " name=" << m_name;
    }
    return "";
  }

  /**
   * @brief 从YAML String 转换为参数值
   * @exception 当转换失败抛出异常
   */
  bool fromString(const std::string& val) override {
    try {
      setValue(FromStr()(val));
      return true;
    } catch (std::exception& e) {
      LIONET_ERROR(LIONET_LOG_ROOT())
          << "ConfigVar::fromString exception" << e.what()
          << " convert: string to " << TypeToName<T>() << " name=" << m_name
          << " - " << val;
    }
    return false;
  }

  /**
   * @brief 获取当前参数的值
   */
  const T getValue() { return m_val; }

  /**
     * @brief 设置当前参数的值
     * @details 如果参数的值有发生变化,则通知对应的注册回调函数
     */
  void setValue(const T& v) {
    if (v == m_val) {
      return;
    }
    for (auto& i : m_cbs) {
      // 触发回调
      i.second(m_val, v);
    }
    m_val = v;
  }

  /**
   * @brief 返回参数值的类型名称(typeinfo)
   */
  std::string getTypeName() const override { return TypeToName<T>(); }

  /**
   * @brief 添加变化回调函数
   * @return 返回该回调函数对应的唯一id,用于删除回调
   */
  uint64_t addListener(on_change_cb cb) {
    static uint64_t s_fun_id = 0;
    ++s_fun_id;
    m_cbs[s_fun_id] = cb;
    return s_fun_id;
  }

  /**
   * @brief 删除回调函数
   * @param[in] key 回调函数的唯一id
   */
  void delListener(uint64_t key) { m_cbs.erase(key); }

  /**
   * @brief 获取回调函数
   * @param[in] key 回调函数的唯一id    
   * @return 如果存在返回对应的回调函数,否则返回nullptr
   */
  on_change_cb getListener(uint64_t key) {
    auto it = m_cbs.find(key);
    return it == m_cbs.end() ? nullptr : it->second;
  }

  /**
   * @brief 清理所有的回调函数
   */
  void clearListener() { m_cbs.clear(); }

 private:
  T m_val;
  // 变更回调函数组, uint64_t key, 要求唯一
  std::map<uint64_t, on_change_cb> m_cbs;
};

/**
 * @brief ConfigVar的管理类
 * @details 提供便捷的方法创建/访问Config
*/
class Config {
 public:
  typedef std::unordered_map<std::string, ConfigVarBase::ptr> ConfigVarMap;

  /**
   * @brief 获取/创建对应参数名的配置参数
   * @param[in] name 配置参数名称
   * @param[in] default_value 参数默认值
   * @param[in] description 参数描述
   * @details 获取参数名为name的配置参数,如果存在直接返回
   *          如果不存在,创建参数配置并用default_value赋值
   * @return 返回对应的配置参数,如果参数名存在但是类型不匹配则返回nullptr
   * @exception 如果参数名包含非法字符[^0-9a-z_.] 抛出异常 std::invalid_argument
   */
  template <class T>
  static typename ConfigVar<T>::ptr Lookup(const std::string& name,
                                           const T& default_value,
                                           const std::string& desc = "") {
    auto it = GetDatas().find(name);
    if (it != GetDatas().end()) {
      auto tmp = std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
      if (tmp) {
        LIONET_INFO(LIONET_LOG_ROOT()) << "Lookup name=" << name << " exists";
        return tmp;
      } else {
        LIONET_INFO(LIONET_LOG_ROOT())
            << "Lookup name=" << name << " exists but type not "
            << TypeToName<T>() << " real_type= " << it->second->getTypeName()
            << " " << it->second->toString();

        return nullptr;
      }
    }

    if (name.find_first_not_of("abcdefghikjlmnopqrstuvwxyz._012345678") !=
        std::string::npos) {
      LIONET_ERROR(LIONET_LOG_ROOT()) << "Lookup name invalid" << name;
      throw std::invalid_argument(name);
    }

    typename ConfigVar<T>::ptr v(new ConfigVar<T>(name, default_value, desc));
    GetDatas()[name] = v;
    return v;
  }

  /**
     * @brief 查找配置参数
     * @param[in] name 配置参数名称
     * @return 返回配置参数名为name的配置参数
     */
  template <class T>
  static typename ConfigVar<T>::ptr Lookup(const std::string& name) {
    auto it = GetDatas().find(name);
    if (it == GetDatas().end()) {
      return nullptr;
    }
    return std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
  }

  /**
   * @brief 使用YAML::Node初始化配置模块
   */
  static void LoadFromYaml(const YAML::Node& root);

  /**
   * @brief 加载path文件夹里面的配置文件
   */
  static void LoadFromConfDir(const std::string& path, bool force = false);

  /**
   * @brief 查找配置参数,返回配置参数的基类
   * @param[in] name 配置参数名称
   */
  static ConfigVarBase::ptr LookupBase(const std::string& name);

  /**
   * @brief 遍历配置模块里面所有配置项
   * @param[in] cb 配置项回调函数
   */
  static void Visit(std::function<void(ConfigVarBase::ptr)> cb);

 private:
  /**
   * @brief 返回所有的配置项
   */
  static ConfigVarMap& GetDatas() {
    static ConfigVarMap s_datas;
    return s_datas;
  }
};

}  // namespace LioNet

#endif