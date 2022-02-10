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

class Function;

/**
 * Converts Darcy friction factor function into material property
 */
class ADWallFrictionFunctionMaterial : public Material
{
public:
  ADWallFrictionFunctionMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  const Function & _function;

  const MaterialPropertyName _f_D_name;
  ADMaterialProperty<Real> & _f_D;

public:
  static InputParameters validParams();
};
