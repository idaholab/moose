//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ViscoplasticityStressUpdateBase.h"
#include "SingleVariableReturnMappingSolution.h"

template <bool is_ad>
class PorousViscoplasticityStressUpdateTempl
  : public ViscoplasticityStressUpdateBaseTempl<is_ad>,
    public SingleVariableReturnMappingSolutionTempl<is_ad>
{
public:
  static InputParameters validParams();

  PorousViscoplasticityStressUpdateTempl(const InputParameters & parameters);

  enum class SubsteppingType
  {
    NONE,
    INCREMENT_BASED
  };

  virtual void updateState(
      GenericRankTwoTensor<is_ad> & strain_increment,
      GenericRankTwoTensor<is_ad> & inelastic_strain_increment,
      const GenericRankTwoTensor<is_ad> & rotation_increment,
      GenericRankTwoTensor<is_ad> & stress_new,
      const RankTwoTensor & stress_old,
      const GenericRankFourTensor<is_ad> & elasticity_tensor,
      const RankTwoTensor & elastic_strain_old,
      bool compute_full_tangent_operator = false,
      RankFourTensor & tangent_operator = StressUpdateBaseTempl<is_ad>::_identityTensor) override;

  virtual void updateStateSubstep(
      GenericRankTwoTensor<is_ad> & strain_increment,
      GenericRankTwoTensor<is_ad> & inelastic_strain_increment,
      const GenericRankTwoTensor<is_ad> & rotation_increment,
      GenericRankTwoTensor<is_ad> & stress_new,
      const RankTwoTensor & stress_old,
      const GenericRankFourTensor<is_ad> & elasticity_tensor,
      const RankTwoTensor & elastic_strain_old,
      bool compute_full_tangent_operator = false,
      RankFourTensor & tangent_operator = StressUpdateBaseTempl<is_ad>::_identityTensor) override;

  virtual bool substeppingCapabilityEnabled() override;
  virtual bool substeppingCapabilityRequested() override;
  virtual void resetIncrementalMaterialProperties() override;

  virtual GenericReal<is_ad>
  minimumPermissibleValue(const GenericReal<is_ad> & effective_trial_stress) const override;

  virtual GenericReal<is_ad>
  maximumPermissibleValue(const GenericReal<is_ad> & effective_trial_stress) const override;

  virtual Real
  computeReferenceResidual(const GenericReal<is_ad> & effective_trial_stress,
                           const GenericReal<is_ad> & scalar_effective_inelastic_strain) override;

protected:
  /**
   * Compute an initial guess for the value of the scalar. For some cases, an
   * intellegent starting point can provide enhanced robustness in the Newton
   * iterations. This is also an opportunity for classes that derive from this
   * to perform initialization tasks.
   * @param effective_trial_stress Effective trial stress
   */
  virtual GenericReal<is_ad>
  initialGuess(const GenericReal<is_ad> & effective_trial_stress) override;

  /**
   * Perform any necessary steps to finalize state after return mapping iterations
   * @param inelasticStrainIncrement Inelastic strain increment
   */
  virtual GenericReal<is_ad> computeResidual(const GenericReal<is_ad> & effective_trial_stress,
                                             const GenericReal<is_ad> & scalar) override;

  virtual GenericReal<is_ad>
  computeDerivative(const GenericReal<is_ad> & /*effective_trial_stress*/,
                    const GenericReal<is_ad> & /*scalar*/) override
  {
    return _derivative;
  }

  void outputIterationSummary(std::stringstream * iter_output,
                              const unsigned int total_it) override;

  GenericReal<is_ad>
  computeH(const Real n, const GenericReal<is_ad> & gauge_stress, const bool derivative = false);

  GenericRankTwoTensor<is_ad> computeDGaugeDSigma(const GenericReal<is_ad> & gauge_stress,
                                                  const GenericReal<is_ad> & equiv_stress,
                                                  const GenericRankTwoTensor<is_ad> & dev_stress,
                                                  const GenericRankTwoTensor<is_ad> & stress);

  void computeInelasticStrainIncrement(GenericReal<is_ad> & gauge_stress,
                                       GenericReal<is_ad> & dpsi_dgauge,
                                       GenericRankTwoTensor<is_ad> & creep_strain_increment,
                                       const GenericReal<is_ad> & equiv_stress,
                                       const GenericRankTwoTensor<is_ad> & dev_stress,
                                       const GenericRankTwoTensor<is_ad> & stress);

  /// Compute the gauge stress for the current stress invariants and porosity.
  void computeGaugeStress(GenericReal<is_ad> & gauge_stress,
                          const GenericReal<is_ad> & equiv_stress);

  /// Hydrostatic stress driving the pore: matrix hydrostatic stress plus bubble pressure.
  GenericReal<is_ad> effectiveHydroStress() const;

  /// True when either deviatoric stress or gas/pore hydrostatic stress can drive viscoplasticity.
  bool hasViscoplasticDrive(const GenericReal<is_ad> & equiv_stress) const;

  /// Positive stress scale used to initialize and bound the gauge-stress Newton solve.
  GenericReal<is_ad> gaugeStressScale(const GenericReal<is_ad> & equiv_stress) const;

  /// Perform one explicit viscoplastic update over the current local value of _dt.
  void updateStateOneStep(GenericRankTwoTensor<is_ad> & elastic_strain_increment,
                          GenericRankTwoTensor<is_ad> & inelastic_strain_increment,
                          GenericRankTwoTensor<is_ad> & stress,
                          const GenericRankFourTensor<is_ad> & elasticity_tensor,
                          const GenericRankTwoTensor<is_ad> & elastic_strain_old,
                          GenericReal<is_ad> & effective_inelastic_strain_increment);

  /// Estimate the number of local constitutive substeps from the full-step trial stress.
  unsigned int estimateNumberSubsteps(const GenericRankTwoTensor<is_ad> & stress);

  /// Integrate a prescribed number of explicit local constitutive substeps.
  void updateStateSubstepInternal(
      GenericRankTwoTensor<is_ad> & strain_increment,
      GenericRankTwoTensor<is_ad> & inelastic_strain_increment,
      const GenericRankTwoTensor<is_ad> & rotation_increment,
      GenericRankTwoTensor<is_ad> & stress_new,
      const RankTwoTensor & stress_old,
      const GenericRankFourTensor<is_ad> & elasticity_tensor,
      const RankTwoTensor & elastic_strain_old,
      unsigned int total_number_substeps,
      bool compute_full_tangent_operator = false,
      RankFourTensor & tangent_operator = StressUpdateBaseTempl<is_ad>::_identityTensor);

  /// Enum to choose which viscoplastic model to use
  const enum class ViscoplasticityModel { LPS, GTN } _model;

  /// Enum to choose which pore shape model to use
  const enum class PoreShapeModel { SPHERICAL, CYLINDRICAL } _pore_shape;

  /// Pore shape factor depending on pore shape model
  const Real _pore_shape_factor;

  /// Exponent on the effective stress
  const Real _power;

  /// Power factor used for LPS model
  const Real _power_factor;

  /// Leading coefficient
  const GenericMaterialProperty<Real, is_ad> & _coefficient;

  /// Optional gas pressure in the pore/bubble
  const GenericMaterialProperty<Real, is_ad> * const _additional_porosity_pressure;

  /// Gauge stress
  GenericMaterialProperty<Real, is_ad> & _gauge_stress;

  /// Maximum ratio between the gauge stress and the equivalent stress/pressure scale
  const Real _maximum_gauge_ratio;

  /// Minimum stress scale below which viscoplasticity is not calculated
  const Real _minimum_equivalent_stress;

  /// Maximum value of equivalent stress above which an exception is thrown
  const Real _maximum_equivalent_stress;

  /// Whether and how local explicit substepping is used
  const SubsteppingType _use_substepping;

  /// Target fraction of max_inelastic_increment in one local substep
  const Real _substep_tolerance;

  /// Whether a failed constitutive attempt is retried with more substeps
  const bool _adaptive_substepping;

  /// Maximum number of local constitutive substeps
  const unsigned int _maximum_number_substeps;

  /// Original global timestep, restored after local substepping
  Real _dt_original;

  /// Container for matrix hydrostatic stress
  GenericReal<is_ad> _hydro_stress;

  /// Rank two identity tensor
  const RankTwoTensor _identity_two;

  /// Derivative of hydrostatic stress with respect to the stress tensor
  const RankTwoTensor _dhydro_stress_dsigma;

  /// Container for dF/dLambda
  GenericReal<is_ad> _derivative;

  usingViscoplasticityStressUpdateBaseMembers;
};

typedef PorousViscoplasticityStressUpdateTempl<false> PorousViscoplasticityStressUpdate;
typedef PorousViscoplasticityStressUpdateTempl<true> ADPorousViscoplasticityStressUpdate;
