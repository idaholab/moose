#ifndef PFCRFFKERNELACTION_H
#define PFCRFFKERNELACTION_H

#include "Action.h"

//Forward Declarations
class PFCRFFKernelAction;

template<>
InputParameters validParams<PFCRFFKernelAction>();

class PFCRFFKernelAction: public Action
{
public:
  PFCRFFKernelAction(const InputParameters & params);

  virtual void act();

private:
  unsigned int _num_L;
  std::string _L_name_base;
  std::string _n_name;
};

#endif //PFCRFFKERNELACTION_H
