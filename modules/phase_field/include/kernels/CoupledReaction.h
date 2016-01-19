/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COUPLEDREACTION_H
#define COUPLEDREACTION_H

#include "Kernel.h"

// Forward Declaration
class CoupledReaction;

template<>
InputParameters validParams<CoupledReaction>();

class CoupledReaction : public Kernel
{
public:
  CoupledReaction(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  VariableValue & _v;
  unsigned int _v_var;
};

#endif //COUPLEDREACTION_H
