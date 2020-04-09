//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowAdvectiveFluxCalculatorSaturatedMultiComponent.h"

/**
 * Computes the advective flux of fluid of given phase and component.
 * Hence this UserObject is relevant to multi-phase, multi-component situations.
 * Explicitly, the UserObject computes
 * (mass_fraction * density * relative_permeability / viscosity) * (- permeability * (grad(P) -
 * density * gravity))
 */
class PorousFlowAdvectiveFluxCalculatorUnsaturatedMultiComponent
  : public PorousFlowAdvectiveFluxCalculatorSaturatedMultiComponent
{
public:
  static InputParameters validParams();

  PorousFlowAdvectiveFluxCalculatorUnsaturatedMultiComponent(const InputParameters & parameters);

protected:
  virtual Real computeU(unsigned i) const override;

  virtual Real computedU_dvar(unsigned i, unsigned pvar) const override;

  /// Relative permeability of each phase
  const MaterialProperty<std::vector<Real>> & _relative_permeability;

  /// Derivative of relative permeability of each phase wrt PorousFlow variables
  const MaterialProperty<std::vector<std::vector<Real>>> & _drelative_permeability_dvar;
};
