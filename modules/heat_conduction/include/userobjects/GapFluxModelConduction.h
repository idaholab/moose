//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GapFluxModelBase.h"

/**
 * Base class for gap flux models used by ModularGapConductanceConstraint
 */
class GapFluxModelConduction : public GapFluxModelBase
{
public:
  static InputParameters validParams();

  GapFluxModelConduction(const InputParameters & parameters);

  virtual ADReal computeFlux() const override;

  virtual ADReal gapAttenuation() const;

protected:
  /// Primary surface temperature
  const ADVariableValue & _primary_T;
  /// Secondary surface temperature
  const ADVariableValue & _secondary_T;

  /// Gap conductivity constant
  const Real _gap_conductivity;

  const Function * const _gap_conductivity_function;
  const VariableValue * const _gap_conductivity_function_variable;

  const Real _min_gap;

  const unsigned int _min_gap_order;
};
