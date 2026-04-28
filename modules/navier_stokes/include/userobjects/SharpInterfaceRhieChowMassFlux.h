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
  static InputParameters validParams();

  SharpInterfaceRhieChowMassFlux(const InputParameters & params);

  void meshChanged() override;
  void initialSetup() override;
  void initialize() override;
  Real getMassFlux(const FaceInfo & fi) const override;
  Real getVolumetricFaceFlux(const FaceInfo & fi) const override;
  Real getVolumetricFaceFlux(const Moose::FV::InterpMethod m,
                             const FaceInfo & fi,
                             const Moose::StateArg & time,
                             const THREAD_ID tid,
                             bool subtract_mesh_velocity) const override;
  void setUseVOFRhoPhi(const bool use_vof_rho_phi) { _use_vof_rho_phi = use_vof_rho_phi; }
  bool useVOFRhoPhi() const { return _use_vof_rho_phi; }
  bool useFaceBasedPredictorBodyForce() const { return _use_face_based_predictor_body_force; }
  bool seedHydrostaticPressure(LinearSystem & pressure_system,
                               const dof_id_type pressure_pin_dof,
                               const Real pressure_pin_value) const;
  void addMomentumPredictorExplicitForcing(const unsigned int system_i,
                                           NumericVector<Number> & rhs) const override;
  void addMomentumPredictorBodyForceForcing(const unsigned int system_i,
                                            NumericVector<Number> & rhs) const override;

  /// Update the additional pressure-equation source-flux functors before the pressure solve.
  void updateAdditionalPressureFluxFunctors(const bool with_updated_pressure, const bool verbose);

  /// Apply the physical counterpart of the additional source fluxes to the final face mass flux.
  void applyAdditionalFaceMassFluxCorrection();

  /// Apply any matching cell-velocity correction after the base cell velocity update.
  void applyAdditionalCellVelocityCorrection();

  void setSuppressExplicitHydrostaticPressureFlux(
      const bool suppress_explicit_hydrostatic_pressure_flux)
  {
    _suppress_explicit_hydrostatic_pressure_flux = suppress_explicit_hydrostatic_pressure_flux;
  }
  bool suppressExplicitHydrostaticPressureFlux() const
  {
    return _suppress_explicit_hydrostatic_pressure_flux;
  }
  void auditRepresentativeHorizontalFaceReconstruction();
  void auditRepresentativePredictorBodyForce() const;
  void clearPressureCoupledVelocityCorrectionAudit();
  void printPressureCoupledVelocityCorrectionAudit(const std::string & label) const;

protected:
  using FaceVectorField =
      FaceCenteredMapFunctor<RealVectorValue, std::unordered_map<dof_id_type, RealVectorValue>>;

  void initializeAdditionalPressureFluxStorage();
  void rebuildSharpInterfaceFaceInfo();

  Moose::FaceArg makeCenteredFaceArg(const FaceInfo * fi,
                                     const Moose::StateArg * limiter_state = nullptr) const;

  Real interpolateFaceDensity(const FaceInfo * fi, const Moose::StateArg & time_arg) const;

  RealVectorValue interpolateFaceRawAinv(const FaceInfo * fi) const;

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
  void updatePressureCoupledVelocityCorrectionFaceField(const Moose::StateArg & time_arg);
  RealVectorValue reconstructPressureCoupledCellVelocityDelta(
      const ElemInfo * elem_info, const Moose::StateArg & time_arg) const;

  FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>> _transient_projection_flux;
  FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>> _capillary_hydrostatic_flux;
  FaceCenteredMapFunctor<RealVectorValue, std::unordered_map<dof_id_type, RealVectorValue>>
      _pressure_coupled_velocity_correction_face;

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

  const Moose::Functor<RealVectorValue> * const _transient_projection_face_acceleration;
  const Moose::Functor<RealVectorValue> * const _surface_tension_face_acceleration;
  const Moose::Functor<RealVectorValue> * const _surface_tension_cell_acceleration;
  const Moose::Functor<RealVectorValue> * const _hydrostatic_density_gradient_face_acceleration;
  const Moose::Functor<RealVectorValue> * const _hydrostatic_density_gradient_cell_acceleration;
  const Moose::Functor<Real> * _vof_rho_phi;
  bool _use_vof_rho_phi = false;
  bool _suppress_explicit_hydrostatic_pressure_flux = false;
  bool _pressure_coupled_velocity_correction_valid = false;
  bool _pressure_coupled_velocity_correction_audit_valid = false;
  Real _last_pressure_coupled_velocity_delta_l2 = 0.0;
  Real _last_pressure_coupled_velocity_delta_max = 0.0;
  dof_id_type _last_pressure_coupled_velocity_worst_elem_id = 0;
  Point _last_pressure_coupled_velocity_worst_centroid;
};
