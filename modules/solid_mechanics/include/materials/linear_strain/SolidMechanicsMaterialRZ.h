#ifndef SOLIDMECHANICSMATERIALRZ_H
#define SOLIDMECHANICSMATERIALRZ_H

#include "Material.h"


//Forward Declarations
class SolidMechanicsMaterialRZ;
class SymmElasticityTensor;
class VolumetricModel;
class SymmTensor;

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

  virtual void initialSetup();

  virtual void computeStress(const SymmTensor & total_strain,
                             const SymmTensor & strain,
                             const SymmElasticityTensor & elasticity_tensor,
                             SymmTensor & stress);

  virtual void computeStrain(const SymmTensor & total_strain, SymmTensor & elastic_strain) = 0;

  virtual void computeCracking(const SymmTensor & strain,
                               SymmTensor & stress);

  /**
   * The current quadrature point.
   */
  unsigned int _qp;

  const Real _youngs_modulus;
  const Real _poissons_ratio;
  const Real _shear_modulus;

  const bool _large_strain;

  const Real _cracking_strain;

  const Real _t_ref;
  const Real _alpha;

  VariableValue & _disp_r;
  VariableValue & _disp_z;

  VariableGradient & _grad_disp_r;
  VariableGradient & _grad_disp_z;

  bool _has_temp;
  VariableValue & _temp;

  std::vector<VolumetricModel*> _volumetric_models;

  MaterialProperty<SymmTensor> & _stress;
  MaterialProperty<SymmTensor> & _stress_old;
  MaterialProperty<SymmTensor> & _total_strain;
  MaterialProperty<SymmTensor> & _total_strain_old;
  MaterialProperty<RealVectorValue> * _crack_flags;
  MaterialProperty<RealVectorValue> * _crack_flags_old;
  MaterialProperty<SymmElasticityTensor> & _elasticity_tensor;
  MaterialProperty<SymmElasticityTensor> & _Jacobian_mult;
  MaterialProperty<SymmTensor> & _elastic_strain;
  MaterialProperty<SymmTensor> & _v_strain;
  MaterialProperty<SymmTensor> & _v_strain_old;

  SymmElasticityTensor * _local_elasticity_tensor;

};

#endif //SOLIDMECHANICSMATERIALRZ_H
