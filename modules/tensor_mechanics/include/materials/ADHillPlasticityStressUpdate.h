//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADAnisotropicReturnPlasticityStressUpdateBase.h"

/**
 * This class uses the stress update material in an anisotropic return mapping.
 * This class is one of the generalized radial return constitutive models based on Hill's criterion;
 * it assumes and isotropic elasticity tensor and an anisotropic plastic yield surface.
 * Constitutive models that combine creep and plasticity can be used.
 */

class ADHillPlasticityStressUpdate : public ADAnisotropicReturnPlasticityStressUpdateBase
{
public:
  static InputParameters validParams();

  ADHillPlasticityStressUpdate(const InputParameters & parameters);

protected:
  virtual void computeStressInitialize(const ADDenseVector & stress_dev,
                                       const ADDenseVector & stress,
                                       const ADRankFourTensor & elasticity_tensor) override;
  virtual ADReal computeResidual(const ADDenseVector & effective_trial_stress,
                                 const ADDenseVector & stress_new,
                                 const ADReal & scalar) override;
  virtual ADReal computeDerivative(const ADDenseVector & effective_trial_stress,
                                   const ADDenseVector & stress_new,
                                   const ADReal & scalar) override;

  virtual Real computeReferenceResidual(const ADDenseVector & effective_trial_stress,
                                        const ADDenseVector & stress_new,
                                        const ADReal & residual,
                                        const ADReal & scalar_effective_inelastic_strain) override;
  virtual void propagateQpStatefulProperties() override;
  /**
   * Does the model require the elasticity tensor to be isotropic? Yes, this class only does
   * anisotropic *plasticity*
   */
  bool requiresIsotropicTensor() override { return true; }

  ADReal computeHardeningDerivative();
  ADReal computeHardeningValue(const ADReal & scalar, const ADReal & omega);
  /**
   * Compute eigendecomposition of Hill's tensor for anisotropic plasticity
   * @param hill_tensor 6x6 matrix representing fourth order Hill's tensor describing anisotropy
   */
  void computeHillTensorEigenDecomposition(ADDenseMatrix & hill_tensor);

  /**
   * Perform any necessary steps to finalize strain increment after return mapping iterations
   * @param inelasticStrainIncrement Inelastic strain increment
   */
  virtual void computeStrainFinalize(ADRankTwoTensor & /*inelasticStrainIncrement*/,
                                     const ADRankTwoTensor & /*stress*/,
                                     const ADDenseVector & /*stress_dev*/,
                                     const ADReal & /*delta_gamma*/) override;

  /**
   * Perform any necessary steps to finalize state after return mapping iterations
   * @param inelasticStrainIncrement Inelastic strain increment
   */
  virtual void computeStressFinalize(const ADRankTwoTensor & inelasticStrainIncrement,
                                     const ADReal & delta_gamma,
                                     ADRankTwoTensor & stress,
                                     const ADDenseVector & /*stress_dev*/,
                                     const ADRankTwoTensor & /*stress_old*/,
                                     const ADRankFourTensor & /*elasticity_tensor*/) override;

  ADReal computeOmega(const ADReal & delta_gamma, const ADDenseVector & stress_trial);

  void computeDeltaDerivatives(const ADReal & delta_gamma,
                               const ADDenseVector & stress_trial,
                               const ADReal & sy_alpha,
                               ADReal & omega,
                               ADReal & omega_gamma,
                               ADReal & sy_gamma);

  /// Hill constants for orthotropic creep
  std::vector<Real> _hill_constants;

  /// Square of the q function for orthotropy
  ADReal _qsigma;

  ADDenseVector _eigenvalues_hill;
  ADDenseMatrix _eigenvectors_hill;

  const Real _hardening_constant;
  const Function * const _hardening_function;

  ADMaterialProperty<Real> & _hardening_variable;
  const MaterialProperty<Real> & _hardening_variable_old;
  ADReal _hardening_slope;
  ADReal _yield_condition;
  ADReal _yield_stress;
  ADDenseMatrix _hill_tensor;
  ADDenseVector _stress_np1;
  /// 2 * shear modulus
  ADReal _two_shear_modulus;
};
