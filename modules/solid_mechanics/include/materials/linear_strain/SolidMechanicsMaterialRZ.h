#ifndef SOLIDMECHANICSMATERIALRZ_H
#define SOLIDMECHANICSMATERIALRZ_H

#include "Material.h"
#include "SymmTensor.h"

//Forward Declarations
class SolidMechanicsMaterialRZ;
class SymmElasticityTensor;
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

  virtual void initialSetup();

  virtual void computeStress(const SymmTensor & total_strain,
                             const SymmElasticityTensor & elasticity_tensor,
                             SymmTensor & strain,
                             SymmTensor & stress);

  virtual void computePreconditioning();

  virtual void computeStrain(const SymmTensor & input_strain, SymmTensor & elastic_strain) = 0;

  virtual void computeCracking(const SymmTensor & strain,
                               SymmTensor & stress);

  /// Determine if cracking occurred.  If so, perform rotations, etc.
  virtual void crackingStrainRotation( const SymmTensor & total_strain,
                                       SymmTensor & strain_inc );

  /// Rotate old and new stress to global, if cracking active
  virtual void crackingStressRotation();

  /**
   * The current quadrature point.
   */
  unsigned int _qp;

  const Real _youngs_modulus;
  const Real _poissons_ratio;
  const Real _shear_modulus;

  const bool _large_strain;

  const Real _cracking_strain;
  bool _cracking_locally_active;
  ColumnMajorMatrix _e_vec;

  const Real _alpha;

  VariableValue & _disp_r;
  VariableValue & _disp_z;

  VariableGradient & _grad_disp_r;
  VariableGradient & _grad_disp_z;

  bool _has_temp;
  VariableValue & _temp;
  VariableValue & _temp_old;

  std::vector<VolumetricModel*> _volumetric_models;

  MaterialProperty<SymmTensor> & _stress;
  MaterialProperty<SymmTensor> & _stress_old;
  MaterialProperty<SymmTensor> & _total_strain;
  MaterialProperty<SymmTensor> & _total_strain_old;
  MaterialProperty<RealVectorValue> * _crack_flags;
  MaterialProperty<RealVectorValue> * _crack_flags_old;
  MaterialProperty<SymmElasticityTensor> & _elasticity_tensor;
  MaterialProperty<SymmElasticityTensor> & _Jacobian_mult;
  // Accumulate derivatives of strain tensors with respect to Temperature into this
  SymmTensor _d_strain_dT;
  // The derivative of the stress with respect to Temperature
  MaterialProperty<SymmTensor> & _d_stress_dT;
  MaterialProperty<SymmTensor> & _elastic_strain;

  SymmTensor _stress_old_temp;
  SymmElasticityTensor * _local_elasticity_tensor;

};

#endif //SOLIDMECHANICSMATERIALRZ_H
