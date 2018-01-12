#ifndef REG_BIN_H_
#define REG_BIN_H_

#include <string>

namespace reg {
class Bin {
 public:
  explicit Bin(const std::string name, const std::string version);
  
  std::string name() const {return name_;}
  void set_name(std::string name) {name_ = name;}

  std::string version() const {return version_;}
  void set_version(std::string version) {version_ = version;}
  
 private:
  std::string name_;
  std::string version_;
};
}

#endif
