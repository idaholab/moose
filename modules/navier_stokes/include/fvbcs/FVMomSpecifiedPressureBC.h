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

/**
 * BC for the boundary pressure term from
 * the momentum equation with specified pressure
 */
class CNSFVMomSpecifiedPressureBC : public FVFluxBC
{
public:
  CNSFVMomSpecifiedPressureBC(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  virtual ADReal computeQpResidual() override;

  /// porosity
  // const VariableValue & _eps;
  const Real _eps;

  /// specified pressure
  const PostprocessorValue & _pressure;

  /// index x|y|z
  unsigned int _index;
};
