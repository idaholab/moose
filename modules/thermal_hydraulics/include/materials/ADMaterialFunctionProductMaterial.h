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
 * Computes the product of a material property and a function.
 */
class ADMaterialFunctionProductMaterial : public Material
{
public:
  ADMaterialFunctionProductMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// Product
  ADMaterialProperty<Real> & _product;
  /// Scale
  const ADMaterialProperty<Real> & _scale;
  /// Function
  const Function & _function;

public:
  static InputParameters validParams();
};
