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
  virtual void computeProperties();

  virtual void computeStress(const ColumnMajorMatrix & strain,
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

  Real _input_density;

  ElasticityTensor * _local_elasticity_tensor;
};

#endif //LINEARISOTROPICMATERIALRZ_H
