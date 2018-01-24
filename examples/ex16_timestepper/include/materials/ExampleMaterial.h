//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Material.h"

#ifndef EXAMPLEMATERIAL_H
#define EXAMPLEMATERIAL_H

// Forward Declarations
class ExampleMaterial;

template <>
InputParameters validParams<ExampleMaterial>();

/**
 * Example material class that defines a few properties.
 */
class ExampleMaterial : public Material
{
public:
  ExampleMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

private:
  Real _input_diffusivity;
  Real _input_time_coefficient;

  MaterialProperty<Real> & _diffusivity;
  MaterialProperty<Real> & _time_coefficient;
};

#endif // EXAMPLEMATERIAL_H
