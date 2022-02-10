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
class ConvectiveHeatTransferCoefficientMaterial : public Material
{
public:
  ConvectiveHeatTransferCoefficientMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// Wall heat transfer coefficient
  MaterialProperty<Real> & _Hw;
  /// Nusselt number
  const MaterialProperty<Real> & _Nu;
  /// Hydraulic diameter
  const MaterialProperty<Real> & _D_h;
  /// Thermal conductivity
  const MaterialProperty<Real> & _k;

public:
  static InputParameters validParams();
};
