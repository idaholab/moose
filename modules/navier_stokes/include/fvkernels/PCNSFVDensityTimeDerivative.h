//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVTimeKernel.h"

class PCNSFVDensityTimeDerivative : public FVTimeKernel
{
public:
  static InputParameters validParams();
  PCNSFVDensityTimeDerivative(const InputParameters & parameters);

protected:
  ADReal computeQpResidual() override;

  /// The time derivative of the primary variable
  const ADVariableValue & _u_dot;

  /// The porosity
  const MaterialProperty<Real> & _eps;
  const ADVariableValue & _rho_dot;
  const ADVariableValue & _rho;
};
