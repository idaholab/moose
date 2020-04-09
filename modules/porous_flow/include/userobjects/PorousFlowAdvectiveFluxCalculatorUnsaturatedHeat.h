//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowAdvectiveFluxCalculatorSaturatedHeat.h"

/**
 * Computes the advective flux of heat energy in a given phase, assuming unsaturated conditions.
 * Hence this UserObject is only relevant to single-phase situations, or multi-phase situations
 * where each fluid component appears in one phase only.
 * Explicitly, the UserObject computes
 * (density * enthalpy * relative_permeability / viscosity) * (- permeability * (grad(P) - density *
 * gravity))
 */
class PorousFlowAdvectiveFluxCalculatorUnsaturatedHeat
  : public PorousFlowAdvectiveFluxCalculatorSaturatedHeat
{
public:
  static InputParameters validParams();

  PorousFlowAdvectiveFluxCalculatorUnsaturatedHeat(const InputParameters & parameters);

protected:
  virtual Real computeU(unsigned i) const override;

  virtual Real computedU_dvar(unsigned i, unsigned pvar) const override;

  /// Relative permeability of each phase
  const MaterialProperty<std::vector<Real>> & _relative_permeability;

  /// Derivative of relative permeability of each phase wrt PorousFlow variables
  const MaterialProperty<std::vector<std::vector<Real>>> & _drelative_permeability_dvar;
};
