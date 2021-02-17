//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVMomentumPressure.h"

class FVPorosityMomentumPressure : public FVMomentumPressure
{
public:
  static InputParameters validParams();
  FVPorosityMomentumPressure(const InputParameters & params);

protected:
  ADReal computeQpResidual() override;

  /// The porosity
  const MaterialProperty<Real> & _eps;
};
