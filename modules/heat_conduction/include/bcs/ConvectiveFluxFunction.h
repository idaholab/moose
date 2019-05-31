//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IntegratedBC.h"

class ConvectiveFluxFunction : public IntegratedBC
{
public:
  ConvectiveFluxFunction(const InputParameters & parameters);
  virtual ~ConvectiveFluxFunction() {}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  const Function & _T_infinity;
  const Real _coefficient;
  const Function * const _coef_func;
};

template <>
InputParameters validParams<ConvectiveFluxFunction>();
