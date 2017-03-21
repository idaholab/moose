/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef NSGRAVITYPOWER_H
#define NSGRAVITYPOWER_H

#include "Kernel.h"

// Forward Declarations
class NSGravityPower;

template <>
InputParameters validParams<NSGravityPower>();

class NSGravityPower : public Kernel
{
public:
  NSGravityPower(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  unsigned int _momentum_var;
  const VariableValue & _momentum;

  const Real _acceleration;
};

#endif // NSGRAVITYPOWER_H
