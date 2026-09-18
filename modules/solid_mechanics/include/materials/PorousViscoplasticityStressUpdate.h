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

#include <array>
#include <cmath>
#include <limits>
#include <optional>
#include <vector>

/**
 * Spherical LPS porous viscoplasticity with porosity solved as a local constitutive unknown.
 *
 * Matrix hydrostatic stress p, equivalent stress q, and porosity f are advanced simultaneously
 * using an analytical local Jacobian. Derived models may optionally expose two independently
 * evolving pore-porosity populations; that path advances (p,q,f_0,f_1) while retaining one common
 * matrix creep response. Optional linear porosity corrections to the isotropic Young's modulus and
 * Poisson's ratio are evaluated at the current local porosity so elastic softening participates in
 * the same constitutive solve. Derived models specialize the pore-pressure closure and
 * accepted-state bookkeeping.
 */
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

  /** The spherical LPS porous constitutive response is isotropic. */
  bool isIsotropic() override { return true; }

  /** Return the exact one-step tangent when available. */
  TangentCalculationMethod getTangentCalculationMethod() override;
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
  bool substeppingCapabilityEnabled() override { return _use_substepping != SubsteppingType::NONE; }
  bool substeppingCapabilityRequested() override { return substeppingCapabilityEnabled(); }
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

  /** Value and first two derivatives of the LPS hydrostatic function H(M). */
  struct LpsHDerivatives
  {
    GenericReal<is_ad> value = 1.0;
    GenericReal<is_ad> first = 0.0;
    GenericReal<is_ad> second = 0.0;
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

  /** Summed LPS response and its exact local partial derivatives. */
  struct LpsCreepResponse
  {
    GenericRankTwoTensor<is_ad> inelastic_strain_increment;
    GenericReal<is_ad> effective_inelastic_strain_increment = 0.0;
    /// Derivative with respect to the common matrix-hydrostatic coordinate.
    /// The legacy member name is retained for source compatibility.
    GenericRankTwoTensor<is_ad> dinelastic_deffective_hydro_stress;
    GenericRankTwoTensor<is_ad> dinelastic_dequiv_stress;
    GenericRankTwoTensor<is_ad> dinelastic_dporosity;
  };

  static constexpr unsigned int MAX_HYDROSTATIC_STRESS_POPULATIONS = 2;
  static constexpr unsigned int INDEPENDENT_LOCAL_SYSTEM_SIZE =
      2 + MAX_HYDROSTATIC_STRESS_POPULATIONS;
  using PorePorosityState = std::array<GenericReal<is_ad>, MAX_HYDROSTATIC_STRESS_POPULATIONS>;

  /** First and second derivatives needed by the independent two-porosity LPS response. */
  struct IndependentLpsDerivatives
  {
    GenericReal<is_ad> F_lambda = 0.0;
    GenericReal<is_ad> F_p = 0.0;
    GenericReal<is_ad> F_q = 0.0;
    PorePorosityState F_porosity{};

    GenericReal<is_ad> F_lambdalambda = 0.0;
    GenericReal<is_ad> F_lambdap = 0.0;
    GenericReal<is_ad> F_lambdaq = 0.0;
    PorePorosityState F_lambdaporosity{};
    GenericReal<is_ad> F_pp = 0.0;
    PorePorosityState F_pporosity{};

    PorePorosityState population_F_p{};
    PorePorosityState population_F_lambdap{};
    PorePorosityState population_F_pp{};
    std::array<PorePorosityState, MAX_HYDROSTATIC_STRESS_POPULATIONS> population_F_pporosity{};
  };

  /** LPS response split into the volumetric increments carried by each pore population. */
  struct IndependentLpsCreepResponse
  {
    GenericRankTwoTensor<is_ad> inelastic_strain_increment;
    GenericReal<is_ad> effective_inelastic_strain_increment = 0.0;
    PorePorosityState population_volumetric_strain_increment{};
    std::array<GenericRankTwoTensor<is_ad>, INDEPENDENT_LOCAL_SYSTEM_SIZE> dinelastic_dx{};
    std::array<std::array<GenericReal<is_ad>, INDEPENDENT_LOCAL_SYSTEM_SIZE>,
               MAX_HYDROSTATIC_STRESS_POPULATIONS>
        dpopulation_volumetric_dx{};
  };

  /** One pore population's share of total porosity and hydrostatic driving stress. */
  struct HydrostaticStressPopulation
  {
    GenericReal<is_ad> fraction = 0.0;
    GenericReal<is_ad> effective_hydro_stress = 0.0;
    GenericReal<is_ad> deffective_hydro_df = 0.0;
    /// Pressure derivatives with respect to independently evolving pore-population porosities.
    PorePorosityState deffective_hydro_dporosity{};
  };

  /**
   * Hydrostatic driving state for one or more pore populations.
   *
   * The first two fields retain the original single-population interface. Derived classes that
   * need multiple pore populations set population_count and populate populations instead.
   */
  struct HydrostaticStressState
  {
    GenericReal<is_ad> effective_hydro_stress = 0.0;
    GenericReal<is_ad> deffective_hydro_df = 0.0;
    std::array<HydrostaticStressPopulation, MAX_HYDROSTATIC_STRESS_POPULATIONS> populations{};
    unsigned int population_count = 0;
  };

  /**
   * Evaluate the hydrostatic driving stress for one local (p,f) state.
   *
   * The default adds additional_porosity_pressure, when supplied, and treats that property as
   * independent of the local porosity unknown. Derived models may provide a porosity-dependent
   * pressure closure and its analytical derivative.
   */
  virtual HydrostaticStressState
  evaluateHydrostaticStress(const GenericReal<is_ad> & matrix_hydro_stress,
                            const GenericReal<is_ad> & porosity) const;

  /** True when the derived model supplies two independently evolving pore-porosity states. */
  virtual bool useIndependentPorePorosityKinematics() const { return false; }

  /** Return beginning-of-substep independent pore porosities whose sum is total porosity. */
  virtual PorePorosityState
  independentPorePorosityState(const GenericReal<is_ad> & total_porosity) const
  {
    return {total_porosity, GenericReal<is_ad>(0.0)};
  }

  /** Return the lower bound for one independently evolving pore population. */
  virtual GenericReal<is_ad> independentPorePorosityFloor(
      unsigned int population_index, const PorePorosityState & pore_porosity_begin) const;

  /** Evaluate pressure closure and pressure derivatives for independent pore porosities. */
  virtual HydrostaticStressState
  evaluateIndependentHydrostaticStress(const GenericReal<is_ad> & matrix_hydro_stress,
                                       const PorePorosityState & pore_porosity) const;

  /** Commit one converged independent pore-population state. */
  virtual void independentPorePorosityStateAccepted(
      const GenericRankTwoTensor<is_ad> & inelastic_strain_increment,
      const PorePorosityState & pore_porosity);

  /** Return the number of active pore populations, including the legacy one-population form. */
  unsigned int hydrostaticStressPopulationCount(const HydrostaticStressState & state) const;

  /** Return one active pore population, including the legacy one-population form. */
  HydrostaticStressPopulation hydrostaticStressPopulation(const HydrostaticStressState & state,
                                                          unsigned int index) const;

  /** Validate pore-population fractions and pressure states before constitutive use. */
  void validateHydrostaticStressState(const HydrostaticStressState & state,
                                      const char * stage) const;

  /** Optional porosity waypoint used only to globalize an upward reduced solve. */
  virtual std::optional<Real> reducedPorositySearchTarget() const { return std::nullopt; }

  /** Hook called once after a converged local porosity state has been committed. */
  virtual void
  porosityStateAccepted(const GenericRankTwoTensor<is_ad> & /*inelastic_strain_increment*/,
                        const GenericReal<is_ad> & /*porosity*/)
  {
  }

  /** Validate and, within tolerance, clamp the beginning-of-substep porosity to its floor. */
  GenericReal<is_ad> boundedBeginningPorosity() const;

  /** Configured lower bound used by the local porosity active set. */
  Real minimumPorosity() const { return _minimum_porosity; }

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

  LpsHDerivatives computeHDerivatives(Real n, const GenericReal<is_ad> & M) const;
  GenericReal<is_ad> computeH(const Real n,
                              const GenericReal<is_ad> & gauge_stress,
                              const bool derivative = false) const;
  GenericReal<is_ad> computeGaugeResidual(const GenericReal<is_ad> & equiv_stress,
                                          const GenericReal<is_ad> & trial_gauge,
                                          const HydrostaticStressState & hydrostatic_stress,
                                          const GenericReal<is_ad> & porosity,
                                          const CreepLaw & law,
                                          GenericReal<is_ad> & derivative) const;

  /// Compute the gauge stress for a specific creep mechanism.
  GenericReal<is_ad> computeGaugeStress(const GenericReal<is_ad> & equiv_stress,
                                        const HydrostaticStressState & hydrostatic_stress,
                                        const GenericReal<is_ad> & porosity,
                                        const CreepLaw & law);

  /// Store one converged per-law gauge stress.
  void setGaugeStress(std::size_t law_index, const GenericReal<is_ad> & gauge_stress);

  /// Evaluate and store all gauge-stress diagnostics at one converged constitutive state.
  void setGaugeStresses(const GenericReal<is_ad> & equiv_stress,
                        const HydrostaticStressState & hydrostatic_stress,
                        const GenericReal<is_ad> & porosity);

  /**
   * Evaluate the summed LPS creep response without modifying material properties. The returned
   * tensor derivatives are with respect to common matrix hydrostatic stress, q, and total f.
   */
  LpsCreepResponse evaluateLpsCreepResponse(const HydrostaticStressState & hydrostatic_stress,
                                            const GenericReal<is_ad> & equiv_stress,
                                            const GenericRankTwoTensor<is_ad> & dev_direction,
                                            const GenericReal<is_ad> & porosity);

  /// Analytical first and second partial derivatives of one LPS gauge residual.
  LpsDerivatives computeLpsDerivatives(const GenericReal<is_ad> & gauge_stress,
                                       const HydrostaticStressState & hydrostatic_stress,
                                       const GenericReal<is_ad> & equiv_stress,
                                       const GenericReal<is_ad> & porosity,
                                       const CreepLaw & law) const;

  IndependentLpsDerivatives
  computeIndependentLpsDerivatives(const GenericReal<is_ad> & gauge_stress,
                                   const HydrostaticStressState & hydrostatic_stress,
                                   const GenericReal<is_ad> & equiv_stress,
                                   const PorePorosityState & pore_porosity,
                                   const CreepLaw & law) const;

  IndependentLpsCreepResponse
  evaluateIndependentLpsCreepResponse(const HydrostaticStressState & hydrostatic_stress,
                                      const GenericReal<is_ad> & equiv_stress,
                                      const GenericRankTwoTensor<is_ad> & dev_direction,
                                      const PorePorosityState & pore_porosity);

  /// Matrix hydrostatic stress for the spherical porous formulation.
  GenericReal<is_ad> matrixHydroStress(const GenericRankTwoTensor<is_ad> & stress) const;

  /// Apply the optional porosity pressure to an explicitly supplied matrix hydrostatic stress.
  GenericReal<is_ad> effectiveHydroStress(const GenericReal<is_ad> & matrix_hydro_stress) const;
  /// True when either deviatoric or hydrostatic stress can drive the porous viscoplastic response.
  bool hasViscoplasticDrive(const GenericReal<is_ad> & equiv_stress,
                            const HydrostaticStressState & hydrostatic_stress,
                            const GenericReal<is_ad> & porosity) const;
  /// Positive stress scale used to initialize and bound the gauge-stress Newton solve.
  GenericReal<is_ad> gaugeStressScale(const GenericReal<is_ad> & equiv_stress,
                                      const HydrostaticStressState & hydrostatic_stress) const;
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
                                               const HydrostaticStressState & hydrostatic_stress,
                                               const GenericReal<is_ad> & porosity);
  /// Convert a predicted full-step effective inelastic increment to a local substep count.
  unsigned int computeRequiredSubsteps(Real effective_inelastic_strain_increment) const;
  /// Estimate adaptive substeps from the previous accepted global-step effective inelastic rate.
  unsigned int estimateAdaptiveNumberSubstepsFromHistory() const;
  /// Store the converged current global-step effective inelastic rate for use after timestep
  /// acceptance.
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
  void updateStateSubstepInternal(GenericRankTwoTensor<is_ad> & strain_increment,
                                  GenericRankTwoTensor<is_ad> & inelastic_strain_increment,
                                  GenericRankTwoTensor<is_ad> & stress_new,
                                  const GenericRankFourTensor<is_ad> & elasticity_tensor,
                                  const RankTwoTensor & elastic_strain_old,
                                  unsigned int total_number_substeps);

private:
  /** Caller-owned and material state restored when one constitutive attempt is rejected. */
  struct ConstitutiveStateSnapshot
  {
    GenericRankTwoTensor<is_ad> strain_increment;
    GenericRankTwoTensor<is_ad> inelastic_strain_increment;
    GenericRankTwoTensor<is_ad> stress;
    GenericReal<is_ad> intermediate_porosity = 0.0;
    GenericReal<is_ad> hydro_stress = 0.0;
    GenericReal<is_ad> gauge_stress = 0.0;
    std::vector<GenericReal<is_ad>> gauge_stresses;
  };

  ConstitutiveStateSnapshot
  captureConstitutiveState(const GenericRankTwoTensor<is_ad> & strain_increment,
                           const GenericRankTwoTensor<is_ad> & inelastic_strain_increment,
                           const GenericRankTwoTensor<is_ad> & stress) const;
  void restoreConstitutiveState(const ConstitutiveStateSnapshot & snapshot,
                                GenericRankTwoTensor<is_ad> & strain_increment,
                                GenericRankTwoTensor<is_ad> & inelastic_strain_increment,
                                GenericRankTwoTensor<is_ad> & stress);

  enum LocalVariableIndex : unsigned int
  {
    P_INDEX,
    Q_INDEX,
    F_INDEX,
    LOCAL_SYSTEM_SIZE
  };

  using LocalJacobian =
      std::array<std::array<GenericReal<is_ad>, LOCAL_SYSTEM_SIZE>, LOCAL_SYSTEM_SIZE>;
  using ScaledLocalJacobian = std::array<std::array<Real, LOCAL_SYSTEM_SIZE>, LOCAL_SYSTEM_SIZE>;
  using LocalResidual = std::array<GenericReal<is_ad>, LOCAL_SYSTEM_SIZE>;

  /** Porosity-corrected isotropic elasticity and its derivative with respect to total porosity. */
  struct PorosityElasticityState
  {
    GenericRankFourTensor<is_ad> tensor;
    GenericRankFourTensor<is_ad> dporosity;
  };

  bool porosityDependentElasticityEnabled() const;
  PorosityElasticityState
  evaluatePorosityElasticity(const GenericRankFourTensor<is_ad> & dense_elasticity_tensor,
                             const GenericReal<is_ad> & porosity) const;

  /** Trial data, numerical scales, and tolerances shared by one local constitutive solve. */
  struct LocalSolveContext
  {
    const GenericReal<is_ad> & p_trial;
    const GenericRankTwoTensor<is_ad> & trial_dev_stress;
    const GenericReal<is_ad> & trial_equiv_stress;
    const GenericReal<is_ad> & porosity_begin;
    const GenericRankTwoTensor<is_ad> & trial_elastic_strain_increment;
    const GenericRankTwoTensor<is_ad> & elastic_strain_old;
    const GenericRankFourTensor<is_ad> & elasticity_tensor;

    // Newton-variable and line-search scales are numerical-conditioning choices. Porosity
    // convergence has its own physical scale so merit balancing cannot loosen the accepted Rf
    // tolerance.
    Real p_scale = 1.0;
    Real q_scale = 1.0;
    Real porosity_variable_scale = 1.0;
    Real porosity_merit_scale = 1.0;
    Real porosity_convergence_scale = 1.0;
    Real reduced_probe_tolerance = 0.0;
    Real reduced_final_tolerance = 0.0;
  };

  enum class PorosityBranch
  {
    FREE,
    FLOOR
  };

  enum class LocalResidualScope
  {
    MECHANICAL,
    COUPLED
  };

  struct LocalCoordinates
  {
    GenericReal<is_ad> p = 0.0;
    GenericReal<is_ad> q = 0.0;
    GenericReal<is_ad> f = 0.0;
  };

  struct LocalPoint
  {
    GenericReal<is_ad> p = 0.0;
    GenericReal<is_ad> q = 0.0;
    GenericReal<is_ad> f = 0.0;
    PorosityBranch porosity_branch = PorosityBranch::FREE;
    LocalResidual residual{};
    GenericRankTwoTensor<is_ad> inelastic_strain_increment;
    GenericReal<is_ad> effective_inelastic_strain_increment = 0.0;
    HydrostaticStressState hydrostatic_stress;
    LocalJacobian jacobian{};

    LocalCoordinates coordinates() const { return {p, q, f}; }
    bool porosityFloorActive() const { return porosity_branch == PorosityBranch::FLOOR; }
  };

  struct LocalSolveResult
  {
    LocalPoint point;
    bool activate_floor = false;
    Real porosity_error_bound = std::numeric_limits<Real>::infinity();
  };

  struct ReducedPorosityBracket
  {
    explicit ReducedPorosityBracket(const LocalPoint & lower_point) : lower(lower_point) {}

    bool complete() const { return upper.has_value(); }

    LocalPoint lower;
    std::optional<LocalPoint> upper;
  };

  struct ReducedPorosityTangent
  {
    GenericReal<is_ad> dp_df = 0.0;
    GenericReal<is_ad> dq_df = 0.0;
    GenericReal<is_ad> drhat_df = 0.0;
  };

  template <typename T>
  void validateFiniteValue(const T & value,
                           const char * state_name,
                           const char * stage,
                           const char * field) const
  {
    const auto raw = MetaPhysicL::raw_value(value);
    if (!std::isfinite(raw))
      mooseException("In ",
                     this->_name,
                     ": nonfinite ",
                     state_name,
                     " during ",
                     stage,
                     " at qp ",
                     this->_qp,
                     ". ",
                     field,
                     " = ",
                     raw,
                     ".");
  }

  template <typename Tensor>
  void validateFiniteTensor(const Tensor & tensor,
                            const char * state_name,
                            const char * stage,
                            const char * field) const
  {
    for (auto i = 0u; i < 3; ++i)
      for (auto j = 0u; j < 3; ++j)
        validateFiniteValue(tensor(i, j), state_name, stage, field);
  }

  LocalPoint evaluateLocalPoint(const LocalCoordinates & coordinates,
                                const LocalSolveContext & context,
                                PorosityBranch porosity_branch);
  LocalResidual scaledResidual(const LocalResidual & residual,
                               const LocalSolveContext & context) const;
  Real convergenceResidualNorm(const LocalResidual & residual,
                               const LocalSolveContext & context) const;
  Real residualNorm(const LocalResidual & residual,
                    LocalResidualScope scope = LocalResidualScope::COUPLED) const;
  void validateFiniteLocalPoint(const LocalPoint & point, const char * stage) const;
  ScaledLocalJacobian scaledJacobian(const LocalJacobian & jacobian,
                                     const LocalSolveContext & context) const;
  LocalPoint reconstructImplicitSensitivity(const LocalPoint & point,
                                            const LocalSolveContext & context);
  RankFourTensor computeConsistentTangent(const LocalPoint & point,
                                          const LocalSolveContext & context) const;
  std::optional<LocalPoint> backtrackingLineSearch(const LocalPoint & point,
                                                   const LocalResidual & correction_scaled,
                                                   Real initial_alpha,
                                                   LocalResidualScope residual_scope,
                                                   const LocalSolveContext & context);
  void initializeLocalSolveScales(LocalSolveContext & context) const;
  bool denseLimitActive(const LocalSolveContext & context) const;
  LocalPoint evaluateDenseLimitPoint(const GenericReal<is_ad> & q,
                                     const LocalSolveContext & context) const;
  LocalPoint solveDenseLimit(const LocalSolveContext & context) const;
  LocalPoint solvePorosityActiveSet(const LocalSolveContext & context,
                                    bool & reduced_porosity_attempted);
  void commitLocalPoint(const LocalPoint & point,
                        const LocalSolveContext & context,
                        GenericRankTwoTensor<is_ad> & elastic_strain_increment,
                        GenericRankTwoTensor<is_ad> & inelastic_strain_increment,
                        GenericRankTwoTensor<is_ad> & stress,
                        GenericReal<is_ad> & effective_inelastic_strain_increment);
  std::optional<LocalPoint> solveMechanicalAtFixedPorosity(const LocalCoordinates & seed,
                                                           Real tolerance,
                                                           const LocalSolveContext & context);
  LocalPoint verifyConvergedPoint(const LocalPoint & point, const LocalSolveContext & context);
  LocalPoint verifyReducedConvergedPoint(const LocalSolveResult & reduced,
                                         const LocalSolveContext & context);
  GenericReal<is_ad> impliedPorosity(const LocalPoint & point) const;
  bool freeIncrementReachesPorosityFloor(const LocalPoint & point) const;
  Real reducedPorosityResidual(const LocalPoint & point) const;
  bool reducedPointConverged(const LocalPoint & point, const LocalSolveContext & context) const;
  std::optional<LocalPoint> solveReducedPoint(const LocalCoordinates & seed,
                                              const LocalSolveContext & context,
                                              Real & best_abs_rf_scaled);
  LocalCoordinates reducedBracketMidpoint(const LocalPoint & lower, const LocalPoint & upper) const;
  std::optional<LocalPoint> recoverReducedPorosityRootBeforeFold(ReducedPorosityBracket & bracket,
                                                                 const LocalPoint & fold_upper,
                                                                 const LocalSolveContext & context,
                                                                 Real & best_abs_rf_scaled);
  std::optional<LocalPoint> discoverReducedPorosityBracket(ReducedPorosityBracket & bracket,
                                                           const LocalSolveContext & context,
                                                           Real & best_abs_rf_scaled);
  std::optional<LocalSolveResult> solveReducedPorosityBracket(ReducedPorosityBracket & bracket,
                                                              const LocalSolveContext & context,
                                                              Real & best_abs_rf_scaled);
  std::optional<ReducedPorosityTangent>
  computeReducedPorosityTangent(const LocalJacobian & jacobian) const;
  std::optional<LocalSolveResult> solveReducedPorosityDownward(LocalPoint upper,
                                                               const LocalSolveContext & context,
                                                               Real & best_abs_rf_scaled);
  std::optional<LocalSolveResult> solveReducedPorosityUpward(const LocalPoint & lower,
                                                             const LocalSolveContext & context,
                                                             Real & best_abs_rf_scaled);
  std::optional<LocalSolveResult> solveReducedPorosity(const LocalPoint & point,
                                                       const LocalSolveContext & context,
                                                       bool & reduced_porosity_attempted);
  LocalSolveResult solveCoupledNewton(LocalPoint point,
                                      const LocalSolveContext & context,
                                      bool & reduced_porosity_attempted);
  LocalSolveResult recoverFailedCoupledLineSearch(const LocalPoint & point,
                                                  const LocalSolveContext & context,
                                                  bool & reduced_porosity_attempted);
  std::optional<LocalSolveResult> tryReducedPorosityRecovery(const LocalPoint & point,
                                                             const LocalSolveContext & context,
                                                             bool & reduced_porosity_attempted);
  bool adaptiveSubstepRefinementAvailable() const;
  [[noreturn]] void throwCoupledLineSearchFailure(const LocalPoint & point,
                                                  const LocalSolveContext & context,
                                                  bool reduced_porosity_attempted);

  enum IndependentLocalVariableIndex : unsigned int
  {
    INDEPENDENT_P_INDEX,
    INDEPENDENT_Q_INDEX,
    PORE_POROSITY_0_INDEX,
    PORE_POROSITY_1_INDEX
  };

  using IndependentLocalResidual = std::array<GenericReal<is_ad>, INDEPENDENT_LOCAL_SYSTEM_SIZE>;
  using IndependentLocalJacobian =
      std::array<std::array<GenericReal<is_ad>, INDEPENDENT_LOCAL_SYSTEM_SIZE>,
                 INDEPENDENT_LOCAL_SYSTEM_SIZE>;
  using IndependentScaledLocalJacobian =
      std::array<std::array<Real, INDEPENDENT_LOCAL_SYSTEM_SIZE>, INDEPENDENT_LOCAL_SYSTEM_SIZE>;

  struct IndependentLocalCoordinates
  {
    GenericReal<is_ad> p = 0.0;
    GenericReal<is_ad> q = 0.0;
    PorePorosityState pore_porosity{};
  };

  struct IndependentLocalSolveContext
  {
    const GenericReal<is_ad> & p_trial;
    const GenericRankTwoTensor<is_ad> & trial_dev_stress;
    const GenericReal<is_ad> & trial_equiv_stress;
    PorePorosityState pore_porosity_begin{};
    const GenericRankTwoTensor<is_ad> & trial_elastic_strain_increment;
    const GenericRankTwoTensor<is_ad> & elastic_strain_old;
    const GenericRankFourTensor<is_ad> & elasticity_tensor;
    Real p_scale = 1.0;
    Real q_scale = 1.0;
    std::array<Real, MAX_HYDROSTATIC_STRESS_POPULATIONS> porosity_variable_scale{{1.0, 1.0}};
    std::array<Real, MAX_HYDROSTATIC_STRESS_POPULATIONS> porosity_merit_scale{{1.0, 1.0}};
    std::array<Real, MAX_HYDROSTATIC_STRESS_POPULATIONS> porosity_convergence_scale{{1.0, 1.0}};
  };

  struct IndependentLocalPoint
  {
    GenericReal<is_ad> p = 0.0;
    GenericReal<is_ad> q = 0.0;
    PorePorosityState pore_porosity{};
    std::array<bool, MAX_HYDROSTATIC_STRESS_POPULATIONS> floor_active{{false, false}};
    IndependentLocalResidual residual{};
    IndependentLocalJacobian jacobian{};
    GenericRankTwoTensor<is_ad> inelastic_strain_increment;
    GenericReal<is_ad> effective_inelastic_strain_increment = 0.0;
    PorePorosityState raw_population_porosity_increment{};
    HydrostaticStressState hydrostatic_stress;

    IndependentLocalCoordinates coordinates() const { return {p, q, pore_porosity}; }
  };

  IndependentLocalPoint evaluateIndependentLocalPoint(
      const IndependentLocalCoordinates & coordinates,
      const IndependentLocalSolveContext & context,
      const std::array<bool, MAX_HYDROSTATIC_STRESS_POPULATIONS> & floor_active);
  void initializeIndependentLocalSolveScales(IndependentLocalSolveContext & context) const;
  IndependentLocalResidual
  scaledIndependentResidual(const IndependentLocalResidual & residual,
                            const IndependentLocalSolveContext & context) const;
  Real independentConvergenceResidualNorm(const IndependentLocalResidual & residual,
                                          const IndependentLocalSolveContext & context) const;
  IndependentScaledLocalJacobian
  scaledIndependentJacobian(const IndependentLocalJacobian & jacobian,
                            const IndependentLocalSolveContext & context) const;
  void validateFiniteIndependentLocalPoint(const IndependentLocalPoint & point,
                                           const char * stage) const;
  std::optional<IndependentLocalPoint>
  independentBacktrackingLineSearch(const IndependentLocalPoint & point,
                                    const IndependentLocalResidual & correction_scaled,
                                    Real initial_alpha,
                                    const IndependentLocalSolveContext & context);
  IndependentLocalPoint solveIndependentCoupledNewton(
      IndependentLocalPoint point,
      const IndependentLocalSolveContext & context,
      std::array<bool, MAX_HYDROSTATIC_STRESS_POPULATIONS> & activate_floor);
  IndependentLocalPoint
  solveIndependentPorosityActiveSet(const IndependentLocalSolveContext & context);
  IndependentLocalPoint
  reconstructIndependentImplicitSensitivity(const IndependentLocalPoint & point,
                                            const IndependentLocalSolveContext & context);
  RankFourTensor
  computeIndependentConsistentTangent(const IndependentLocalPoint & point,
                                      const IndependentLocalSolveContext & context) const;
  void commitIndependentLocalPoint(const IndependentLocalPoint & point,
                                   const IndependentLocalSolveContext & context,
                                   GenericRankTwoTensor<is_ad> & elastic_strain_increment,
                                   GenericRankTwoTensor<is_ad> & inelastic_strain_increment,
                                   GenericRankTwoTensor<is_ad> & stress,
                                   GenericReal<is_ad> & effective_inelastic_strain_increment);
  void updateStateOneStepIndependent(GenericRankTwoTensor<is_ad> & elastic_strain_increment,
                                     GenericRankTwoTensor<is_ad> & inelastic_strain_increment,
                                     GenericRankTwoTensor<is_ad> & stress,
                                     const GenericRankFourTensor<is_ad> & elasticity_tensor,
                                     const GenericRankTwoTensor<is_ad> & elastic_strain_old,
                                     GenericReal<is_ad> & effective_inelastic_strain_increment);

protected:
  /// Equivalent von Mises stress of one deviatoric stress tensor.
  static GenericReal<is_ad> equivalentStress(const GenericRankTwoTensor<is_ad> & dev_stress);

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
    HydrostaticStressState hydrostatic_stress;
    GenericReal<is_ad> porosity = 0.0;
    const CreepLaw * law = nullptr;
  };
  GaugeSolveState _gauge_solve_state;
  /// Rank two identity tensor
  const RankTwoTensor _identity_two;

  /// Container for dF/dLambda
  GenericReal<is_ad> _derivative;

private:
  bool _compute_consistent_tangent;
  RankFourTensor _last_consistent_tangent;

  /// Linear factor a_E in E(f) = E_dense (1 - a_E f), evaluated inside the local LPS solve.
  const Real _youngs_modulus_porosity_factor;
  /// Linear factor a_nu in nu(f) = nu_dense (1 - a_nu f), evaluated inside the local LPS solve.
  const Real _poissons_ratio_porosity_factor;

  const Real _minimum_porosity;
  const Real _porosity_bound_tolerance;
  const Real _local_newton_tolerance;
  const Real _local_newton_stagnation_tolerance;
  const unsigned int _local_newton_max_iterations;
  const Real _local_newton_relaxation;
  const unsigned int _local_newton_max_backtracks;
  const Real _local_porosity_scale_floor;
  const unsigned int _reduced_porosity_max_probes;
  const Real _reduced_porosity_probe_growth;
  const unsigned int _reduced_porosity_root_max_iterations;

protected:
  usingViscoplasticityStressUpdateBaseMembers;
};

using PorousViscoplasticityStressUpdate = PorousViscoplasticityStressUpdateTempl<false>;
using ADPorousViscoplasticityStressUpdate = PorousViscoplasticityStressUpdateTempl<true>;
