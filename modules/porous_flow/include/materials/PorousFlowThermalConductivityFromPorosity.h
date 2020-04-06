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
 * This Material calculates rock-fluid combined thermal conductivity
 * for the single phase, fully saturated case by using a linear
 * weighted average.
 * Thermal conductivity = phi * lambda_f + (1 - phi) * lambda_s,
 * where phi is porosity, and lambda_f, lambda_s are
 * thermal conductivities of the fluid and solid (assumed constant)
 */
class PorousFlowThermalConductivityFromPorosity : public PorousFlowThermalConductivityBase
{
public:
  static InputParameters validParams();

  PorousFlowThermalConductivityFromPorosity(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// Thermal conductivity of the solid phase
  const RealTensorValue _la_s;

  /// Thermal conductivity of the single fluid phase
  const RealTensorValue _la_f;

  /// Quadpoint porosity
  const MaterialProperty<Real> & _porosity_qp;

  /// d(quadpoint porosity)/d(PorousFlow variable)
  const MaterialProperty<std::vector<Real>> & _dporosity_qp_dvar;
};
