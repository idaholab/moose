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
 * Computes Fanning friction factor from Darcy friction factor
 */
class ADFanningFrictionFactorMaterial : public Material
{
public:
  ADFanningFrictionFactorMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// Darcy friction factor
  const ADMaterialProperty<Real> & _f_D;

  /// Fanning friction factor
  ADMaterialProperty<Real> & _f_F;

public:
  static InputParameters validParams();
};
