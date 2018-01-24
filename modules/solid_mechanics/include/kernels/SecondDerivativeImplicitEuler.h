//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
