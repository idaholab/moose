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
 * Computes T_wall from the constitutive model
 */
class ADTemperatureWall3EqnMaterial : public Material
{
public:
  ADTemperatureWall3EqnMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// Wall temperature
  ADMaterialProperty<Real> & _T_wall;
  /// Wall heat flux
  const ADMaterialProperty<Real> & _q_wall;
  /// Heat transfer coefficient
  const ADMaterialProperty<Real> & _Hw;
  /// Fluid temperature
  const ADMaterialProperty<Real> & _T;

public:
  static InputParameters validParams();
};
