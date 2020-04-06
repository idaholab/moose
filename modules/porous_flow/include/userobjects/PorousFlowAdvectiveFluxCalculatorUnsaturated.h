//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowAdvectiveFluxCalculatorSaturated.h"

/**
 * Computes the advective flux of fluid of given phase, assuming unsaturated conditions.
 * Hence this UserObject is only relevant to single-phase situations, or multi-phase
 * situations where each fluid component appears in one phase only.
 * Explicitly, the UserObject computes
 * (density * relative_permeability / viscosity) * (- permeability * (grad(P) - density * gravity))
 */
class PorousFlowAdvectiveFluxCalculatorUnsaturated
  : public PorousFlowAdvectiveFluxCalculatorSaturated
{
public:
  static InputParameters validParams();

  PorousFlowAdvectiveFluxCalculatorUnsaturated(const InputParameters & parameters);

protected:
  virtual Real computeU(unsigned i) const override;

  virtual Real computedU_dvar(unsigned i, unsigned pvar) const override;

  /// Relative permeability of each phase
  const MaterialProperty<std::vector<Real>> & _relative_permeability;

  /// Derivative of relative permeability of each phase wrt PorousFlow variables
  const MaterialProperty<std::vector<std::vector<Real>>> & _drelative_permeability_dvar;
};
