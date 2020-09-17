//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVFluxBC.h"

class FVMomPressureFunctionBC : public FVFluxBC
{
public:
  static InputParameters validParams();
  FVMomPressureFunctionBC(const InputParameters & params);

protected:
  ADReal computeQpResidual() override;

  /// index x|y|z
  unsigned int _index;

  const ADVariableValue & _pressure;
  const Function & _pressure_exact_solution;
};
