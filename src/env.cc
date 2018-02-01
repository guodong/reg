#include "env.h"
#include <unistd.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <string.h>
#include <iostream>

namespace reg {

int Env::AddDependency(reg::Env *env) {
  int exist = 0;
  
  for (std::vector<Env*>::const_iterator iter = dependencies_.begin(); iter != dependencies_.end(); iter++) {
    if (env == *iter) {
      exist = 1;
      break;
    }
  }

  if (exist) {
  //if (0) {
    return 1;
  } else {
    dependencies_.push_back(env);
  }

  return 0;
}

//static void signal_handler(int s) {

//}

int Env::Child(void *data) {
  Env *re = (Env*)data;
  int result;

  //syscall(SYS_unshare, CLONE_NEWNS);
  //mount("proc", "/proc", "proc", 0, NULL);

  // this may lead to host system issue
  //mount("none", "/dev", "tmpfs", MS_NOEXEC | MS_STRICTATIME, NULL);

  result = chroot(re->mount_dir().c_str());
  if (result != 0) {
    std::cout << "change root error " << strerror(errno) << std::endl;
    return -1;
  }

  chdir("/");
  for (int i = 0; i < re->cmds().size(); ++i) {
    std::string cmd = re->cmds().at(i);
  
    char *arg[2];
    arg[0] = (char*)(cmd.c_str());
    arg[1] = NULL;

    execvp(arg[0], arg);
  }
  
  return 0;
}

int Env::Start() {
  // bind user workdir to env /src
  mkdir((mount_dir() + "/src").c_str(), 0755);
  mount("./", (mount_dir() + "/src").c_str(), "bind", MS_BIND, NULL);
  
  child_pid_ = clone(Child, child_stack_ + 8192, CLONE_NEWPID | CLONE_NEWNS | CLONE_NEWIPC | SIGCHLD, this);
  waitpid(child_pid_, NULL, 0);
  
  return 0;
}


}
