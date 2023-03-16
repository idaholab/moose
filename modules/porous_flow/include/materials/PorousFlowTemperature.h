//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowMaterial.h"

/**
 * Creates temperature Materials
 */
template <bool is_ad>
class PorousFlowTemperatureTempl : public PorousFlowMaterial
{
public:
  static InputParameters validParams();

  PorousFlowTemperatureTempl(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  /// Number of PorousFlow variables
  const unsigned int _num_pf_vars;

  /// Variable value of temperature at quadpoints or nodes
  const GenericVariableValue<is_ad> & _temperature_var;

  /// Gradient(_temperature at quadpoints)
  const VariableGradient * const _grad_temperature_var;

  /// Whether the temperature coupled variable is a PorousFlow variable
  const bool _temperature_is_PF;

  /// The PorousFlow variable number of the temperature
  const unsigned int _t_var_num;

  /// Computed temperature at quadpoints or nodes
  GenericMaterialProperty<Real, is_ad> & _temperature;

  /// d(computed temperature)/d(PorousFlow variable)
  MaterialProperty<std::vector<Real>> * const _dtemperature_dvar;

  /// Grad(temperature) at the quadpoints (not needed for nodal_materials)
  MaterialProperty<RealGradient> * const _grad_temperature;

  /// d(grad temperature)/d(grad PorousFlow variable) at the quadpoints
  MaterialProperty<std::vector<Real>> * const _dgrad_temperature_dgradv;

  /// d(grad temperature)/d(PorousFlow variable) at the quadpoints
  MaterialProperty<std::vector<RealGradient>> * const _dgrad_temperature_dv;
};

typedef PorousFlowTemperatureTempl<false> PorousFlowTemperature;
typedef PorousFlowTemperatureTempl<true> ADPorousFlowTemperature;
