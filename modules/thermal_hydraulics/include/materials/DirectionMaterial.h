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
 * Computes the directional vector of 1D elements in 3D space
 */
class DirectionMaterial : public Material
{
public:
  DirectionMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// The direction of the geometry (1D elements in 3D space)
  MaterialProperty<RealVectorValue> & _dir;

public:
  static InputParameters validParams();
};
