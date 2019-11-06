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
 * Adds two material properties together
 */
class SumMaterial : public Material
{
public:
  static InputParameters validParams();

  SumMaterial(const InputParameters & parameters);
  virtual ~SumMaterial();

protected:
  virtual void computeQpProperties();

  std::string _sum_prop_name;
  std::string _mp1_prop_name;
  std::string _mp2_prop_name;

  MaterialProperty<Real> & _sum;
  const MaterialProperty<Real> & _mp1;
  const MaterialProperty<Real> & _mp2;

  Real _val_mp1;
  Real _val_mp2;
};
