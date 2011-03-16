#ifndef SOLIDMECHANICSMATERIALRZ_H
#define SOLIDMECHANICSMATERIALRZ_H

#include "Material.h"


//Forward Declarations
class ElasticityTensor;
class SolidMechanicsMaterialRZ;
class VolumetricModel;

template<>
InputParameters validParams<SolidMechanicsMaterialRZ>();

/**
 * AxisymmetricSolidMechanics material for use in simple applications that don't need material properties.
 */
class SolidMechanicsMaterialRZ : public Material
{
public:
  SolidMechanicsMaterialRZ(const std::string & name, InputParameters parameters);
  virtual ~SolidMechanicsMaterialRZ();

protected:

  virtual void computeProperties();

  virtual void subdomainSetup();

  virtual void computeStress(const ColumnMajorMatrix & strain,
                             const ElasticityTensor & elasticity_tensor,
                             RealTensorValue & stress);

  virtual void computeStrain(const ColumnMajorMatrix & total_strain, ColumnMajorMatrix & elastic_strain) = 0;

  bool _initialized;

  /**
   * The current quadrature point.
   */
  unsigned int _qp;

  const Real _youngs_modulus;
  const Real _poissons_ratio;
  const Real _shear_modulus;

  const Real _t_ref;
  const Real _alpha;

  VariableValue & _disp_r;
  VariableValue & _disp_z;

  VariableGradient & _grad_disp_r;
  VariableGradient & _grad_disp_z;

  bool _has_temp;
  VariableValue & _temp;

  std::vector<VolumetricModel*> _volumetric_models;

  MaterialProperty<RealTensorValue> & _stress;
  MaterialProperty<RealTensorValue> & _stress_old;
  MaterialProperty<ColumnMajorMatrix> & _elasticity_tensor;
  MaterialProperty<ColumnMajorMatrix> & _Jacobian_mult;
  MaterialProperty<ColumnMajorMatrix> & _elastic_strain;
  MaterialProperty<ColumnMajorMatrix> & _v_strain;
  MaterialProperty<ColumnMajorMatrix> & _v_strain_old;

  ElasticityTensor * _local_elasticity_tensor;

};

#endif //SOLIDMECHANICSMATERIALRZ_H
