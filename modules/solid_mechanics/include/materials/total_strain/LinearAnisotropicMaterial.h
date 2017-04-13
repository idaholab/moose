/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
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
