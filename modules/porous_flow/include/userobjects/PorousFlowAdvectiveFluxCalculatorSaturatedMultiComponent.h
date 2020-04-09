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
 * Computes the advective flux of fluid of given phase and fluid component.
 * Explicitly, the UserObject computes
 * (mass_fraction * density / viscosity) * (- permeability * (grad(P) - density * gravity))
 */
class PorousFlowAdvectiveFluxCalculatorSaturatedMultiComponent
  : public PorousFlowAdvectiveFluxCalculatorSaturated
{
public:
  static InputParameters validParams();

  PorousFlowAdvectiveFluxCalculatorSaturatedMultiComponent(const InputParameters & parameters);

protected:
  virtual Real computeU(unsigned i) const override;

  virtual Real computedU_dvar(unsigned i, unsigned pvar) const override;

  /// The fluid component
  const unsigned int _fluid_component;

  /// Mass fraction of each component in each phase
  const MaterialProperty<std::vector<std::vector<Real>>> & _mass_fractions;

  /// Derivative of the mass fraction of each component in each phase wrt PorousFlow variables
  const MaterialProperty<std::vector<std::vector<std::vector<Real>>>> & _dmass_fractions_dvar;
};
