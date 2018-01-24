//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#ifndef RANDOMMATERIAL_H
#define RANDOMMATERIAL_H

#include "Material.h"

class RandomMaterial;

template <>
InputParameters validParams<RandomMaterial>();

class RandomMaterial : public Material
{
public:
  RandomMaterial(const InputParameters & parameters);
  virtual void computeQpProperties();

protected:
  MaterialProperty<Real> & _rand_real;
  MaterialProperty<unsigned long> & _rand_long;
};

#endif // RANDOMMATERIAL_H
