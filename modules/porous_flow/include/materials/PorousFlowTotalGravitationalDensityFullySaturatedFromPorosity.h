//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowTotalGravitationalDensityBase.h"

/**
 * Material designed to provide the density of the porous medium for the
 * fully-saturated case. Density is calculated as a
 * weighted average of the fluid and solid densities:
 * density = phi * rho_f + (1 - phi) * rho_s
 * where phi is porosity, rho_f is fluid density and rho_s is solid
 * density (assumed constant).
 */
class PorousFlowTotalGravitationalDensityFullySaturatedFromPorosity
  : public PorousFlowTotalGravitationalDensityBase
{
public:
  static InputParameters validParams();

  PorousFlowTotalGravitationalDensityFullySaturatedFromPorosity(const InputParameters & parameters);

protected:
  void initQpStatefulProperties() override;
  void computeQpProperties() override;

  /// Solid density
  const Real _rho_s;

  /// Fluid density at qps
  const MaterialProperty<std::vector<Real>> & _rho_f_qp;

  /// Porosity at qps
  const MaterialProperty<Real> & _porosity_qp;

  /// d(rho_f)/d(PorousFlow variable)
  const MaterialProperty<std::vector<std::vector<Real>>> & _drho_f_qp_dvar;

  /// d(porosity)/d(PorousFlow variable)
  const MaterialProperty<std::vector<Real>> & _dporosity_qp_dvar;
};
