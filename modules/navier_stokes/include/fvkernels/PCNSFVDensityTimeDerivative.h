//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVQpTimeKernel.h"

class PCNSFVDensityTimeDerivative : public FVQpTimeKernel
{
public:
  static InputParameters validParams();
  PCNSFVDensityTimeDerivative(const InputParameters & parameters);

protected:
  ADReal computeQpResidual() override;

  /// The porosity
  const MaterialProperty<Real> & _eps;
  const ADVariableValue & _rho_dot;
  const ADVariableValue & _rho;
};
