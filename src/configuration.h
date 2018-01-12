#ifndef REG_CONFIGURATION_H_
#define REG_CONFIGURATION_H_

#include <string>
#include <map>

namespace reg {
class Configuration {
 public:
  void AddBin(std::string name, std::string version);
  
  std::map<std::string, std::string> bins() {return bins_;}

 private:
  std::map<std::string, std::string> bins_;
};
}

#endif
