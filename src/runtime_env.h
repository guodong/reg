#ifndef REG_RUNTIME_ENV_H_
#define REG_RUNTIME_ENV_H_

#include <string>
#include <vector>

namespace reg {

class Bin;

class RuntimeEnv {
 public:
  RuntimeEnv();
  int InstallBin(const std::string name, const std::string version);

  static int Child(void *data);
  int Start();
  
  std::string id() {return id_;}
  std::string lower_dir() { return envroot_ + "/lower"; }
  std::string upper_dir() { return envroot_ + "/upper"; }
  std::string work_dir() { return envroot_ + "/work"; }
  std::string mount_dir() { return envroot_ + "/mount"; }
  std::string src_dir() { return envroot_ + "/src"; }
  
  void set_id(const std::string id) {id_ = id;}
  void set_lower_dir(const std::string dir) {lower_dir_ = dir;}
  void set_upper_dir(const std::string dir) {upper_dir_ = dir;}
  void set_work_dir(const std::string dir) {work_dir_ = dir;}
  void set_mount_dir(const std::string dir) {mount_dir_ = dir;}
  void set_src_dir(const std::string dir) {src_dir_ = dir;}

  std::string cmd() {return cmd_;}
  void set_cmd(std::string cmd) {cmd_ = cmd;}


 private:
  std::string id_;
  std::string envroot_;
  std::string lower_dir_;
  std::string upper_dir_;
  std::string work_dir_;
  std::string mount_dir_;
  std::string src_dir_;

  std::vector<class Bin*> bins_;

  pid_t child_pid_;
  char child_stack_[8192];

  std::string cmd_;

};
}

#endif
