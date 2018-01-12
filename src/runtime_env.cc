#include "runtime_env.h"
#include <unistd.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <fstream>
#include <iostream>
#include <cstring>
#include <yaml-cpp/yaml.h>

namespace reg {

RuntimeEnv::RuntimeEnv() {
  envroot_ = ".reg";
}

int RuntimeEnv::InstallBin(const std::string name, const std::string version) {
  std::string oldName = "/var/lib/dygre/bins/" + name + "/" + version + "/" + name;
  std::string newName = "./lower/bin/" + name;

  int result;
  result = link(oldName.c_str(), newName.c_str());
  std::cout << "linking: " + oldName << std::endl;
  if (result != 0) {
    std::cout << "create bin link error." << oldName << std::endl;
    return -1;
  }

  YAML::Node config = YAML::LoadFile(("/var/lib/dygre/bins/" + name + "/" + version + "/" + "dygre.yaml").c_str());
  YAML::Node binlibs = config["libs"];
  for (YAML::const_iterator itx = binlibs.begin(); itx != binlibs.end(); ++itx) {
    std::string n = itx->first.as<std::string>();
    std::string v = itx->second.as<std::string>();
    oldName = "/var/lib/dygre/libs/" + n + "/" + n + ".so." + v;

    newName = "/var/lib/reg/runtimes/" + this->id_ + "/lower/lib/" + n + ".so." + v;
    std::cout << oldName << std::endl;
    
    std::fstream _file;
    _file.open(newName, std::ios::in);
    if (!_file) {
      result = link(oldName.c_str(), newName.c_str());
      if (result != 0) {
        std::cout << "create link error." << newName << std::endl;
        return -1;
      }
    }
    
  }
  
  YAML::Node binbins = config["bins"];
  //std::cout << binbins.size() << "ok" << std::endl;
  if (config["bins"]) {
    YAML::Node binbins = config["bins"];
    std::cout << "hello" << std::endl;
    for (YAML::const_iterator it = binbins.begin(); it != binbins.end(); it++) {
      this->InstallBin(it->first.as<std::string>(), it->second.as<std::string>());
    }
  }
  
  return 0;
}

int RuntimeEnv::Child(void *data) {
  RuntimeEnv *re = (RuntimeEnv*)data;

  mount("proc", "/proc", "proc", 0, NULL);
  mount("none", "/dev", "tmpfs", MS_NOEXEC | MS_STRICTATIME, NULL);

  mkdir((re->mount_dir() + "/pivot").c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  int result = syscall(SYS_pivot_root, re->mount_dir().c_str(), (re->mount_dir() + "/pivot").c_str());
  if (result != 0) {
    std::cout << "pivot error " << result << std::endl;
    return -1;
  }

  result = umount2("/pivot", MNT_DETACH);
  rmdir("/pivot");

  chdir("/src");

  /**
   * parse cmd line
   **/
  int argnum = 1, i = 0;
  for (i; re->cmd().c_str()[i] != '\0'; i++) {
    if (re->cmd().c_str()[i] == ' ')
      argnum++;
  }

  char split[] = " ";
        
  char arglist[argnum + 1][128];

  char *substr = strtok((char*)(re->cmd().c_str()), split);
  i = 0;
  while (substr != NULL) {
    strcpy(arglist[i], substr);
    i++;
    substr = strtok(NULL, split);
  }
  if (strcmp(arglist[0], "bash") == 0) {
    execlp("/bin/bash", "bash", NULL);
    return 0;
  }
  arglist[argnum][0] = 0;
  char *al[] = {arglist[0], arglist[1], 0};
  execvp(al[0], (char* const*)al);
  
  return 0;
}

int RuntimeEnv::Start() {
  std::string args = "lowerdir=" + lower_dir() + ",upperdir=" + upper_dir() + ",workdir=" + work_dir();

  int result = mount(NULL, mount_dir().c_str(), "overlay", 0, args.c_str());
  if (result != 0) {
    std::cout << "mount error." << std::endl;
    return -1;
  }
  mkdir((mount_dir() + "/src").c_str(), 0755);
  mount(src_dir().c_str(), (mount_dir() + "/src").c_str(), "bind", MS_BIND, NULL);
  child_pid_ = clone(Child, child_stack_ + 8192, CLONE_NEWPID | CLONE_NEWNS | CLONE_NEWIPC | SIGCHLD, this);
  waitpid(child_pid_, NULL, 0);
  return 0;
}


}
