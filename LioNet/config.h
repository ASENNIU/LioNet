#ifndef __LIONET_CONFIG_H__
#define __LIONET_CONFIG_H__

#include <memory>
#include <sstream>
#include "log.h"

namespace LioNet {

class ConfigVarBase {
 public:
  typedef std::shared_ptr<ConfigVarBase> ptr;

  ConfigVarBase(const std::string& name, const std::string& desc = "")
      : m_name(name), m_description(desc) {}
  virtual ~ConfigVarBase() = default;

  const std::string& getName() const { return m_name; }
  const std::string& getDescription() const { return m_description; }

  virtual std::string toString() = 0;
  virtual bool fromString(const std::string& val) = 0;

 protected:
  std::string m_name;
  std::string m_description;
};

template <class T>
class ConfigVar : public ConfigVarBase {
 public:
  typedef std::shared_ptr<ConfigVar> ptr;

  ConfigVar(const std::string& name, const T& default_val,
            const std::string& desc = "")
      : ConfigVarBase(name, desc), m_val(default_val) {}

  std::string toString() override;
  bool fromString(const std::string& val) override;

 private:
  T m_val;
};

}  // namespace LioNet

#endif