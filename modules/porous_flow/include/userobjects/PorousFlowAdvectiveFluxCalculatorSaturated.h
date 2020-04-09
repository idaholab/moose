//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowAdvectiveFluxCalculatorBase.h"

/**
 * Computes the advective flux of fluid of given phase, assuming fully-saturated conditions.
 * Hence this UserObject is only relevant to single-phase situations.
 * Explicitly, this UserObject computes
 * (density / viscosity) * (- permeability * (grad(P) - density * gravity))
 */
class PorousFlowAdvectiveFluxCalculatorSaturated : public PorousFlowAdvectiveFluxCalculatorBase
{
public:
  static InputParameters validParams();

  PorousFlowAdvectiveFluxCalculatorSaturated(const InputParameters & parameters);

protected:
  virtual Real computeU(unsigned i) const override;
  virtual Real computedU_dvar(unsigned i, unsigned pvar) const override;

  /// Whether to multiply the flux by the fluid density
  const bool _multiply_by_density;

  /// Fluid density for each phase (at the node)
  const MaterialProperty<std::vector<Real>> * const _fluid_density_node;

  /// Derivative of the fluid density for each phase wrt PorousFlow variables (at the node)
  const MaterialProperty<std::vector<std::vector<Real>>> * const _dfluid_density_node_dvar;

  /// Viscosity of each component in each phase
  const MaterialProperty<std::vector<Real>> & _fluid_viscosity;

  /// Derivative of the fluid viscosity for each phase wrt PorousFlow variables
  const MaterialProperty<std::vector<std::vector<Real>>> & _dfluid_viscosity_dvar;
};
