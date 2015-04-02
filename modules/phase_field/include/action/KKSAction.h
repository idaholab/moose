#ifndef KKSACTION_H
#define KKSACTION_H

#include "InputParameters.h"
#include "Action.h"

class KKSAction;

template<>
InputParameters validParams<KKSAction>();

/**
 * Automatically generates all variables and kernels to set up a KKS phase field simulation
 */
class KKSAction: public Action
{
public:
  KKSAction(const std::string & name, InputParameters params);
  virtual void act();

private:
  std::string _c_name_base;
};

#endif //KKSACTION_H
