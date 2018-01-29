#include "env.h"

int Env::AddDependency(reg::Env *env) {
  int exist = 0;
  
  for (std::vector<Env*>::const_iterator iter = dependencies_.begin(); iter != dependencies_.end(); iter++) {
    if (env == *iter) {
      exist = 1;
      break;
    }
  }

  if (exist) {
    return 1;
  } else {
    dependencies_.push_back(env);
  }

  return 0;
}
