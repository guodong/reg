#ifndef REG_ENV_H_
#define REG_ENV_H_

#include <vector>

namespace reg {
class Env {
 public:
  int AddDependency(Env *env);

  void set_root(const std::string root) { root_ = root; }
  std::string root() { return root_; }

  void set_name(const std::string name) { name_ = name; }
  std::string name() { return name_; }

  void set_version(const std::string version) { version_ = version; }
  std::string version() { return version_; }

 private:
  std::vector<Env> deps;
  std::string root_;

  // name of env
  std::string name_;

  // version of env, could be 1.2.3 or latest
  std::string version_;
};
}

#endif
