//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef LINEARANISOTROPICMATERIAL_H
#define LINEARANISOTROPICMATERIAL_H

#include "LinearIsotropicMaterial.h"

// Forward Declarations
class LinearAnisotropicMaterial;
class ElasticityTensor;

template <>
InputParameters validParams<LinearAnisotropicMaterial>();

/**
 * LinearIsotropic material for use in simple applications that don't need material properties.
 */
class LinearAnisotropicMaterial : public LinearIsotropicMaterial
{
public:
  LinearAnisotropicMaterial(const InputParameters & parameters);

protected:
  Real _material_constant_c11;
  Real _material_constant_c12;
  Real _material_constant_c44;
  Real _euler_angle_1;
  Real _euler_angle_2;
  Real _euler_angle_3;
};

#endif // LINEARANISOTROPICMATERIAL_H
