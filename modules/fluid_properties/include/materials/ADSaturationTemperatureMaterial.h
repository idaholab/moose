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
 * Computes saturation temperature at some pressure
 */
class ADSaturationTemperatureMaterial : public Material
{
public:
  static InputParameters validParams();

  ADSaturationTemperatureMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// Pressure
  const ADMaterialProperty<Real> & _p;
  /// Saturation temperature material property name
  const MaterialPropertyName & _T_sat_name;
  /// Saturation temperature
  ADMaterialProperty<Real> & _T_sat;

  /// 2-phase fluid properties object
  const TwoPhaseFluidProperties & _fp_2phase;
};
