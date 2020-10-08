//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"

class TwoPhaseFluidProperties;

/**
 * Computes saturation pressure at some temperature.
 */
template <bool is_ad>
class SaturationPressureMaterialTempl : public Material
{
public:
  static InputParameters validParams();

  SaturationPressureMaterialTempl(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// Temperature
  const GenericMaterialProperty<Real, is_ad> & _T;
  /// Saturation pressure material property name
  const MaterialPropertyName & _p_sat_name;
  /// Saturation pressure
  GenericMaterialProperty<Real, is_ad> & _p_sat;

  /// 2-phase fluid properties object
  const TwoPhaseFluidProperties & _fp_2phase;
};

typedef SaturationPressureMaterialTempl<false> SaturationPressureMaterial;
typedef SaturationPressureMaterialTempl<true> ADSaturationPressureMaterial;
