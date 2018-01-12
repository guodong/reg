#ifndef REG_GENERATOR_H_
#define REG_GENERATOR_H_

#include <string>
#include "runtime_env.h"

namespace reg {
class Generator {
 public:
  int generate(const std::string& ret, const std::string& version, RuntimeEnv *re);

};
}

#endif
