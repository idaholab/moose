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
 * Computes the advective flux of heat energy in the given phase, assuming fully-saturated
 * conditions. Hence this UserObject is only relevant to single-phase situations. Explicitly, the
 * UserObject computes (density * enthalpy / viscosity) * (- permeability * (grad(P) - density *
 * gravity))
 */
class PorousFlowAdvectiveFluxCalculatorSaturatedHeat
  : public PorousFlowAdvectiveFluxCalculatorSaturated
{
public:
  static InputParameters validParams();

  PorousFlowAdvectiveFluxCalculatorSaturatedHeat(const InputParameters & parameters);

protected:
  virtual Real computeU(unsigned i) const override;

  virtual Real computedU_dvar(unsigned i, unsigned pvar) const override;

  /// Enthalpy of each phase
  const MaterialProperty<std::vector<Real>> & _enthalpy;

  /// Derivative of enthalpy of each phase wrt PorousFlow variables
  const MaterialProperty<std::vector<std::vector<Real>>> & _denthalpy_dvar;
};
