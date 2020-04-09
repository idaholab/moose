//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowSink.h"

/**
 * Provides either a porepressure or a temperature
 * to derived classes, depending on _involves_fluid
 * defined in PorousFlowSink
 */
class PorousFlowSinkPTDefiner : public PorousFlowSink
{
public:
  static InputParameters validParams();

  PorousFlowSinkPTDefiner(const InputParameters & parameters);

protected:
  /// Nodal pore pressure in each phase
  const MaterialProperty<std::vector<Real>> * const _pp;

  /// d(Nodal pore pressure in each phase)/d(PorousFlow variable)
  const MaterialProperty<std::vector<std::vector<Real>>> * const _dpp_dvar;

  /// Nodal temperature
  const MaterialProperty<Real> * const _temp;

  /// d(Nodal temperature)/d(PorousFlow variable)
  const MaterialProperty<std::vector<Real>> * const _dtemp_dvar;

  /// Subtract this from porepressure or temperature before evaluating PiecewiseLinearSink, HalfCubicSink, etc
  const VariableValue & _pt_shift;

  /// Provides the variable value (either porepressure, or temperature, depending on _involves_fluid)
  virtual Real ptVar() const;

  /// Provides the d(variable)/(d PorousFlow Variable pvar)
  virtual Real dptVar(unsigned pvar) const;
};
