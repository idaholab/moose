//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVElementalKernel.h"

class INSFVMomentumPressure : public FVElementalKernel
{
public:
  static InputParameters validParams();
  INSFVMomentumPressure(const InputParameters & params);

protected:
  ADReal computeQpResidual() override;

  /// The pressure variable
  const MooseVariableFVReal * const _p_var;

  /// index x|y|z
  const unsigned int _index;
};
