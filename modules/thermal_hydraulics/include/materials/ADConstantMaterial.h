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
 * Constant material with zero-valued derivatives
 */
class ADConstantMaterial : public Material
{
public:
  ADConstantMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  const Real & _value;

  const MaterialPropertyName _property_name;

  ADMaterialProperty<Real> & _property;

public:
  static InputParameters validParams();
};
