#ifndef REG_ENV_H_
#define REG_ENV_H_

#include <vector>
#include <string>

namespace reg {
class Env {
 public:
  int AddDependency(Env *env);

  int Start();

  void set_root(const std::string root) { root_ = root; }
  std::string root() { return root_; }

  void set_name(const std::string name) { name_ = name; }
  std::string name() { return name_; }

  void set_version(const std::string version) { version_ = version; }
  std::string version() { return version_; }

  std::string source_dir() { return "/var/lib/envhub/envs/" + name_ + "/" + version_; }

  std::vector<Env*> dependencies() { return dependencies_; }

  void set_cmds(std::vector<std::string> cmds) { cmds_ = cmds; }
  std::vector<std::string> cmds() { return cmds_; }

  void set_parallel(bool parallel) { parallel_ = parallel; }

  std::string mount_dir() { return root_ + "/mount"; }

 private:
  static int Child(void *data);
  std::vector<Env*> dependencies_;
  std::string root_;

  // name of env
  std::string name_;

  // version of env, could be 1.2.3 or latest
  std::string version_;

  std::vector<std::string> cmds_;
  int parallel_;
  pid_t child_pid_;
  char child_stack_[8192];
};
}

#endif
