//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "RhieChowMassFlux.h"
#include "PetscVectorReader.h"

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
  struct SharpFaceOperatorState
  {
    Real face_rho = 0.0;
    Real negative_sn_grad_p = 0.0;
    Real hydrostatic_mass_flux_density_raw = 0.0;
    Real normal_raw_ainv = 0.0;
    RealVectorValue face_normal;
    RealVectorValue face_raw_ainv;
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
  Real getVolumetricFaceFlux(const Moose::FV::InterpMethod m,
                             const FaceInfo & fi,
                             const Moose::StateArg & time,
                             const THREAD_ID tid,
                             bool subtract_mesh_velocity) const override;
  void commitAcceptedTimestepTransportHistory();
  void freezeVOFTransportState(const bool use_previous_timestep_flux = false);
  void adoptPublishedVOFTransportState();
  void clearVOFTransportState();
  void updateVelocityBoundaryState() override;

  /// Update the additional pressure-equation source-flux functors before the pressure solve.
  void updateAdditionalPressureFluxFunctors(const bool with_updated_pressure, const bool verbose);

  /// Apply the physical counterpart of the additional source fluxes to the final face mass flux.
  void applyAdditionalFaceMassFluxCorrection();
  void computePressureCorrectedCellVelocity();

  void
  setSuppressExplicitHydrostaticPressureFlux(const bool suppress_explicit_hydrostatic_pressure_flux)
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
  RealVectorValue
  reducedPressureMomentumPredictorForceDensity(const ElemInfo & elem_info,
                                               const Moose::StateArg & time_arg) const;
  Real predictorVelocityComponent(const ElemInfo & elem_info, const unsigned int component) const;
  Real vofRhoPhiIntegrated(const FaceInfo & fi) const;

protected:
  using FaceScalarField = FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>>;

  Real pressureBoundaryTargetFlux(const FaceInfo * fi,
                                  const Moose::StateArg & time_arg) const override;
  Real pressureBoundaryNormalAinv(const FaceInfo * fi) const override;
  bool pressureCorrectionFluxIsIntegrated() const override { return true; }
  Real pressureFaceNormalAinv(const FaceInfo * fi, const Moose::StateArg & time_arg) const;

  void initializeAdditionalPressureFluxStorage(const bool preserve_corrected_face_phi = false);
  void writePressureCorrectedVelocityToMomentumSolution(const Moose::StateArg & time_arg);
  void rebuildSharpInterfaceFaceInfo();
  void cacheCurrentCorrectedVolumetricFlux(const Real degenerate_normal_pressure_ainv_tol = 0.0);
  PressureVelocityFaceState
  pressureVelocityFaceState(const FaceInfo * fi,
                            const Moose::StateArg & time_arg,
                            const Real degenerate_normal_pressure_ainv_tol = 0.0) const;
  Real maxPressureFaceNormalAinv(const Moose::StateArg & time_arg) const;
  Real transportMassFluxDensityFromVolumetricPhi(const FaceInfo * fi,
                                                 const Real volumetric_phi,
                                                 const Moose::StateArg & time_arg) const;

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
  RealVectorValue
  interpolateFaceRawAinv(const FaceInfo * fi,
                         const std::vector<PetscVectorReader> & raw_ainv_readers) const;
  void
  buildSharpFaceRawAinvReaders(std::vector<std::unique_ptr<NumericVector<Number>>> & owned_raw_ainv,
                               std::vector<PetscVectorReader> & raw_ainv_readers) const;
  void
  buildSelectedPressureGradientReaders(const bool with_updated_pressure,
                                       std::vector<PetscVectorReader> & pressure_gradient_readers);
  SharpFaceOperatorState buildSharpFaceOperatorState(
      const FaceInfo * fi,
      const Moose::StateArg & time_arg,
      const std::vector<PetscVectorReader> & raw_ainv_readers,
      const std::vector<PetscVectorReader> * pressure_gradient_readers = nullptr) const;

  Real evaluateFaceScalarFunctor(const Moose::Functor<Real> * functor,
                                 const FaceInfo * fi,
                                 const Moose::StateArg & time_arg,
                                 const Moose::StateArg * limiter_state) const;

  Real computeFaceNormalRawAinv(const RealVectorValue & face_ainv_raw,
                                const RealVectorValue & face_normal) const;
  Real computeDefaultTransientProjectionVolumetricFlux(const FaceInfo * fi,
                                                       const Moose::StateArg & time_arg,
                                                       const SharpFaceOperatorState & state);
  Real massFluxDensityToVolumetricNormalFlux(const FaceInfo * fi,
                                             const Real mass_flux_density) const;
  Real computeDiscretePressureFaceVolumetricFlux(const FaceInfo * fi) const;
  Real computeDiscretePressureFaceFlux(const FaceInfo * fi) const override;
  Real computeFaceNormalDensityGradient(const FaceInfo * fi,
                                        const Moose::StateArg & time_arg) const;
  Real computeFaceNormalPressureGradient(const FaceInfo * fi,
                                         const Moose::StateArg & time_arg) const;
  Real computeFaceNormalPressureGradient(
      const FaceInfo * fi, const std::vector<PetscVectorReader> & pressure_gradient_readers) const;
  bool useExplicitHydrostaticPredictorForce() const;
  Real
  computeDiscreteHydrostaticPredictorFaceNormalForceDensity(const FaceInfo * fi,
                                                            const Moose::StateArg & time_arg) const;
  void updatePressureCoupledVelocityCorrectionFaceField(const Moose::StateArg & time_arg);
  RealVectorValue
  reconstructFaceNormalScalarToCellVector(const ElemInfo * elem_info,
                                          const Moose::StateArg & time_arg,
                                          const FaceScalarField & scalar_field) const;
  RealVectorValue
  reconstructBasePressureCoupledCellVelocityDelta(const ElemInfo * elem_info,
                                                  const Moose::StateArg & time_arg) const;
  virtual RealVectorValue
  reconstructPressureCoupledCellVelocityDelta(const ElemInfo * elem_info,
                                              const Moose::StateArg & time_arg) const;
  bool useConstrainedBoundaryPredictorState(const FaceInfo * fi) const;
  FaceScalarField _corrected_face_phi;
  FaceScalarField _previous_timestep_corrected_face_phi;
  FaceScalarField _vof_transport_phi;
  FaceScalarField _pressure_coupled_cell_reconstruction_scalar;

  std::vector<const FaceInfo *> _sharp_interface_face_info;

  const RealVectorValue _gravity;
  const Point _reference_pressure_point;

  const MooseFunctorName _vof_rho_phi_name;

  const Moose::Functor<Real> * _vof_rho_phi;
  bool _vof_transport_phi_valid = false;
  bool _corrected_face_phi_seeded = false;
  bool _suppress_explicit_hydrostatic_pressure_flux = false;
  bool _suppress_startup_pressure_predictor_flux_sources = false;
  bool _pressure_coupled_velocity_correction_valid = false;
};
