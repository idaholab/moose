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
 * Set a material property to the norm of the gradient of a non-linear variable
 */
class VariableGradientMaterial : public Material
{
public:
  static InputParameters validParams();

  VariableGradientMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  const VariableGradient & _grad;
  MaterialProperty<Real> & _prop;
};
