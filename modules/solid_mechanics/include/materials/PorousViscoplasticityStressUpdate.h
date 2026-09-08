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
  virtual void initQpStatefulProperties() override;
  virtual void propagateQpStatefulProperties() override;
  virtual void resetIncrementalMaterialProperties() override;

  virtual GenericReal<is_ad>
  minimumPermissibleValue(const GenericReal<is_ad> & effective_trial_stress) const override;

  virtual GenericReal<is_ad>
  maximumPermissibleValue(const GenericReal<is_ad> & effective_trial_stress) const override;
  virtual Real
  computeReferenceResidual(const GenericReal<is_ad> & effective_trial_stress,
                           const GenericReal<is_ad> & scalar_effective_inelastic_strain) override;

protected:
  /** One Norton-type creep mechanism and its exponent-dependent porous gauge-surface factor. */
  struct CreepLaw
  {
    const GenericMaterialProperty<Real, is_ad> * coefficient = nullptr;
    Real power = 1.0;
    Real power_factor = 0.0;
  };

  /** First and second partial derivatives of one LPS gauge residual. */
  struct LpsDerivatives
  {
    GenericReal<is_ad> F_lambda = 0.0;
    GenericReal<is_ad> F_p = 0.0;
    GenericReal<is_ad> F_q = 0.0;
    GenericReal<is_ad> F_f = 0.0;

    GenericReal<is_ad> F_lambdalambda = 0.0;
    GenericReal<is_ad> F_lambdap = 0.0;
    GenericReal<is_ad> F_lambdaq = 0.0;
    GenericReal<is_ad> F_lambdaf = 0.0;
    GenericReal<is_ad> F_pp = 0.0;
    GenericReal<is_ad> F_pf = 0.0;
  };

  /** Side-effect-free response of one LPS creep mechanism. */
  struct LpsMechanismResponse
  {
    bool active = false;
    GenericReal<is_ad> coefficient = 0.0;
    GenericReal<is_ad> gauge_stress = 0.0;
    GenericReal<is_ad> creep_rate = 0.0;
    GenericReal<is_ad> effective_inelastic_strain_increment = 0.0;
    GenericReal<is_ad> F_lambda = 0.0;
    std::array<GenericReal<is_ad>, 3> dgauge_dx{};
    GenericRankTwoTensor<is_ad> inelastic_strain_increment;
    std::array<GenericRankTwoTensor<is_ad>, 3> dinelastic_dx;
  };

  /**
   * Summed LPS response and its partial derivatives with respect to effective hydrostatic stress,
   * equivalent stress, and explicit porosity, in that order.
   */
  struct LpsCreepResponse
  {
    GenericRankTwoTensor<is_ad> inelastic_strain_increment;
    GenericReal<is_ad> effective_inelastic_strain_increment = 0.0;
    GenericReal<is_ad> primary_gauge_stress = 0.0;
    std::array<GenericRankTwoTensor<is_ad>, 3> dinelastic_dx;
  };

  /** Number of independently evaluated Norton creep mechanisms. */
  std::size_t creepLawCount() const { return _creep_laws.size(); }

  /** Access one creep mechanism. */
  const CreepLaw & creepLaw(const std::size_t index) const { return _creep_laws[index]; }

  /** Return one finite, nonnegative Norton coefficient. */
  GenericReal<is_ad> creepCoefficient(std::size_t law_index) const;

  /** Evaluate C_i Lambda_i^n_i for one creep mechanism. */
  GenericReal<is_ad> computeCreepRate(const CreepLaw & law,
                                      const GenericReal<is_ad> & coefficient,
                                      const GenericReal<is_ad> & gauge_stress) const;

  /**
   * Compute an initial guess for the value of the scalar. For some cases, an
   * intelligent starting point can provide enhanced robustness in the Newton
   * iterations. This is also an opportunity for classes that derive from this
   * to perform initialization tasks.
   * @param effective_trial_stress Effective trial stress
   */
  virtual GenericReal<is_ad>
  initialGuess(const GenericReal<is_ad> & effective_trial_stress) override;

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

  GenericReal<is_ad> computeH(const Real n,
                              const GenericReal<is_ad> & gauge_stress,
                              const bool derivative = false) const;
  GenericReal<is_ad> computeGaugeResidual(const GenericReal<is_ad> & equiv_stress,
                                          const GenericReal<is_ad> & trial_gauge,
                                          const GenericReal<is_ad> & effective_hydro_stress,
                                          const GenericReal<is_ad> & porosity,
                                          const CreepLaw & law,
                                          GenericReal<is_ad> & derivative) const;
  GenericRankTwoTensor<is_ad> computeDGaugeDSigma(const GenericReal<is_ad> & gauge_stress,
                                                  const GenericReal<is_ad> & equiv_stress,
                                                  const GenericRankTwoTensor<is_ad> & dev_stress,
                                                  const GenericReal<is_ad> & effective_hydro_stress,
                                                  const GenericReal<is_ad> & porosity,
                                                  const CreepLaw & law) const;
  void computeInelasticStrainIncrement(GenericReal<is_ad> & total_creep_rate,
                                       GenericRankTwoTensor<is_ad> & creep_strain_increment,
                                       const GenericReal<is_ad> & equiv_stress,
                                       const GenericRankTwoTensor<is_ad> & dev_stress,
                                       const GenericReal<is_ad> & effective_hydro_stress,
                                       const GenericReal<is_ad> & porosity);

  /// Compute the gauge stress for a specific creep mechanism.
  void computeGaugeStress(GenericReal<is_ad> & gauge_stress,
                          const GenericReal<is_ad> & equiv_stress,
                          const GenericReal<is_ad> & effective_hydro_stress,
                          const GenericReal<is_ad> & porosity,
                          const CreepLaw & law);

  /// Store one converged per-law gauge stress.
  void setGaugeStress(std::size_t law_index, const GenericReal<is_ad> & gauge_stress);

  /// Evaluate and store all gauge-stress diagnostics at one converged constitutive state.
  void setGaugeStresses(const GenericReal<is_ad> & equiv_stress,
                        const GenericReal<is_ad> & effective_hydro_stress,
                        const GenericReal<is_ad> & porosity);

  /**
   * Evaluate the summed LPS creep response without modifying material properties. The returned
   * tensor derivatives are partial derivatives with respect to p_eff, q, and f.
   */
  LpsCreepResponse evaluateLpsCreepResponse(const GenericReal<is_ad> & effective_hydro_stress,
                                            const GenericReal<is_ad> & equiv_stress,
                                            const GenericRankTwoTensor<is_ad> & dev_direction,
                                            const GenericReal<is_ad> & porosity);

  /** Evaluate one LPS mechanism and exact p_eff-q-f partial derivatives without side effects. */
  LpsMechanismResponse
  evaluateLpsMechanismResponse(std::size_t law_index,
                               const GenericReal<is_ad> & effective_hydro_stress,
                               const GenericReal<is_ad> & equiv_stress,
                               const GenericRankTwoTensor<is_ad> & dev_direction,
                               const GenericReal<is_ad> & porosity);

  /// Analytical first and second partial derivatives of one LPS gauge residual.
  LpsDerivatives computeLpsDerivatives(const GenericReal<is_ad> & gauge_stress,
                                       const GenericReal<is_ad> & effective_hydro_stress,
                                       const GenericReal<is_ad> & equiv_stress,
                                       const GenericReal<is_ad> & porosity,
                                       const CreepLaw & law) const;

  /// Matrix hydrostatic stress for the configured pore geometry.
  GenericReal<is_ad> matrixHydroStress(const GenericRankTwoTensor<is_ad> & stress) const;

  /// Apply the optional porosity pressure to an explicitly supplied matrix hydrostatic stress.
  GenericReal<is_ad> effectiveHydroStress(const GenericReal<is_ad> & matrix_hydro_stress) const;
  /// True when either deviatoric or hydrostatic stress can drive the porous viscoplastic response.
  bool hasViscoplasticDrive(const GenericReal<is_ad> & equiv_stress,
                            const GenericReal<is_ad> & effective_hydro_stress,
                            const GenericReal<is_ad> & porosity) const;
  /// Positive stress scale used to initialize and bound the gauge-stress Newton solve.
  GenericReal<is_ad> gaugeStressScale(const GenericReal<is_ad> & equiv_stress,
                                      const GenericReal<is_ad> & effective_hydro_stress) const;
  /// Perform one viscoplastic update over the current constitutive timestep.
  virtual void updateStateOneStep(GenericRankTwoTensor<is_ad> & elastic_strain_increment,
                                  GenericRankTwoTensor<is_ad> & inelastic_strain_increment,
                                  GenericRankTwoTensor<is_ad> & stress,
                                  const GenericRankFourTensor<is_ad> & elasticity_tensor,
                                  const GenericRankTwoTensor<is_ad> & elastic_strain_old,
                                  GenericReal<is_ad> & effective_inelastic_strain_increment);
  /// Estimate the number of local constitutive substeps from the full-step trial stress.
  virtual unsigned int estimateNumberSubsteps(const GenericRankTwoTensor<is_ad> & stress);
  /// Estimate local substeps from an explicitly supplied hydrostatic driving stress and porosity.
  unsigned int estimateNumberSubstepsFromState(const GenericRankTwoTensor<is_ad> & stress,
                                               const GenericReal<is_ad> & effective_hydro_stress,
                                               const GenericReal<is_ad> & porosity);
  /// Estimate adaptive substeps from the previous accepted global-step effective inelastic rate.
  unsigned int estimateAdaptiveNumberSubstepsFromHistory() const;
  /// Store the converged current global-step effective inelastic rate for use after timestep acceptance.
  void recordEffectiveInelasticStrainRate(
      const GenericReal<is_ad> & effective_inelastic_strain_increment);
  /**
   * Check one accepted local effective inelastic increment against the requested substep target.
   * If the target is exceeded, record an a-posteriori retry suggestion and throw so the adaptive
   * driver can restart from the accepted global-old state.
   */
  void checkSubstepIncrement(const GenericReal<is_ad> & effective_inelastic_strain_increment,
                             unsigned int total_number_substeps,
                             unsigned int substep_index);

  /// Integrate a prescribed number of local constitutive substeps.
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

  /// Independently evaluated creep mechanisms. Each LPS exponent owns its own gauge stress.
  std::vector<CreepLaw> _creep_laws;

  /// Optional gas pressure in the pore/bubble
  const GenericMaterialProperty<Real, is_ad> * const _additional_porosity_pressure;

  /// Gauge stress for the first active creep law, retained for backward-compatible output.
  GenericMaterialProperty<Real, is_ad> & _gauge_stress;

  /// Per-law gauge stress outputs, declared only when multiple creep laws are supplied.
  std::vector<GenericMaterialProperty<Real, is_ad> *> _gauge_stress_laws;

  /// Maximum ratio between the gauge stress and the equivalent stress/pressure scale
  const Real _maximum_gauge_ratio;
  /// Minimum stress scale below which viscoplasticity is not calculated
  const Real _minimum_stress_magnitude;

  /// Maximum value of equivalent stress above which an exception is thrown
  const Real _maximum_stress_magnitude;

  /// Whether and how local constitutive substepping is used
  const SubsteppingType _use_substepping;

  /// Target fraction of max_inelastic_increment in one local substep
  const Real _substep_tolerance;
  /// Whether a failed constitutive attempt is retried with more substeps
  const bool _adaptive_substepping;

  /// Maximum number of local constitutive substeps
  const unsigned int _maximum_number_substeps;

  /// Current converged global-step effective inelastic rate.
  GenericMaterialProperty<Real, is_ad> & _effective_inelastic_strain_rate;
  /// Previous accepted global-step effective inelastic rate used by adaptive substep prediction.
  const MaterialProperty<Real> & _effective_inelastic_strain_rate_old;

  /// A-posteriori number of substeps suggested by the last failed increment check.
  unsigned int _suggested_number_substeps;

  /// Effective inelastic increment from the last successful one-step update.
  Real _last_effective_inelastic_strain_increment;

  /// Container for matrix hydrostatic stress
  GenericReal<is_ad> _hydro_stress;
  /**
   * State used by SingleVariableReturnMappingSolution while solving for gauge stress. The generic
   * return-mapping callback only passes the equivalent stress and scalar unknown, so the explicitly
   * supplied hydrostatic stress, porosity, and active creep law are stored here for the duration of
   * the solve.
   */
  struct GaugeSolveState
  {
    GenericReal<is_ad> effective_hydro_stress = 0.0;
    GenericReal<is_ad> porosity = 0.0;
    const CreepLaw * law = nullptr;
  } _gauge_solve_state;
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
