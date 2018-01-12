#ifndef REG_PARSER_H_
#define REG_PARSER_H_

#include <string>

namespace reg {

class Configuration;

class Parser {
 public:

  static void Parse(const std::string, Configuration* configuration);
 private:
  std::string config_file_;
  
};
}
#endif
