//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TimeKernel.h"
#include "SinglePhaseFluidProperties.h"

/**
 * The transient term of the porous-media mass conservation equation
 */
class PINSFEFluidPressureTimeDerivative : public TimeKernel
{
public:
  static InputParameters validParams();

  PINSFEFluidPressureTimeDerivative(const InputParameters & parameters);
  virtual ~PINSFEFluidPressureTimeDerivative() {}

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  const VariableValue & _temperature;
  const VariableValue & _temperature_dot;
  const VariableValue & _d_temperaturedot_du;
  unsigned _temperature_var_number;
  const VariableValue & _porosity;

  const SinglePhaseFluidProperties & _eos;
};
