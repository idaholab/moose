#ifndef MMSREACTION_H_
#define MMSREACTION_H_

#include "Kernel.h"

class MMSReaction;

template<>
InputParameters validParams<MMSReaction>();

class MMSReaction : public Kernel
{
public:
  MMSReaction(const std::string & name, InputParameters parameters);
           
protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
};

#endif //MMSREACTION_H_
