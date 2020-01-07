//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Moose includes
#include "Material.h"

/**
 * A test material for testing the ability for properties to be recomputed
 *
 * @see NewtonMaterial
 */
class RecomputeMaterial : public Material
{
public:
  static InputParameters validParams();

  RecomputeMaterial(const InputParameters & parameters);

protected:
  void computeQpProperties();
  void resetQpProperties();

private:
  MaterialProperty<Real> & _f;
  MaterialProperty<Real> & _f_prime;
  const MaterialProperty<Real> & _p;
  const Real & _constant;
};
