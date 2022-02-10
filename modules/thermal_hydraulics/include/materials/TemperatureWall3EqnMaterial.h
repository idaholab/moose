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
class TemperatureWall3EqnMaterial : public Material
{
public:
  TemperatureWall3EqnMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// Wall temperature
  MaterialProperty<Real> & _T_wall;
  /// Wall heat flux
  const MaterialProperty<Real> & _q_wall;
  /// Heat transfer coefficient
  const MaterialProperty<Real> & _Hw;
  /// Fluid temperature
  const MaterialProperty<Real> & _T;

public:
  static InputParameters validParams();
};
