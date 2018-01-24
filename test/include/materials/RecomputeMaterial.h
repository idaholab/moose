//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef RECOMPUTEMATERIAL_H
#define RECOMPUTEMATERIAL_H

// Moose includes
#include "Material.h"

// Forward declarations
class RecomputeMaterial;

template <>
InputParameters validParams<RecomputeMaterial>();

/**
 * A test material for testing the ability for properties to be recomputed
 *
 * @see NewtonMaterial
 */
class RecomputeMaterial : public Material
{
public:
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

#endif /* RECOMPUTEMATERIAL_H */
