//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GapFluxModelConductionBase.h"

/**
 * Gap flux model for varying gap conductance using a coupled variable for temperature
 */
class GapFluxModelConduction : public GapFluxModelConductionBase
{
public:
  static InputParameters validParams();

  GapFluxModelConduction(const InputParameters & parameters);

  virtual ADReal computeFlux() const override;

protected:
  /// Primary surface temperature
  const ADVariableValue & _primary_T;
  /// Secondary surface temperature
  const ADVariableValue & _secondary_T;

  const Function * const _gap_conductivity_function;
  const VariableValue * const _gap_conductivity_function_variable;
};
