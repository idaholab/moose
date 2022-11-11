//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FluidPressureTimeDerivative.h"

// The transient term of the porous-media mass conservation equation
class PMFluidPressureTimeDerivative : public FluidPressureTimeDerivative
{
public:
  static InputParameters validParams();

  PMFluidPressureTimeDerivative(const InputParameters & parameters);
  virtual ~PMFluidPressureTimeDerivative() {}

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  // porosity
  const VariableValue & _porosity;
};
