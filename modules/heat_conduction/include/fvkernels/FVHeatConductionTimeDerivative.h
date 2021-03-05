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

class FVHeatConductionTimeDerivative : public FVTimeKernel
{
public:
  static InputParameters validParams();

  FVHeatConductionTimeDerivative(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

  /// Specific heat material property
  const ADMaterialProperty<Real> & _specific_heat;

  /// Density material property
  const ADMaterialProperty<Real> & _density;
};
