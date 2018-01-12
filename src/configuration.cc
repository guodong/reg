#include "configuration.h"

namespace reg {
void Configuration::AddBin(std::string name, std::string version) {
  //bins_[name] = version;
  bins_.insert(std::pair<std::string, std::string>(name, version)); 
}


}
