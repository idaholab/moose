//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MMSIMPLICITEULER_H_
#define MMSIMPLICITEULER_H_

#include "TimeKernel.h"

class MMSImplicitEuler;

template <>
InputParameters validParams<MMSImplicitEuler>();

class MMSImplicitEuler : public TimeKernel
{
public:
  MMSImplicitEuler(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  const VariableValue & _u_old;
};

#endif // MMSIMPLICITEULER_H_
