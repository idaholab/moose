//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AnisotropicReturnCreepStressUpdateBase.h"

/**
 * This class uses the stress update material for an anisotropic creep
 * model.  This class is one of the basic radial return constitutive models; more complex
 * constitutive models combine creep and plasticity.
 *
 * This class inherits from AnisotropicReturnCreepStressUpdateBase and must be used
 * in conjunction with ComputeMultipleInelasticStress.  This class calculates
 * creep based on stress, temperature, and time effects.  This class also
 * computes the creep strain as a stateful material property.
 *
 * This class extends the usage of PowerLawCreep to Hill (i.e. anisotropic)
 * cases. For more information, consult, e.g. Stewart et al, "An anisotropic tertiary creep
 * damage constitutive model for anistropic materials", International Journal of Pressure Vessels
 * and Piping 88 (2011) 356--364.
 */
template <bool is_ad>
class HillCreepStressUpdateTempl : public AnisotropicReturnCreepStressUpdateBaseTempl<is_ad>
{
public:
  static InputParameters validParams();

  HillCreepStressUpdateTempl(const InputParameters & parameters);

protected:
  using Material::_current_elem;
  using Material::_dt;
  using Material::_q_point;
  using Material::_qp;
  using Material::_t;

  virtual void
  computeStressInitialize(const GenericDenseVector<is_ad> & stress_dev,
                          const GenericDenseVector<is_ad> & stress,
                          const GenericRankFourTensor<is_ad> & elasticity_tensor) override;
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

  virtual GenericReal<is_ad>
  initialGuess(const GenericDenseVector<is_ad> & /*stress_dev*/) override;

  /**
   * Does the model require the elasticity tensor to be isotropic? Not in principle.
   * TODO: Take care of rotation of anisotropy parameters
   */
  bool requiresIsotropicTensor() override { return false; }

  /**
   * Compute the limiting value of the time step for this material according to the numerical
   * integration error
   * @return Limiting time step
   */
  virtual Real computeIntegrationErrorTimeStep() override
  {
    return this->_max_integration_error_time_step;
  }

  /// Flag to determine if temperature is supplied by the user
  const bool _has_temp;

  /// Temperature variable value
  const VariableValue & _temperature;

  /// Leading coefficient
  const Real _coefficient;

  /// Exponent on the effective stress
  const Real _n_exponent;

  /// Exponent on time
  const Real _m_exponent;

  /// Activation energy for exp term
  const Real _activation_energy;

  /// Gas constant for exp term
  const Real _gas_constant;

  /// Simulation start time
  const Real _start_time;

  /// Exponential calculated from activation, gas constant, and temperature
  Real _exponential;

  /// Exponential calculated from current time
  Real _exp_time;

  /// Hill constant material
  const MaterialProperty<std::vector<Real>> & _hill_constants;

  /// Hill tensor, when global axes do not (somehow) align with those of the material
  /// Example: Large rotation due to rigid body and/or large deformation kinematics
  const MaterialProperty<DenseMatrix<Real>> * _hill_tensor;

  /// Square of the q function for orthotropy
  GenericReal<is_ad> _qsigma;

  /// 2 * shear modulus
  GenericReal<is_ad> _two_shear_modulus;
};

typedef HillCreepStressUpdateTempl<false> HillCreepStressUpdate;
typedef HillCreepStressUpdateTempl<true> ADHillCreepStressUpdate;
