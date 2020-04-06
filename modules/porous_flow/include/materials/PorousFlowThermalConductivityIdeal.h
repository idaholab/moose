//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowThermalConductivityBase.h"

/**
 * This material computes thermal conductivity for a PorousMedium - fluid
 * system, by using
 * Thermal conductivity = dry_thermal_conductivity + S^exponent * (wet_thermal_conductivity -
 * dry_thermal_conductivity),
 * where S is the aqueous saturation.
 */
class PorousFlowThermalConductivityIdeal : public PorousFlowThermalConductivityBase
{
public:
  static InputParameters validParams();

  PorousFlowThermalConductivityIdeal(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// Dry thermal conductivity of rock
  const RealTensorValue _la_dry;

  /// Whether _la_wet has been supplied
  const bool _wet_and_dry_differ;

  /// Wet thermal conductivity of rock
  const RealTensorValue _la_wet;

  /// Exponent for saturation
  const Real _exponent;

  /// Whether this is a fluid simulation
  const bool _aqueous_phase;

  /// Phase number of the aqueous phase
  const unsigned _aqueous_phase_number;

  /// Saturation of the fluid phases at the quadpoints
  const MaterialProperty<std::vector<Real>> * const _saturation_qp;

  /// d(Saturation)/d(PorousFlow variable)
  const MaterialProperty<std::vector<std::vector<Real>>> * const _dsaturation_qp_dvar;
};
