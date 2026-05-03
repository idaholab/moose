#pragma once

#include "RhieChowMassFlux.h"

class ElemInfo;
class LinearSystem;

/**
 * Rhie-Chow face-flux provider extended with additional reduced-pressure source
 * fluxes for large-density-ratio sharp-interface coupling.
 *
 * Sign convention:
 * - The face functors published here are PRESSURE-EQUATION SOURCE FLUXES in the
 *   same sign convention as the stock HbyA functor used by LinearFVDivergence.
 * - The physical face mass-flux correction is therefore the negative of the
 *   published source functor. This mirrors the stock MOOSE relation
 *     face_mass_flux = -HbyA + pressure_diffusion_flux.
 */
class SharpInterfaceRhieChowMassFlux : public RhieChowMassFlux
{
public:
  struct MomentumProbeSample
  {
    bool valid = false;
    dof_id_type elem_id = DofObject::invalid_id;
    Point centroid;
    Real rho = 0.0;
    Real pressure = 0.0;
    RealVectorValue grad_p;
    RealVectorValue pressure_force;
    RealVectorValue body_force;
    RealVectorValue total_force;
    RealVectorValue hbya_raw;
    RealVectorValue predictor_velocity;
    RealVectorValue pressure_coupled_delta_velocity;
    RealVectorValue writeback_velocity;
    RealVectorValue current_velocity;
  };

  static InputParameters validParams();

  SharpInterfaceRhieChowMassFlux(const InputParameters & params);

  void meshChanged() override;
  void initialSetup() override;
  void initialize() override;
  void initFaceMassFlux() override;
  void computeFaceMassFlux() override;
  void computeCellVelocity() override;
  Real getMassFlux(const FaceInfo & fi) const override;
  Real getVolumetricFaceFlux(const FaceInfo & fi) const override;
  Real getVolumetricFaceFlux(const Moose::FV::InterpMethod m,
                             const FaceInfo & fi,
                             const Moose::StateArg & time,
                             const THREAD_ID tid,
                             bool subtract_mesh_velocity) const override;
  void setUseVOFRhoPhi(const bool use_vof_rho_phi) { _use_vof_rho_phi = use_vof_rho_phi; }
  bool useVOFRhoPhi() const { return _use_vof_rho_phi; }
  void freezeOuterIterationConvectiveState();
  void clearOuterIterationConvectiveState();
  bool hasOuterIterationConvectiveState() const { return _outer_iteration_convective_state_valid; }
  bool useFaceBasedPredictorBodyForce() const { return _use_face_based_predictor_body_force; }
  bool seedHydrostaticPressure(LinearSystem & pressure_system,
                               const dof_id_type pressure_pin_dof,
                               const Real pressure_pin_value) const;
  void addMomentumPredictorExplicitForcing(const unsigned int system_i,
                                           NumericVector<Number> & rhs) const override;
  void addMomentumPredictorBodyForceForcing(const unsigned int system_i,
                                            NumericVector<Number> & rhs) const override;
  void updateVelocityBoundaryState() override;

  /// Update the additional pressure-equation source-flux functors before the pressure solve.
  void updateAdditionalPressureFluxFunctors(const bool with_updated_pressure, const bool verbose);

  /// Apply the physical counterpart of the additional source fluxes to the final face mass flux.
  void applyAdditionalFaceMassFluxCorrection();
  void computeProvisionalCellVelocity();

  void setSuppressExplicitHydrostaticPressureFlux(
      const bool suppress_explicit_hydrostatic_pressure_flux)
  {
    _suppress_explicit_hydrostatic_pressure_flux = suppress_explicit_hydrostatic_pressure_flux;
  }
  bool suppressExplicitHydrostaticPressureFlux() const
  {
    return _suppress_explicit_hydrostatic_pressure_flux;
  }
  void setSuppressStartupPressurePredictorFluxSources(
      const bool suppress_startup_pressure_predictor_flux_sources)
  {
    _suppress_startup_pressure_predictor_flux_sources =
        suppress_startup_pressure_predictor_flux_sources;
  }
  bool suppressStartupPressurePredictorFluxSources() const
  {
    return _suppress_startup_pressure_predictor_flux_sources;
  }
  void auditRepresentativeHorizontalFaceReconstruction();
  void auditRepresentativePredictorBodyForce() const;
  void collectMomentumProbeSamples(const std::vector<const ElemInfo *> & elem_infos,
                                   std::vector<MomentumProbeSample> & samples) const;
  void clearPressureCoupledVelocityCorrectionAudit();
  void printPressureCoupledVelocityCorrectionAudit(const std::string & label) const;
  Real rawRhieChowMassFlux(const FaceInfo & fi) const;
  Real predictorOperatorFaceMassFlux(const FaceInfo & fi, const Moose::StateArg & time_arg) const;
  Real pressureCoupledWritebackMassFlux(const FaceInfo & fi) const;
  Real maxVolumeFractionCourant(const Real dt) const;
  RealVectorValue pressureCoupledCellVelocityDelta(const ElemInfo & elem_info,
                                                   const Moose::StateArg & time_arg) const;
  Real predictorVelocityComponent(const ElemInfo & elem_info, const unsigned int component) const;
  bool hasVOFRhoPhiFunctor() const { return _vof_rho_phi != nullptr; }
  Real vofRhoPhiMassFlux(const FaceInfo & fi) const;
  const std::array<dof_id_type, 3> & watchedInternalFaceIds() const
  {
    return _last_pressure_coupled_velocity_worst_internal_face_ids;
  }

protected:
  using FaceScalarField =
      FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>>;
  using FaceVectorField =
      FaceCenteredMapFunctor<RealVectorValue, std::unordered_map<dof_id_type, RealVectorValue>>;

  void initializeAdditionalPressureFluxStorage(const bool preserve_corrected_face_phi = false);
  void rebuildSharpInterfaceFaceInfo();
  void cacheCurrentCorrectedVolumetricFlux();
  void syncPredictorConvectiveStateFromCurrentFluxes();
  void updatePredictorOperatorPhiField(const Moose::StateArg & time_arg);
  Real transportMassFluxDensityFromVolumetricPhi(const FaceInfo * fi,
                                                 const Real volumetric_phi,
                                                 const Moose::StateArg & time_arg) const;
  Real transportIntegratedRhoPhiFromVolumetricPhi(const FaceInfo * fi,
                                                  const Real volumetric_phi,
                                                  const Moose::StateArg & time_arg) const;

  Moose::FaceArg makeCenteredFaceArg(const FaceInfo * fi,
                                     const Moose::StateArg * limiter_state = nullptr) const;

  Real interpolateFaceDensity(const FaceInfo * fi, const Moose::StateArg & time_arg) const;
  Real predictorFaceDensity(const FaceInfo * fi, const Moose::StateArg & time_arg) const;

  RealVectorValue interpolateFaceRawAinv(const FaceInfo * fi) const;
  RealVectorValue interpolateFaceRau(const FaceInfo * fi) const;

  RealVectorValue evaluateFaceVectorFunctor(const Moose::Functor<RealVectorValue> * functor,
                                            const FaceInfo * fi,
                                            const Moose::StateArg & time_arg,
                                            const Moose::StateArg * limiter_state) const;
  RealVectorValue evaluateBoundaryAwareVectorFunctor(
      const Moose::Functor<RealVectorValue> * functor,
      const FaceInfo * fi,
      const Moose::StateArg & time_arg) const;
  RealVectorValue interpolateCellVectorFunctorToFace(
      const Moose::Functor<RealVectorValue> * functor,
      const FaceInfo * fi,
      const Moose::StateArg & time_arg) const;
  RealVectorValue interpolateCellBodyForceDensityToFace(
      const Moose::Functor<RealVectorValue> * acceleration_functor,
      const FaceInfo * fi,
      const Moose::StateArg & time_arg) const;
  Real evaluateFaceScalarFunctor(const Moose::Functor<Real> * functor,
                                 const FaceInfo * fi,
                                 const Moose::StateArg & time_arg,
                                 const Moose::StateArg * limiter_state) const;
  Real evaluateBoundaryAwareScalarFunctor(const Moose::Functor<Real> * functor,
                                          const FaceInfo * fi,
                                          const Moose::StateArg & time_arg) const;

  Real projectPhysicalMassFluxDensity(const Real face_rho,
                                      const RealVectorValue & face_ainv_raw,
                                      const RealVectorValue & face_acceleration,
                                      const RealVectorValue & face_normal) const;
  Real computeFaceNormalRawAinv(const RealVectorValue & face_ainv_raw,
                                const RealVectorValue & face_normal) const;
  Real massFluxDensityToVolumetricNormalFlux(const FaceInfo * fi,
                                             const Real mass_flux_density) const;
  Real computeFaceNormalDensityGradient(const FaceInfo * fi, const Moose::StateArg & time_arg) const;
  Real computeHydrostaticFaceMassFlux(const FaceInfo * fi,
                                      const Real face_rho,
                                      const RealVectorValue & face_ainv_raw,
                                      const RealVectorValue & face_normal,
                                      const Moose::StateArg & time_arg) const;
  RealVectorValue evaluateLegacyMomentumPredictorBodyForceDensity(
      const ElemInfo * elem_info, const Moose::StateArg & time_arg) const;
  bool populateMomentumPredictorPressureForceFaceField(FaceVectorField & face_field,
                                                       const Moose::StateArg & time_arg);
  RealVectorValue evaluateFaceBasedMomentumPredictorPressureForceDensity(
      const ElemInfo * elem_info,
      const Moose::StateArg & time_arg,
      const FaceVectorField * face_field) const;
  RealVectorValue evaluateFaceBasedMomentumPredictorBodyForceDensity(
      const ElemInfo * elem_info,
      const Moose::StateArg & time_arg,
      const FaceVectorField * face_field) const;
  bool populateMomentumPredictorBodyForceFaceField(FaceVectorField & face_field,
                                                   const Moose::StateArg & time_arg) const;
  RealVectorValue evaluateMomentumPredictorBodyForceDensity(
      const ElemInfo * elem_info,
      const Moose::StateArg & time_arg,
      const FaceVectorField * face_field) const;
  Real pressureVelocityWritebackFluxDensity(const FaceInfo * fi) const;
  void updatePressureCoupledVelocityCorrectionFaceField(const Moose::StateArg & time_arg);
  RealVectorValue reconstructMatchedPressureCoupledCellCorrectionSource(
      const ElemInfo * elem_info, const Moose::StateArg & time_arg) const;
  RealVectorValue reconstructPressureCoupledCellCorrectionSource(
      const ElemInfo * elem_info, const Moose::StateArg & time_arg) const;
  RealVectorValue matchedSourcePressureCoupledCellVelocityDelta(
      const ElemInfo * elem_info, const Moose::StateArg & time_arg) const;
  RealVectorValue reconstructPressureCoupledCellVelocityDelta(
      const ElemInfo * elem_info, const Moose::StateArg & time_arg) const;
  void populateVelocityDeltaFromPressurePotential(
      const NumericVector<Number> & potential_raw,
      std::vector<std::unique_ptr<NumericVector<Number>>> & velocity_delta_raw);
  void solveScalarResidualWritebackCorrection(const Moose::StateArg & time_arg);
  void solveGlobalWritebackProjection(const Moose::StateArg & time_arg);
  void updateCorrectedFaceVelocityField(const Moose::StateArg & time_arg);
  void printPressureCoupledVelocityCorrectionInternalFaceAudit(const std::string & label) const;
  bool useConstrainedBoundaryPredictorState(const FaceInfo * fi) const;
  RealVectorValue referenceFaceVelocityState(const FaceInfo * fi,
                                             const Moose::StateArg & time_arg) const;
  Real referenceFaceMassFluxState(const FaceInfo * fi, const Moose::StateArg & time_arg) const;
  bool shouldUseCorrectedBoundaryVelocityState(const FaceInfo * fi) const;
  Real pressureBoundaryTargetFlux(const FaceInfo * fi,
                                  const Moose::StateArg & time_arg) const override;
  Real pressureBoundaryNormalAinv(const FaceInfo * fi) const override;

  FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>> _transient_projection_flux;
  FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>> _capillary_hydrostatic_flux;
  FaceCenteredMapFunctor<RealVectorValue, std::unordered_map<dof_id_type, RealVectorValue>>
      _pressure_Ainv;
  FaceScalarField _predictor_convective_mass_flux;
  FaceScalarField _predictor_convective_phi;
  FaceScalarField _predictor_operator_phi;
  FaceScalarField _pressure_predictor_base_phi;
  FaceScalarField _pressure_equation_volumetric_flux;
  FaceScalarField _pressure_correction_phi;
  FaceScalarField _sharp_pressure_predictor_flux;
  FaceScalarField _reference_face_mass_flux_for_writeback;
  FaceScalarField _corrected_face_phi;
  FaceScalarField _outer_iteration_rho_phi;
  FaceScalarField _outer_iteration_phi;
  FaceScalarField _pressure_coupled_velocity_correction_scalar;
  FaceCenteredMapFunctor<RealVectorValue, std::unordered_map<dof_id_type, RealVectorValue>>
      _pressure_coupled_velocity_correction_face;
  FaceCenteredMapFunctor<RealVectorValue, std::unordered_map<dof_id_type, RealVectorValue>>
      _corrected_face_velocity;

  std::vector<const FaceInfo *> _sharp_interface_face_info;

  const bool _add_transient_projection_flux;
  const bool _add_capillary_hydrostatic_flux;
  const bool _use_face_based_predictor_body_force;
  const RealVectorValue _gravity;
  const Point _reference_pressure_point;

  const MooseFunctorName _transient_projection_face_acceleration_name;
  const MooseFunctorName _surface_tension_face_acceleration_name;
  const MooseFunctorName _surface_tension_cell_acceleration_name;
  const MooseFunctorName _hydrostatic_density_gradient_face_acceleration_name;
  const MooseFunctorName _hydrostatic_density_gradient_cell_acceleration_name;
  const MooseFunctorName _vof_rho_phi_name;
  const MooseFunctorName _vof_alpha_phi_limited_name;
  const MooseFunctorName _liquid_density_name;
  const MooseFunctorName _gas_density_name;

  const Moose::Functor<RealVectorValue> * const _transient_projection_face_acceleration;
  const Moose::Functor<RealVectorValue> * const _surface_tension_face_acceleration;
  const Moose::Functor<RealVectorValue> * const _surface_tension_cell_acceleration;
  const Moose::Functor<RealVectorValue> * const _hydrostatic_density_gradient_face_acceleration;
  const Moose::Functor<RealVectorValue> * const _hydrostatic_density_gradient_cell_acceleration;
  const Moose::Functor<Real> * _vof_rho_phi;
  const Moose::Functor<Real> * _vof_alpha_phi_limited;
  const Moose::Functor<Real> * _liquid_density;
  const Moose::Functor<Real> * _gas_density;
  const bool _use_scalar_residual_writeback_correction;
  const Real _scalar_residual_writeback_beta_multiplier;
  const bool _use_global_writeback_projection;
  const Real _global_writeback_projection_beta_multiplier;
  bool _use_vof_rho_phi = false;
  bool _outer_iteration_convective_state_valid = false;
  bool _corrected_face_phi_seeded = false;
  bool _suppress_explicit_hydrostatic_pressure_flux = false;
  bool _suppress_startup_pressure_predictor_flux_sources = false;
  bool _pressure_coupled_velocity_correction_valid = false;
  bool _corrected_face_velocity_valid = false;
  bool _pressure_coupled_velocity_correction_audit_valid = false;
  std::unique_ptr<NumericVector<Number>> _scalar_residual_writeback_potential_raw;
  std::vector<std::unique_ptr<NumericVector<Number>>> _scalar_residual_writeback_velocity_delta_raw;
  bool _scalar_residual_writeback_valid = false;
  std::vector<std::unique_ptr<NumericVector<Number>>> _global_writeback_velocity_delta_raw;
  bool _global_writeback_velocity_delta_valid = false;
  Real _last_pressure_coupled_velocity_delta_l2 = 0.0;
  Real _last_pressure_coupled_velocity_delta_max = 0.0;
  dof_id_type _last_pressure_coupled_velocity_worst_elem_id = 0;
  Point _last_pressure_coupled_velocity_worst_centroid;
  mutable std::array<dof_id_type, 3> _last_pressure_coupled_velocity_worst_internal_face_ids{
      DofObject::invalid_id, DofObject::invalid_id, DofObject::invalid_id};
};
