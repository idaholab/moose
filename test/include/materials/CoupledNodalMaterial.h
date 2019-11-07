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
 * Material for testing coupling of nodal values
 */
class CoupledNodalMaterial : public Material
{
public:
  static InputParameters validParams();

  CoupledNodalMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// The time level of the coupled variable
  MooseEnum _lag;
  /// Values of the coupled variable
  const Real & _coupled_val;
};
