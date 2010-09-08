#ifndef MMSREACTION_H
#define MMSREACTION_H

#include "Kernel.h"

class MMSReaction;

template<>
InputParameters validParams<MMSReaction>();

class MMSReaction : public Kernel
{
public:

  MMSReaction(const std::string & name, MooseSystem & moose_system, InputParameters parameters);
           
protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

};
#endif //MMSREACTION_H
