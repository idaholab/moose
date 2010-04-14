#ifndef LINEARISOTROPICMATERIAL_H
#define LINEARISOTROPICMATERIAL_H

#include "SolidMechanicsMaterial.h"

//Forward Declarations
class LinearIsotropicMaterial;
class ElasticityTensor;

template<>
InputParameters validParams<LinearIsotropicMaterial>();

/**
 * LinearIsotropic material for use in simple applications that don't need material properties.
 */
class LinearIsotropicMaterial : public SolidMechanicsMaterial
{
public:
  LinearIsotropicMaterial(std::string name,
                          MooseSystem & moose_system,
                         InputParameters parameters);
  
protected:
  virtual void computeProperties();

  void computeStress(const RealVectorValue & x,
                     const RealVectorValue & y,
                     const RealVectorValue & z,
                     RealTensorValue & stress);

  Real _youngs_modulus;
  Real _poissons_ratio;

  ElasticityTensor * _local_elasticity_tensor;
};

#endif //LINEARISOTROPICMATERIAL_H
