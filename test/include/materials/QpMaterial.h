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
#include "MaterialProperty.h"

/**
 * Material with a single property that corresponds to the quadrature
 * point index.  Used to ensure that the constant_on_elem flag
 * actually works correctly (the MaterialProperty should output as
 * identically zero when the flag is turned on).
 */
class QpMaterial : public Material
{
public:
  static InputParameters validParams();

  QpMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  const std::string _prop_name;
  MaterialProperty<Real> & _mat_prop;
};
