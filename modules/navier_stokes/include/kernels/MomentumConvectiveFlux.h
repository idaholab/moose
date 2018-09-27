//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MOMENTUMCONVECTIVEFLUX_H
#define MOMENTUMCONVECTIVEFLUX_H

#include "Kernel.h"

class MomentumConvectiveFlux;

template <>
InputParameters validParams<MomentumConvectiveFlux>();

/**
 * Momentum convective flux: \f$\rho \vec u \vec u : \nabla v \f$
 *
 */
class MomentumConvectiveFlux : public Kernel
{
public:
  MomentumConvectiveFlux(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  // coupled variables
  const VariableValue & _vel_x;
  const VariableValue & _vel_y;
  const VariableValue & _vel_z;
};

#endif /* MOMENTUMCONVECTIVEFLUX_H */
