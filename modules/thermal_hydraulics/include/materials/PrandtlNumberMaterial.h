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
 * Computes Prandtl number as material property
 */
class PrandtlNumberMaterial : public Material
{
public:
  PrandtlNumberMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// Prandtl number
  MaterialProperty<Real> & _Pr;
  /// Constant-pressure specific heat
  const MaterialProperty<Real> & _cp;
  /// Dynamic viscosity
  const MaterialProperty<Real> & _mu;
  /// Thermal conductivity
  const MaterialProperty<Real> & _k;

public:
  static InputParameters validParams();
};
