#ifndef LINEARANISOTROPICMATERIAL_H
#define LINEARANISOTROPICMATERIAL_H

#include "LinearIsotropicMaterial.h"

//Forward Declarations
class LinearAnisotropicMaterial;
class ElasticityTensor;

template<>
InputParameters validParams<LinearAnisotropicMaterial>();

/**
 * LinearIsotropic material for use in simple applications that don't need material properties.
 */
class LinearAnisotropicMaterial : public LinearIsotropicMaterial
{
public:
  LinearAnisotropicMaterial(const std::string & name,
                          InputParameters parameters);
  
protected:

  Real _material_constant_c11;
  Real _material_constant_c12;
  Real _material_constant_c44;
  Real _euler_angle_1;
  Real _euler_angle_2;
  Real _euler_angle_3;

//  ElasticityTensor * _local_elasticity_tensor;
};

#endif //LINEARANISOTROPICMATERIAL_H
