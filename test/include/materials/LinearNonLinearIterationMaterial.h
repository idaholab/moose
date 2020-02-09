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

// Forward Declarations
class LinearNonLinearIterationMaterial;

template <>
InputParameters validParams<LinearNonLinearIterationMaterial>();
/**
 * A material that tracks the number of times computeQpProperties has been called.
 */
class LinearNonLinearIterationMaterial : public Material
{
public:
  static InputParameters validParams();

  LinearNonLinearIterationMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;
  MaterialProperty<Real> & _mat_prop;
  const Real _prefactor;
};
