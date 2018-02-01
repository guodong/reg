#ifndef REG_GENERATOR_H_
#define REG_GENERATOR_H_

#include <string>
#include "runtime_env.h"
#include "env.h"

namespace reg {
class Generator {
 public:
  
  // Example: generate("python@3.5.2", re)
  Env* Generate(const std::string& ret);

  Env* Generate(const std::vector<std::string> rets);

  void set_envroot(const std::string envroot) { envroot_ = envroot; }
  std::string envroot() { return envroot_; }

 private:
  int Init();
  Env* FetchEnv(const std::string &env_name);
  std::vector<std::string> GetDependencyDirs(Env *env);
  int MakeFs(Env *env);
  int Gendirs();

  std::string envroot_;

  Env env_;
};
}

#endif
