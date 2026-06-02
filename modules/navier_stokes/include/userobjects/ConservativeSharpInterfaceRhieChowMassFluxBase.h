#pragma once

#include "RhieChowMassFlux.h"
#include "PetscVectorReader.h"
#include <array>

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
class ConservativeSharpInterfaceRhieChowMassFluxBase : public RhieChowMassFlux
{
public:
  struct PressureCorrectionReconstructionDebug
  {
    std::array<Real, 9> normal_matrix{{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}};
    std::array<Real, 3> rhs{{0.0, 0.0, 0.0}};
    std::array<Real, 3> solution{{0.0, 0.0, 0.0}};
    std::array<Real, 3> reference_delta_velocity{{0.0, 0.0, 0.0}};
    std::array<Real, 3> smooth_delta_velocity{{0.0, 0.0, 0.0}};
    std::array<Real, 3> delta_velocity{{0.0, 0.0, 0.0}};
    unsigned int contributing_faces = 0;
    bool singular = false;
  };

  struct MomentumPredictorExplicitForceDebug
  {
    std::array<Real, 3> pressure_force_density{{0.0, 0.0, 0.0}};
    std::array<Real, 3> body_force_density{{0.0, 0.0, 0.0}};
    std::array<Real, 3> cell_body_force_density{{0.0, 0.0, 0.0}};
    std::array<Real, 3> scalar_reconstructed_pressure_force_density{{0.0, 0.0, 0.0}};
    std::array<Real, 3> scalar_reconstructed_body_force_density{{0.0, 0.0, 0.0}};
    std::array<Real, 3> total_force_density{{0.0, 0.0, 0.0}};
    std::array<Real, 3> rhs_contribution{{0.0, 0.0, 0.0}};
    bool face_based_pressure = false;
  };

  struct SharpFaceOperatorState
  {
    Real face_rho = 0.0;
    Real negative_sn_grad_p = 0.0;
    Real hydrostatic_mass_flux_density_raw = 0.0;
    Real normal_raw_ainv = 0.0;
    RealVectorValue face_normal;
    RealVectorValue face_raw_ainv;
  };

  struct DensityNormalGradientDebug
  {
    Real orthogonal_part = 0.0;
    Real base_part = 0.0;
    Real correction_part = 0.0;
    Real limited_correction_part = 0.0;
    Real total = 0.0;
  };

  struct PressureVelocityFaceState
  {
    bool predictor_valid = false;
    Real predictor_transport_phi = 0.0;
    Real pressure_equation_phi = 0.0;
    Real pressure_writeback_phi = 0.0;
    Real corrected_transport_phi = 0.0;
    Real normal_pressure_ainv = 0.0;
    Real writeback_reconstruction_scalar = 0.0;
    RealVectorValue writeback_reconstruction_vector;
  };

  static InputParameters validParams();

  ConservativeSharpInterfaceRhieChowMassFluxBase(const InputParameters & params);

  void meshChanged() override;
  void initialSetup() override;
  void initialize() override;
  void initFaceMassFlux() override;
  void cachePressureEquationFlux() override;
  void computeFaceMassFlux() override;
  void computeCellVelocity() override;
  Real getMassFlux(const FaceInfo & fi) const override;
  Real getVolumetricFaceFlux(const FaceInfo & fi) const override;
  Real getVOFTransportVolumetricFaceFlux(const FaceInfo & fi) const override;
  Real getVolumetricFaceFlux(const Moose::FV::InterpMethod m,
                             const FaceInfo & fi,
                             const Moose::StateArg & time,
                             const THREAD_ID tid,
                             bool subtract_mesh_velocity) const override;
  void commitAcceptedTimestepTransportHistory();
  void freezeVOFTransportState(const bool use_previous_timestep_flux = false);
  void adoptPublishedVOFTransportState();
  void clearVOFTransportState();
  bool seedHydrostaticPressure(LinearSystem & pressure_system,
                               const dof_id_type pressure_pin_dof,
                               const Real pressure_pin_value) const;
  void updateVelocityBoundaryState() override;

  /// Update the additional pressure-equation source-flux functors before the pressure solve.
  void updateAdditionalPressureFluxFunctors(const bool with_updated_pressure, const bool verbose);

  /// Apply the physical counterpart of the additional source fluxes to the final face mass flux.
  void applyAdditionalFaceMassFluxCorrection();
  void computePressureCorrectedCellVelocity();

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
  Real rawRhieChowMassFlux(const FaceInfo & fi) const;
  Real predictorOperatorFaceMassFlux(const FaceInfo & fi, const Moose::StateArg & time_arg) const;
  Real pressureCoupledWritebackMassFlux(const FaceInfo & fi) const;
  Real storedCorrectedFacePhi(const FaceInfo & fi) const;
  Real storedPressurePredictorBasePhi(const FaceInfo & fi) const;
  Real storedPressureEquationVolumetricFlux(const FaceInfo & fi) const;
  Real storedPredictorOperatorPhi(const FaceInfo & fi) const;
  Real storedPressureCorrectionPhi(const FaceInfo & fi) const;
  Real storedVOFTransportPhi(const FaceInfo & fi) const;
  Real storedOuterIterationPhi(const FaceInfo & fi) const;
  Real storedOuterIterationRhoPhiIntegrated(const FaceInfo & fi) const;
  Real storedPredictorConvectivePhi(const FaceInfo & fi) const;
  Real storedPredictorConvectiveMassFlux(const FaceInfo & fi) const;
  Real storedPhigFlux(const FaceInfo & fi) const;
  Real storedCapillaryHydrostaticFlux(const FaceInfo & fi) const;
  Real storedTransientProjectionFlux(const FaceInfo & fi) const;
  Real debugHydrostaticFaceMassFluxDensityRaw(const FaceInfo & fi) const;
  Real debugFaceNormalDensityGradient(const FaceInfo & fi, const Moose::StateArg & time_arg) const;
  Real debugFaceNormalDensityGradientOrthogonalPart(const FaceInfo & fi,
                                                    const Moose::StateArg & time_arg) const;
  Real debugFaceNormalDensityGradientBasePart(const FaceInfo & fi,
                                              const Moose::StateArg & time_arg) const;
  Real debugFaceNormalDensityGradientCorrectionPart(const FaceInfo & fi,
                                                    const Moose::StateArg & time_arg) const;
  Real debugFaceNormalDensityGradientLimitedCorrectionPart(
      const FaceInfo & fi, const Moose::StateArg & time_arg) const;
  Real debugFaceNormalDensityWeightedAinv(const FaceInfo & fi) const;
  Real debugFaceNormalRawAinv(const FaceInfo & fi, const Moose::StateArg & time_arg) const;
  Real debugHydrostaticGh(const FaceInfo & fi) const;
  Real debugElemAlpha(const FaceInfo & fi, const Moose::StateArg & time_arg) const;
  Real debugNeighborAlpha(const FaceInfo & fi, const Moose::StateArg & time_arg) const;
  Real debugElemDensity(const FaceInfo & fi, const Moose::StateArg & time_arg) const;
  Real debugNeighborDensity(const FaceInfo & fi, const Moose::StateArg & time_arg) const;
  Real maxVolumeFractionCourant(const Real dt) const;
  virtual RealVectorValue pressureCoupledCellVelocityDelta(
      const ElemInfo & elem_info, const Moose::StateArg & time_arg) const;
  virtual PressureCorrectionReconstructionDebug
  pressureCorrectionReconstructionDebug(const ElemInfo & elem_info,
                                        const Moose::StateArg & time_arg) const;
  MomentumPredictorExplicitForceDebug
  momentumPredictorExplicitForceDebug(const ElemInfo & elem_info,
                                      const Moose::StateArg & time_arg);
  RealVectorValue reducedPressureMomentumPredictorForceDensity(
      const ElemInfo & elem_info, const Moose::StateArg & time_arg) const;
  Real predictorVelocityComponent(const ElemInfo & elem_info, const unsigned int component) const;
  bool hasVOFRhoPhiFunctor() const { return _vof_rho_phi != nullptr; }
  Real vofRhoPhiIntegrated(const FaceInfo & fi) const;
  Real vofAlphaPhiLimitedIntegrated(const FaceInfo & fi) const;
  Real vofBaseGasRhoPhiIntegrated(const FaceInfo & fi) const;
  Real vofAlphaCorrectionRhoPhiIntegrated(const FaceInfo & fi) const;
  Real debugGasDensityFace(const FaceInfo & fi, const Moose::StateArg & time_arg) const;
  Real debugLiquidDensityFace(const FaceInfo & fi, const Moose::StateArg & time_arg) const;
  void dumpPressureCorrectorFaceDebugCSV(const std::string & path);

protected:
  using FaceScalarField =
      FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>>;
  using FaceVectorField =
      FaceCenteredMapFunctor<RealVectorValue, std::unordered_map<dof_id_type, RealVectorValue>>;

  Real pressureBoundaryTargetFlux(const FaceInfo * fi,
                                  const Moose::StateArg & time_arg) const override;
  Real pressureBoundaryNormalAinv(const FaceInfo * fi) const override;
  Real pressureFaceNormalAinv(const FaceInfo * fi, const Moose::StateArg & time_arg) const;

  void initializeAdditionalPressureFluxStorage(const bool preserve_corrected_face_phi = false);
  void writePressureCorrectedVelocityToMomentumSolution(const Moose::StateArg & time_arg);
  void rebuildSharpInterfaceFaceInfo();
  void cacheCurrentCorrectedVolumetricFlux(
      const Real degenerate_normal_pressure_ainv_tol = 0.0);
  PressureVelocityFaceState pressureVelocityFaceState(
      const FaceInfo * fi,
      const Moose::StateArg & time_arg,
      const Real degenerate_normal_pressure_ainv_tol = 0.0) const;
  Real maxPressureFaceNormalAinv(const Moose::StateArg & time_arg) const;
  Real transportMassFluxDensityFromVolumetricPhi(const FaceInfo * fi,
                                                 const Real volumetric_phi,
                                                 const Moose::StateArg & time_arg) const;
  Real transportIntegratedRhoPhiFromVolumetricPhi(const FaceInfo * fi,
                                                  const Real volumetric_phi,
                                                  const Moose::StateArg & time_arg) const;
  Real transportVolumetricPhiFromMassFluxDensity(const FaceInfo * fi,
                                                 const Real mass_flux_density) const;
  Real transportVolumetricPhiFromIntegratedRhoPhi(const FaceInfo * fi,
                                                  const Real integrated_rho_phi) const;
  Real publishedVOFRhoPhiIntegrated(const FaceInfo * fi, const Moose::StateArg & time_arg) const;
  Real faceAlphaRho(const FaceInfo * fi, const Moose::StateArg & time_arg) const;

  Moose::FaceArg makeCenteredFaceArg(const FaceInfo * fi,
                                     const Moose::StateArg * limiter_state = nullptr) const;

  Real interpolateFaceDensity(const FaceInfo * fi, const Moose::StateArg & time_arg) const;
  Real predictorFaceDensity(const FaceInfo * fi, const Moose::StateArg & time_arg) const;
  virtual Real cellPhysicalVelocityComponent(const ElemInfo & elem_info,
                                             const unsigned int component,
                                             const Moose::StateArg & time_arg) const;
  virtual Real boundaryPhysicalVelocityComponent(const FaceInfo * fi,
                                                 const unsigned int component,
                                                 const Moose::StateArg & time_arg) const;
  Real boundaryPhysicalVolumetricFluxTarget(const FaceInfo * fi,
                                            const Moose::StateArg & time_arg) const;
  Real facePhysicalVelocityComponent(const FaceInfo * fi,
                                     const unsigned int component,
                                     const Moose::StateArg & time_arg) const;
  Real interpolatedPhysicalFaceFlux(const FaceInfo * fi, const Moose::StateArg & time_arg) const;

  RealVectorValue interpolateFaceRawAinv(const FaceInfo * fi) const;
  RealVectorValue interpolateFaceRawAinv(const FaceInfo * fi,
                                         const std::vector<PetscVectorReader> & raw_ainv_readers) const;
  RealVectorValue interpolatePressureFaceRau(
      const FaceInfo * fi, const std::vector<PetscVectorReader> & raw_ainv_readers) const;
  const ElemInfo * sharpInterfaceOneSidedInterpolationOwner(const FaceInfo * fi,
                                                            const Moose::StateArg & time_arg) const;
  void buildSharpFaceRawAinvReaders(
      std::vector<std::unique_ptr<NumericVector<Number>>> & owned_raw_ainv,
      std::vector<PetscVectorReader> & raw_ainv_readers) const;
  void buildSelectedPressureGradientReaders(
      const bool with_updated_pressure,
      std::vector<PetscVectorReader> & pressure_gradient_readers);
  SharpFaceOperatorState buildSharpFaceOperatorState(
      const FaceInfo * fi,
      const Moose::StateArg & time_arg,
      const std::vector<PetscVectorReader> & raw_ainv_readers,
      const std::vector<PetscVectorReader> * pressure_gradient_readers = nullptr) const;

  RealVectorValue evaluateBoundaryAwareVectorFunctor(
      const Moose::Functor<RealVectorValue> * functor,
      const FaceInfo * fi,
      const Moose::StateArg & time_arg) const;
  RealVectorValue interpolateCellVectorFunctorToFace(
      const Moose::Functor<RealVectorValue> * functor,
      const FaceInfo * fi,
      const Moose::StateArg & time_arg) const;
  Real evaluateFaceScalarFunctor(const Moose::Functor<Real> * functor,
                                 const FaceInfo * fi,
                                 const Moose::StateArg & time_arg,
                                 const Moose::StateArg * limiter_state) const;
  Real evaluateCellBasedFaceScalarFunctor(const Moose::Functor<Real> * functor,
                                          const FaceInfo * fi,
                                          const Moose::StateArg & time_arg) const;

  Real projectPhysicalMassFluxDensity(const Real face_rho,
                                      const RealVectorValue & face_ainv_raw,
                                      const RealVectorValue & face_acceleration,
                                      const RealVectorValue & face_normal) const;
  Real computeFaceNormalRawAinv(const RealVectorValue & face_ainv_raw,
                                const RealVectorValue & face_normal) const;
  Real computeDefaultTransientProjectionVolumetricFlux(
      const FaceInfo * fi,
      const Moose::StateArg & time_arg,
      const SharpFaceOperatorState & state) const;
  Real massFluxDensityToVolumetricNormalFlux(const FaceInfo * fi,
                                             const Real mass_flux_density) const;
  Real volumetricNormalFluxToPressureMassFluxDensity(const FaceInfo * fi,
                                                     const Real volumetric_flux) const;
  Real computeDiscretePressureFaceVolumetricFlux(const FaceInfo * fi) const;
  Real computeDiscretePressureFaceFlux(const FaceInfo * fi) const override;
  DensityNormalGradientDebug computeFaceNormalDensityGradientDebug(
      const FaceInfo * fi, const Moose::StateArg & time_arg) const;
  Real computeFaceNormalDensityGradient(const FaceInfo * fi, const Moose::StateArg & time_arg) const;
  Real computeFaceNormalPressureGradient(const FaceInfo * fi,
                                         const Moose::StateArg & time_arg) const;
  Real computeFaceNormalPressureGradient(
      const FaceInfo * fi, const std::vector<PetscVectorReader> & pressure_gradient_readers) const;
  bool useExplicitHydrostaticPredictorForce() const;
  bool useDiscreteHydrostaticPredictorFaceForce() const;
  bool useLegacyHydrostaticPredictorFaceAcceleration() const;
  bool hasHydrostaticPredictorFaceForce() const;
  Real computeDiscreteHydrostaticPredictorFaceNormalForceDensity(
      const FaceInfo * fi, const Moose::StateArg & time_arg) const;
  RealVectorValue evaluateCellMomentumPredictorPressureForceDensity(
      const ElemInfo & elem_info) const;
  RealVectorValue evaluateCellHydrostaticMomentumPredictorBodyForceDensity(
      const ElemInfo * elem_info, const Moose::StateArg & time_arg) const;
  RealVectorValue evaluateLegacyMomentumPredictorBodyForceDensity(
      const ElemInfo * elem_info, const Moose::StateArg & time_arg) const;
  bool populateMomentumPredictorPressureForceFaceField(FaceVectorField & face_field,
                                                       const Moose::StateArg & time_arg) const;
  RealVectorValue reconstructFaceVectorFieldToCellSourceDensity(
      const ElemInfo * elem_info,
      const Moose::StateArg & time_arg,
      const FaceVectorField & face_field) const;
  RealVectorValue evaluateFaceBasedMomentumPredictorBodyForceDensity(
      const ElemInfo * elem_info,
      const Moose::StateArg & time_arg,
      const FaceVectorField * face_field) const;
  bool populateMomentumPredictorBodyForceFaceField(FaceVectorField & face_field,
                                                   const Moose::StateArg & time_arg) const;
  Real pressureVelocityWritebackFluxDensity(const FaceInfo * fi) const;
  void updatePressureCoupledVelocityCorrectionFaceField(const Moose::StateArg & time_arg);
  PressureCorrectionReconstructionDebug reconstructReferenceFaceScalarToCellVectorDebug(
      const ElemInfo * elem_info,
      const Moose::StateArg & time_arg,
      const FaceScalarField & scalar_field) const;
  RealVectorValue reconstructFaceNormalScalarToCellVector(
      const ElemInfo * elem_info,
      const Moose::StateArg & time_arg,
      const FaceScalarField & scalar_field) const;
  RealVectorValue reconstructReferenceStylePressureCoupledCellVelocityDelta(
      const ElemInfo * elem_info, const Moose::StateArg & time_arg) const;
  virtual RealVectorValue reconstructPressureCoupledCellVelocityDelta(
      const ElemInfo * elem_info, const Moose::StateArg & time_arg) const;
  bool useConstrainedBoundaryPredictorState(const FaceInfo * fi) const;

  FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>> _transient_projection_flux;
  FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>> _capillary_hydrostatic_flux;
  FaceScalarField _pressure_equation_volumetric_flux;
  FaceScalarField _pressure_correction_phi;
  FaceScalarField _corrected_face_phi;
  FaceScalarField _previous_timestep_corrected_face_phi;
  FaceScalarField _vof_transport_phi;
  FaceScalarField _debug_update_hydrostatic_face_mass_flux_density_raw;
  FaceScalarField _debug_update_physical_capillary_hydrostatic_flux;
  FaceScalarField _debug_update_hydrostatic_branch_taken;
  FaceScalarField _pressure_coupled_cell_reconstruction_scalar;
  FaceVectorField _pressure_coupled_cell_reconstruction_vector;

  std::vector<const FaceInfo *> _sharp_interface_face_info;

  const bool _add_transient_projection_flux;
  const bool _add_capillary_hydrostatic_flux;
  const bool _apply_pressure_velocity_writeback;
  const bool _apply_pressure_face_flux_correction;
  const RealVectorValue _gravity;
  const Point _reference_pressure_point;
  const Real _near_interface_lower;
  const Real _near_interface_upper;
  const Real _pressure_writeback_face_ainv_relative_tolerance;
  const MooseEnum _density_sn_grad_scheme;
  const Real _density_sn_grad_limiter_coefficient;
  const MooseEnum _hydrostatic_predictor_discretization;

  const MooseFunctorName _volume_fraction_name;
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
  const Moose::Functor<Real> * _volume_fraction;
  const Moose::Functor<Real> * _vof_rho_phi;
  const Moose::Functor<Real> * _vof_alpha_phi_limited;
  const Moose::Functor<Real> * _liquid_density;
  const Moose::Functor<Real> * _gas_density;
  bool _vof_transport_phi_valid = false;
  bool _corrected_face_phi_seeded = false;
  bool _suppress_explicit_hydrostatic_pressure_flux = false;
  bool _suppress_startup_pressure_predictor_flux_sources = false;
  bool _pressure_coupled_velocity_correction_valid = false;
};
