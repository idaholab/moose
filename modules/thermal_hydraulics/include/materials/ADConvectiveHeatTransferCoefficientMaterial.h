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

/**
 * Computes convective heat transfer coefficient from Nusselt number
 */
class ADConvectiveHeatTransferCoefficientMaterial : public Material
{
public:
  ADConvectiveHeatTransferCoefficientMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// Wall heat transfer coefficient
  ADMaterialProperty<Real> & _Hw;
  /// Nusselt number
  const ADMaterialProperty<Real> & _Nu;
  /// Hydraulic diameter
  const ADMaterialProperty<Real> & _D_h;
  /// Thermal conductivity
  const ADMaterialProperty<Real> & _k;

public:
  static InputParameters validParams();
};
