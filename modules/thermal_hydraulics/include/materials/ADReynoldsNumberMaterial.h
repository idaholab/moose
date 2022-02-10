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

class SinglePhaseFluidProperties;

/**
 * Computes Reynolds number as a material property
 */
class ADReynoldsNumberMaterial : public Material
{
public:
  ADReynoldsNumberMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// Reynolds number property name
  const MaterialPropertyName & _Re_name;

  /// Density of the phase
  const ADMaterialProperty<Real> & _rho;

  /// Velocity of the phase
  const ADMaterialProperty<Real> & _vel;

  /// Hydraulic diameter
  const ADMaterialProperty<Real> & _D_h;

  /// Dynamic viscosity of the phase
  const ADMaterialProperty<Real> & _mu;

  /// Reynolds
  ADMaterialProperty<Real> & _Re;

public:
  static InputParameters validParams();
};
