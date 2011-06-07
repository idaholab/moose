#ifndef LINEARISOTROPICMATERIALRZ_H
#define LINEARISOTROPICMATERIALRZ_H

#include "SolidMechanicsMaterialRZ.h"

//Forward Declarations
class LinearIsotropicMaterialRZ;
class ElasticityTensor;

template<>
InputParameters validParams<LinearIsotropicMaterialRZ>();

/**
 * AxisymmetriclinearIsotropic material for use in simple applications that don't need material properties.
 */
class LinearIsotropicMaterialRZ : public SolidMechanicsMaterialRZ
{
public:
  LinearIsotropicMaterialRZ(const std::string & name,
                                      InputParameters parameters);

  virtual ~LinearIsotropicMaterialRZ();

protected:

  virtual void computeStress(const SymmTensor & total_strain,
                             const SymmTensor & strain,
                             const ElasticityTensor & elasticity_tensor,
                             SymmTensor & stress);

  /**
   * Will always be passed to full symmetric strain tensor.
   * What should come out is a modified strain tensor.
   */
  virtual void computeStrain(const SymmTensor & total_strain, SymmTensor & elastic_strain);

  Real _input_thermal_conductivity;

  Real _input_density;

};

#endif //LINEARISOTROPICMATERIALRZ_H
