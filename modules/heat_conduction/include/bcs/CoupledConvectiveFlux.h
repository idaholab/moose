//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COUPLEDCONVECTIVEFLUX_H
#define COUPLEDCONVECTIVEFLUX_H

#include "IntegratedBC.h"

class CoupledConvectiveFlux : public IntegratedBC
{
public:
  CoupledConvectiveFlux(const InputParameters & parameters);
  virtual ~CoupledConvectiveFlux() {}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  const VariableValue & _T_infinity;
  const Real _coefficient;
};

template <>
InputParameters validParams<CoupledConvectiveFlux>();

#endif // COUPLEDCONVECTIVEFLUX_H
