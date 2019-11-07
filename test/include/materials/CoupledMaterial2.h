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
 * A material that couples a material property
 */
class CoupledMaterial2 : public Material
{
public:
  static InputParameters validParams();

  CoupledMaterial2(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  MaterialPropertyName _mat_prop_name;
  MaterialProperty<Real> & _mat_prop;

  const MaterialProperty<Real> & _coupled_mat_prop;
  const MaterialProperty<Real> & _coupled_mat_prop2;
};
