#ifndef POLYREACTION_H
#define POLYREACTION_H

#include "Kernel.h"

class PolyReaction;

template<>
InputParameters validParams<PolyReaction>();

class PolyReaction : public Kernel
{
public:

  PolyReaction(std::string name, MooseSystem & moose_system, InputParameters parameters);
           
protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

};
#endif //POLYREACTION_H
