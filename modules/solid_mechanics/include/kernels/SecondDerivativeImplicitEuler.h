/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef SECONDDERIVATIVEIMPLICITEULER_H
#define SECONDDERIVATIVEIMPLICITEULER_H

#include "TimeKernel.h"

// Forward Declarations
class SecondDerivativeImplicitEuler;

template <>
InputParameters validParams<SecondDerivativeImplicitEuler>();

class SecondDerivativeImplicitEuler : public TimeKernel
{
public:
  SecondDerivativeImplicitEuler(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  const VariableValue & _u_old;
  const VariableValue & _u_older;
};

#endif // SECONDDERIVATIVEIMPLICITEULER_H
