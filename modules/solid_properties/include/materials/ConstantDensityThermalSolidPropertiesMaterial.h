//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ThermalSolidPropertiesMaterial.h"

/**
 * Computes solid thermal properties as a function of temperature but with a constant density.
 */
template <bool is_ad>
class ConstantDensityThermalSolidPropertiesMaterialTempl
  : public ThermalSolidPropertiesMaterialTempl<is_ad>
{
public:
  static InputParameters validParams();

  ConstantDensityThermalSolidPropertiesMaterialTempl(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// Reference temperature for constant density
  const Real & _T_ref;

  /// Constant density
  const Real _rho_constant;

  using ThermalSolidPropertiesMaterialTempl<is_ad>::_temperature;
  using ThermalSolidPropertiesMaterialTempl<is_ad>::_cp;
  using ThermalSolidPropertiesMaterialTempl<is_ad>::_k;
  using ThermalSolidPropertiesMaterialTempl<is_ad>::_rho;
  using ThermalSolidPropertiesMaterialTempl<is_ad>::_sp;
  using ThermalSolidPropertiesMaterialTempl<is_ad>::_qp;
};

typedef ConstantDensityThermalSolidPropertiesMaterialTempl<false>
    ConstantDensityThermalSolidPropertiesMaterial;
typedef ConstantDensityThermalSolidPropertiesMaterialTempl<true>
    ADConstantDensityThermalSolidPropertiesMaterial;
