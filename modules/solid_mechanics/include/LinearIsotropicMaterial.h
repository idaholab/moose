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
  LinearIsotropicMaterial(const std::string & name,
                          MooseSystem & moose_system,
                          InputParameters parameters);
  
protected:
  virtual void computeProperties();

  virtual void computeStress(const RealVectorValue & x,
                     const RealVectorValue & y,
                     const RealVectorValue & z,
                     RealTensorValue & stress);

  /**
   * Will always be passed to full symmetric strain tensor.
   * What should come out is a modified strain tensor.
   */
  virtual void computeStrain(const ColumnMajorMatrix & total_strain, ColumnMajorMatrix & elastic_strain);

  /**
   * The current quadrature point.
   */
  unsigned int _qp;

  Real _youngs_modulus;
  Real _poissons_ratio;

  Real _t_ref;
  Real _alpha;

  Real _input_thermal_conductivity;

  ElasticityTensor * _local_elasticity_tensor;
};

#endif //LINEARISOTROPICMATERIAL_H
