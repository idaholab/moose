//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AnisotropicReturnPlasticityStressUpdateBase.h"

/**
 * This class uses the stress update material in an anisotropic return mapping.
 * This class is one of the generalized radial return constitutive models based on Hill's criterion;
 * it assumes and anisotropic elasticity tensor and an anisotropic plastic yield surface.
 * Constitutive models that combine creep and plasticity can be used.
 */

template <bool is_ad>
class HillElastoPlasticityStressUpdateTempl
  : public AnisotropicReturnPlasticityStressUpdateBaseTempl<is_ad>
{
public:
  static InputParameters validParams();

  HillElastoPlasticityStressUpdateTempl(const InputParameters & parameters);

  virtual Real computeStrainEnergyRateDensity(
      const GenericMaterialProperty<RankTwoTensor, is_ad> & stress,
      const GenericMaterialProperty<RankTwoTensor, is_ad> & strain_rate) override;

protected:
  using AnisotropicReturnPlasticityStressUpdateBaseTempl<is_ad>::_effective_inelastic_strain;
  using AnisotropicReturnPlasticityStressUpdateBaseTempl<is_ad>::_effective_inelastic_strain_old;
  using AnisotropicReturnPlasticityStressUpdateBaseTempl<is_ad>::_plasticity_strain;
  using AnisotropicReturnPlasticityStressUpdateBaseTempl<is_ad>::_plasticity_strain_old;
  using AnisotropicReturnPlasticityStressUpdateBaseTempl<is_ad>::isBlockDiagonal;
  using Material::_current_elem;
  using Material::_dt;
  using Material::_q_point;
  using Material::_qp;
  using Material::_t;

  virtual void
  computeStressInitialize(const GenericDenseVector<is_ad> & stress_dev,
                          const GenericDenseVector<is_ad> & stress,
                          const GenericRankFourTensor<is_ad> & elasticity_tensor) override;

  /**
   * Compute eigendecomposition of element-wise elasticity tensor
   */
  void computeElasticityTensorEigenDecomposition();

  virtual GenericReal<is_ad>
  computeResidual(const GenericDenseVector<is_ad> & effective_trial_stress,
                  const GenericDenseVector<is_ad> & stress_new,
                  const GenericReal<is_ad> & scalar) override;
  virtual GenericReal<is_ad>
  computeDerivative(const GenericDenseVector<is_ad> & effective_trial_stress,
                    const GenericDenseVector<is_ad> & stress_new,
                    const GenericReal<is_ad> & scalar) override;

  virtual Real
  computeReferenceResidual(const GenericDenseVector<is_ad> & effective_trial_stress,
                           const GenericDenseVector<is_ad> & stress_new,
                           const GenericReal<is_ad> & residual,
                           const GenericReal<is_ad> & scalar_effective_inelastic_strain) override;
  virtual void propagateQpStatefulProperties() override;
  /**
   * Does the model require the elasticity tensor to be isotropic? No, this class does
   * anisotropic *elasto-plasticity*
   */
  bool requiresIsotropicTensor() override { return false; }

  Real computeHardeningDerivative();
  GenericReal<is_ad> computeHardeningValue(const GenericReal<is_ad> & scalar,
                                           const GenericReal<is_ad> & omega);
  /**
   * Compute eigendecomposition of Hill's tensor for anisotropic plasticity
   * @param hill_tensor 6x6 matrix representing fourth order Hill's tensor describing anisotropy
   */
  void computeHillTensorEigenDecomposition(const DenseMatrix<GenericReal<is_ad>> & hill_tensor);

  /**
   * Perform any necessary steps to finalize strain increment after return mapping iterations
   * @param inelasticStrainIncrement Inelastic strain increment
   * @param stress Cauchy stresss tensor
   * @param stress_dev Deviatoric partt of the Cauchy stresss tensor
   * @param delta_gamma Generalized radial return's plastic multiplier
   */
  virtual void computeStrainFinalize(GenericRankTwoTensor<is_ad> & inelasticStrainIncrement,
                                     const GenericRankTwoTensor<is_ad> & stress,
                                     const GenericDenseVector<is_ad> & stress_dev,
                                     const GenericReal<is_ad> & delta_gamma) override;

  /**
   * Perform any necessary steps to finalize state after return mapping iterations
   * @param inelasticStrainIncrement Inelastic strain increment
   * @param delta_gamma Generalized radial return's plastic multiplier
   * @param stress Cauchy stresss tensor
   * @param stress_dev Deviatoric partt of the Cauchy stresss tensor
   */
  virtual void
  computeStressFinalize(const GenericRankTwoTensor<is_ad> & inelasticStrainIncrement,
                        const GenericReal<is_ad> & delta_gamma,
                        GenericRankTwoTensor<is_ad> & stress,
                        const GenericDenseVector<is_ad> & stress_dev,
                        const GenericRankTwoTensor<is_ad> & stress_old,
                        const GenericRankFourTensor<is_ad> & elasticity_tensor) override;

  GenericReal<is_ad> computeOmega(const GenericReal<is_ad> & delta_gamma,
                                  const GenericDenseVector<is_ad> & stress_trial);

  void computeDeltaDerivatives(const GenericReal<is_ad> & delta_gamma,
                               const GenericDenseVector<is_ad> & stress_trial,
                               const GenericReal<is_ad> & sy_alpha,
                               GenericReal<is_ad> & omega,
                               GenericReal<is_ad> & omega_gamma,
                               GenericReal<is_ad> & sy_gamma);

  /// Square of the q function for orthotropy
  GenericReal<is_ad> _qsigma;

  GenericDenseVector<is_ad> _eigenvalues_hill;
  GenericDenseMatrix<is_ad> _eigenvectors_hill;
  GenericDenseMatrix<is_ad> _anisotropic_elastic_tensor;

  /// Name of the elasticity tensor material property
  const std::string _elasticity_tensor_name;
  /// Anisotropic elasticity tensor material property
  const GenericMaterialProperty<RankFourTensor, is_ad> & _elasticity_tensor;

  const Real _hardening_constant;
  const Real _hardening_exponent;

  GenericMaterialProperty<Real, is_ad> & _hardening_variable;
  const MaterialProperty<Real> & _hardening_variable_old;

  GenericMaterialProperty<DenseMatrix<Real>, is_ad> & _elasticity_eigenvectors;
  GenericMaterialProperty<DenseVector<Real>, is_ad> & _elasticity_eigenvalues;

  GenericMaterialProperty<DenseMatrix<Real>, is_ad> & _b_eigenvectors;
  GenericMaterialProperty<DenseVector<Real>, is_ad> & _b_eigenvalues;

  GenericMaterialProperty<DenseMatrix<Real>, is_ad> & _alpha_matrix;
  GenericMaterialProperty<DenseVector<Real>, is_ad> & _sigma_tilde;

  GenericReal<is_ad> _hardening_derivative;
  GenericReal<is_ad> _yield_condition;
  GenericReal<is_ad> _yield_stress;

  /// Hill tensor, when global axes do not (somehow) align with those of the material
  /// Example: Large rotation due to rigid body and/or large deformation kinematics
  const MaterialProperty<DenseMatrix<Real>> & _hill_tensor;
};

typedef HillElastoPlasticityStressUpdateTempl<false> HillElastoPlasticityStressUpdate;
typedef HillElastoPlasticityStressUpdateTempl<true> ADHillElastoPlasticityStressUpdate;
