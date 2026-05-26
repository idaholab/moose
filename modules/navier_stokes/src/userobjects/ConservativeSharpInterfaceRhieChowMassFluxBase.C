#include "ConservativeSharpInterfaceRhieChowMassFluxBase.h"

#include "LinearFVAdvectionDiffusionBC.h"
#include "LinearSystem.h"
#include "MooseFunctorArguments.h"
#include "MooseMesh.h"
#include "PIMPLE.h"
#include "ReducedPressurePIMPLE.h"
#include "SIMPLE.h"
#include "SubProblem.h"
#include "FVUtils.h"
#include "libmesh/dense_matrix.h"
#include "libmesh/dense_vector.h"
#include "libmesh/petsc_matrix.h"
#include "libmesh/petsc_vector.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <vector>

InputParameters
ConservativeSharpInterfaceRhieChowMassFluxBase::validParams()
{
  InputParameters params = RhieChowMassFlux::validParams();

  params.addClassDescription(
      "Rhie-Chow face-flux provider with additional reduced-pressure predictor functors for "
      "large-density-ratio sharp-interface coupling.");

  params.addParam<bool>(
      "add_transient_projection_flux",
      true,
      "Whether to publish a pressure-equation source flux corresponding to a transient "
      "projection correction.");
  params.addParam<bool>(
      "add_capillary_hydrostatic_flux",
      true,
      "Whether to publish a pressure-equation source flux corresponding to explicit "
      "capillary and hydrostatic-density-gradient corrections.");
  params.addParam<bool>(
      "apply_pressure_velocity_writeback",
      true,
      "Whether the solved pressure correction should be reconstructed back to the cell velocity "
      "state after each reduced-pressure correction stage.");
  params.addParam<bool>(
      "apply_pressure_face_flux_correction",
      true,
      "Whether the solved pressure-equation face flux should be applied to the transport "
      "predictor flux (-phiHbyA) when forming the live corrected face mass flux. Disable this "
      "to run a predictor-only face-flux diagnostic while still assembling the pressure "
      "equation.");
  params.addParam<MooseFunctorName>(
      "volume_fraction_functor",
      "",
      "Optional volume-fraction functor used to identify interface-adjacent cells for sharp "
      "hydrostatic-gradient smoothing.");
  params.addParam<Real>(
      "near_interface_lower",
      0.01,
      "Lower threshold used to classify cells as interface-adjacent for predictor-face "
      "localization.");
  params.addParam<Real>(
      "near_interface_upper",
      0.99,
      "Upper threshold used to classify cells as interface-adjacent for predictor-face "
      "localization.");
  params.addRangeCheckedParam<Real>(
      "pressure_writeback_face_ainv_relative_tolerance",
      1e-4,
      "pressure_writeback_face_ainv_relative_tolerance>=0",
      "Relative cutoff used when normalizing the pressure-correction face flux for the "
      "OpenFOAM-style velocity writeback. Faces whose pressure-space normal Ainv falls below "
      "this fraction of the active-face maximum are treated as degenerate and do not "
      "participate in the reconstructed cell-velocity correction.");
  MooseEnum density_sn_grad_scheme("orthogonal corrected limited", "corrected");
  params.addParam<MooseEnum>(
      "density_sn_grad_scheme",
      density_sn_grad_scheme,
      "Face-normal density-gradient operator used in the explicit hydrostatic reduced-pressure "
      "term. 'orthogonal' uses the pure boundary/center-difference normal gradient, "
      "'corrected' adds the explicit non-orthogonal correction, and 'limited' applies a "
      "limited non-orthogonal correction.");
  params.addRangeCheckedParam<Real>(
      "density_sn_grad_limiter_coefficient",
      0.5,
      "density_sn_grad_limiter_coefficient>=0 & density_sn_grad_limiter_coefficient<=1",
      "Limiter coefficient used when density_sn_grad_scheme = limited.");

  params.addParam<MooseFunctorName>(
      "transient_projection_face_acceleration",
      "",
      "Optional face-vector functor containing the already-discretized transient projection "
      "correction in acceleration-like units. The published pressure-equation source flux will "
      "be built as -(rho_f * (Ainv_raw,f o accel_f) . n_f).");
  params.addParam<MooseFunctorName>(
      "surface_tension_face_acceleration",
      "",
      "Optional face-vector functor containing the surface-tension contribution in the "
      "reduced-pressure momentum balance, expressed in acceleration-like units.");
  params.addParam<MooseFunctorName>(
      "surface_tension_cell_acceleration",
      "",
      "Optional cell-vector functor containing the surface-tension contribution in the "
      "reduced-pressure momentum balance, expressed in acceleration-like units.");
  params.addParam<MooseFunctorName>(
      "hydrostatic_density_gradient_face_acceleration",
      "",
      "Optional face-vector functor containing the reduced-pressure hydrostatic density-gradient "
      "contribution, expressed in acceleration-like units. This is only used when "
      "hydrostatic_predictor_discretization = legacy_acceleration.");
  params.addParam<MooseFunctorName>(
      "hydrostatic_density_gradient_cell_acceleration",
      "",
      "Optional cell-vector functor containing the reduced-pressure hydrostatic density-gradient "
      "contribution, expressed in acceleration-like units. This is only used when "
      "hydrostatic_predictor_discretization = legacy_acceleration.");
  MooseEnum hydrostatic_predictor_discretization("legacy_acceleration discrete_density_sn_grad",
                                                 "legacy_acceleration");
  params.addParam<MooseEnum>(
      "hydrostatic_predictor_discretization",
      hydrostatic_predictor_discretization,
      "How the reduced-pressure hydrostatic predictor term is assembled. "
      "'legacy_acceleration' consumes the optional hydrostatic face/cell acceleration "
      "functors, while 'discrete_density_sn_grad' uses the same face-normal "
      "-ghf*snGrad(rho) operator as the pressure path.");
  params.addParam<MooseFunctorName>(
      "vof_rho_phi_functor",
      "rho_phi",
      "Optional alpha-consistent density flux published by the sharp-interface VOF transport. "
      "When present, downstream advection queries use this face mass flux instead of the raw "
      "Rhie-Chow density interpolation.");
  params.addParam<MooseFunctorName>(
      "vof_alpha_phi_limited_functor",
      "alpha_phi_limited",
      "Optional limited alpha face flux published by the sharp-interface VOF transport. When "
      "provided together with vof_rho_phi_functor and phase densities, the outer-iteration "
      "volumetric phi handoff is reconstructed from the exact VOF rhoPhi relation instead of "
      "being inferred from a weaker density ratio.");
  params.addParam<MooseFunctorName>(
      "liquid_density_functor",
      "rho_l",
      "Liquid-phase density functor used to reconstruct the VOF-consistent volumetric face flux "
      "from rhoPhi and alphaPhi during the outer-iteration handoff.");
  params.addParam<MooseFunctorName>(
      "gas_density_functor",
      "rho_g",
      "Gas-phase density functor used to reconstruct the VOF-consistent volumetric face flux "
      "from rhoPhi and alphaPhi during the outer-iteration handoff.");
  params.addParam<RealVectorValue>(
      "gravity",
      RealVectorValue(),
      "Gravity vector used by the reduced-pressure hydrostatic pressure initializer.");
  params.addParam<Point>(
      "reference_pressure_point",
      Point(0, 0, 0),
      "Reference point used to compute the reduced-pressure head gh = g.(x-x_ref).");
  params.set<MooseEnum>("pressure_projection_method") =
      MooseEnum("standard consistent", "consistent");

  return params;
}

ConservativeSharpInterfaceRhieChowMassFluxBase::ConservativeSharpInterfaceRhieChowMassFluxBase(const InputParameters & params)
  : RhieChowMassFlux(params),
    _transient_projection_flux(_moose_mesh, blockIDs(), "transient_projection_flux"),
    _capillary_hydrostatic_flux(_moose_mesh, blockIDs(), "capillary_hydrostatic_flux"),
    _pressure_equation_volumetric_flux(_moose_mesh, blockIDs(), "pressure_equation_volumetric_flux"),
    _pressure_correction_phi(_moose_mesh, blockIDs(), "pressure_correction_phi"),
    _corrected_face_phi(_moose_mesh, blockIDs(), "corrected_face_phi"),
    _previous_timestep_corrected_face_phi(
        _moose_mesh, blockIDs(), "previous_timestep_corrected_face_phi"),
    _vof_transport_phi(_moose_mesh, blockIDs(), "vof_transport_phi"),
    _debug_update_hydrostatic_face_mass_flux_density_raw(
        _moose_mesh, blockIDs(), "debug_update_hydrostatic_face_mass_flux_density_raw"),
    _debug_update_physical_capillary_hydrostatic_flux(
        _moose_mesh, blockIDs(), "debug_update_physical_capillary_hydrostatic_flux"),
    _debug_update_hydrostatic_branch_taken(
        _moose_mesh, blockIDs(), "debug_update_hydrostatic_branch_taken"),
    _pressure_coupled_cell_reconstruction_scalar(
        _moose_mesh, blockIDs(), "pressure_coupled_cell_reconstruction_scalar"),
    _pressure_coupled_cell_reconstruction_vector(
        _moose_mesh, blockIDs(), "pressure_coupled_cell_reconstruction_vector"),
    _add_transient_projection_flux(getParam<bool>("add_transient_projection_flux")),
    _add_capillary_hydrostatic_flux(getParam<bool>("add_capillary_hydrostatic_flux")),
    _apply_pressure_velocity_writeback(getParam<bool>("apply_pressure_velocity_writeback")),
    _apply_pressure_face_flux_correction(getParam<bool>("apply_pressure_face_flux_correction")),
    _gravity(getParam<RealVectorValue>("gravity")),
    _reference_pressure_point(getParam<Point>("reference_pressure_point")),
    _near_interface_lower(getParam<Real>("near_interface_lower")),
    _near_interface_upper(getParam<Real>("near_interface_upper")),
    _pressure_writeback_face_ainv_relative_tolerance(
        getParam<Real>("pressure_writeback_face_ainv_relative_tolerance")),
    _density_sn_grad_scheme(getParam<MooseEnum>("density_sn_grad_scheme")),
    _density_sn_grad_limiter_coefficient(
        getParam<Real>("density_sn_grad_limiter_coefficient")),
    _hydrostatic_predictor_discretization(
        getParam<MooseEnum>("hydrostatic_predictor_discretization")),
    _volume_fraction_name(getParam<MooseFunctorName>("volume_fraction_functor")),
    _transient_projection_face_acceleration_name(
        getParam<MooseFunctorName>("transient_projection_face_acceleration")),
    _surface_tension_face_acceleration_name(
        getParam<MooseFunctorName>("surface_tension_face_acceleration")),
    _surface_tension_cell_acceleration_name(
        getParam<MooseFunctorName>("surface_tension_cell_acceleration")),
    _hydrostatic_density_gradient_face_acceleration_name(
        getParam<MooseFunctorName>("hydrostatic_density_gradient_face_acceleration")),
    _hydrostatic_density_gradient_cell_acceleration_name(
        getParam<MooseFunctorName>("hydrostatic_density_gradient_cell_acceleration")),
    _vof_rho_phi_name(getParam<MooseFunctorName>("vof_rho_phi_functor")),
    _vof_alpha_phi_limited_name(getParam<MooseFunctorName>("vof_alpha_phi_limited_functor")),
    _liquid_density_name(getParam<MooseFunctorName>("liquid_density_functor")),
    _gas_density_name(getParam<MooseFunctorName>("gas_density_functor")),
    _transient_projection_face_acceleration(
        _transient_projection_face_acceleration_name.empty()
            ? nullptr
            : &getFunctor<RealVectorValue>(_transient_projection_face_acceleration_name)),
    _surface_tension_face_acceleration(
        _surface_tension_face_acceleration_name.empty()
            ? nullptr
            : &getFunctor<RealVectorValue>(_surface_tension_face_acceleration_name)),
    _surface_tension_cell_acceleration(
        _surface_tension_cell_acceleration_name.empty()
            ? nullptr
            : &getFunctor<RealVectorValue>(_surface_tension_cell_acceleration_name)),
    _hydrostatic_density_gradient_face_acceleration(
        _hydrostatic_density_gradient_face_acceleration_name.empty()
            ? nullptr
            : &getFunctor<RealVectorValue>(_hydrostatic_density_gradient_face_acceleration_name)),
    _hydrostatic_density_gradient_cell_acceleration(
        _hydrostatic_density_gradient_cell_acceleration_name.empty()
            ? nullptr
            : &getFunctor<RealVectorValue>(_hydrostatic_density_gradient_cell_acceleration_name)),
    _volume_fraction(nullptr),
    _vof_rho_phi(nullptr),
    _vof_alpha_phi_limited(nullptr),
    _liquid_density(nullptr),
    _gas_density(nullptr)
{
  for (const auto tid : make_range(libMesh::n_threads()))
  {
    UserObject::_subproblem.addFunctor("transient_projection_flux", _transient_projection_flux, tid);
    UserObject::_subproblem.addFunctor(
        "capillary_hydrostatic_flux", _capillary_hydrostatic_flux, tid);
    UserObject::_subproblem.addFunctor(
        "pressure_equation_volumetric_flux", _pressure_equation_volumetric_flux, tid);
    UserObject::_subproblem.addFunctor("pressure_correction_phi", _pressure_correction_phi, tid);
    UserObject::_subproblem.addFunctor("corrected_face_phi", _corrected_face_phi, tid);
    UserObject::_subproblem.addFunctor("pressure_coupled_cell_reconstruction_scalar",
                                       _pressure_coupled_cell_reconstruction_scalar,
                                       tid);
    UserObject::_subproblem.addFunctor("pressure_coupled_cell_reconstruction_vector",
                                       _pressure_coupled_cell_reconstruction_vector,
                                       tid);
  }

  if (!dynamic_cast<SIMPLE *>(getMooseApp().getExecutioner()) &&
      !dynamic_cast<PIMPLE *>(getMooseApp().getExecutioner()))
    mooseError(this->name(),
               " should only be used with a linear segregated thermal-hydraulics solver!");

  if (splitMomentumPredictorOperator())
    mooseError(this->name(),
               ": split_momentum_predictor_operator is no longer supported in the sharp-interface "
               "reduced-pressure path. The implementation now follows the live unsplit "
               "interFoam-style predictor contract.");

  rebuildSharpInterfaceFaceInfo();
  initializeAdditionalPressureFluxStorage();
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::getMassFlux(const FaceInfo & fi) const
{
  return RhieChowMassFlux::getMassFlux(fi);
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::rawRhieChowMassFlux(const FaceInfo & fi) const
{
  return RhieChowMassFlux::getMassFlux(fi);
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::predictorOperatorFaceMassFlux(const FaceInfo & fi,
                                                              const Moose::StateArg & time_arg) const
{
  (void)time_arg;
  return libmesh_map_find(_HbyA_flux, fi.id());
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::pressureCoupledWritebackMassFlux(const FaceInfo & fi) const
{
  return pressureVelocityWritebackFluxDensity(&fi);
}

void
ConservativeSharpInterfaceRhieChowMassFluxBase::commitAcceptedTimestepTransportHistory()
{
  if (!_corrected_face_phi_seeded)
    cacheCurrentCorrectedVolumetricFlux();

  for (const auto * fi : flowFaceInfo())
    _previous_timestep_corrected_face_phi[fi->id()] =
        libmesh_map_find(_corrected_face_phi, fi->id());
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::storedCorrectedFacePhi(const FaceInfo & fi) const
{
  return libmesh_map_find(_corrected_face_phi, fi.id());
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::storedPressurePredictorBasePhi(const FaceInfo & fi) const
{
  // updateAdditionalPressureFluxFunctors() already stores the predictor branch
  // in the same internal volumetric-flux convention used by the pressure
  // corrector. Read that state back directly instead of reconstructing it from
  // rho*HbyA transport fluxes, which is not equivalent on conservative
  // high-density-ratio faces.
  return libmesh_map_find(_pressure_predictor_base_flux, fi.id());
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::storedPressureEquationVolumetricFlux(const FaceInfo & fi) const
{
  return libmesh_map_find(_pressure_equation_volumetric_flux, fi.id());
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::storedPredictorOperatorPhi(const FaceInfo & fi) const
{
  return storedPressurePredictorBasePhi(fi) - libmesh_map_find(_transient_projection_flux, fi.id());
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::storedPressureCorrectionPhi(const FaceInfo & fi) const
{
  return libmesh_map_find(_pressure_correction_phi, fi.id());
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::storedVOFTransportPhi(const FaceInfo & fi) const
{
  return libmesh_map_find(_vof_transport_phi, fi.id());
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::storedOuterIterationPhi(const FaceInfo & fi) const
{
  return libmesh_map_find(_previous_timestep_corrected_face_phi, fi.id());
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::storedOuterIterationRhoPhiIntegrated(const FaceInfo & fi) const
{
  return transportIntegratedRhoPhiFromVolumetricPhi(
      &fi, libmesh_map_find(_previous_timestep_corrected_face_phi, fi.id()), Moose::currentState());
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::storedPredictorConvectivePhi(const FaceInfo & fi) const
{
  return -storedPredictorOperatorPhi(fi);
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::storedPredictorConvectiveMassFlux(const FaceInfo & fi) const
{
  return libmesh_map_find(_HbyA_flux, fi.id());
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::storedPhigFlux(const FaceInfo & fi) const
{
  return libmesh_map_find(_phig_flux, fi.id());
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::storedCapillaryHydrostaticFlux(const FaceInfo & fi) const
{
  return libmesh_map_find(_capillary_hydrostatic_flux, fi.id());
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::storedTransientProjectionFlux(const FaceInfo & fi) const
{
  return libmesh_map_find(_transient_projection_flux, fi.id());
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::debugHydrostaticFaceMassFluxDensityRaw(const FaceInfo & fi) const
{
  return libmesh_map_find(_debug_update_hydrostatic_face_mass_flux_density_raw, fi.id());
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::debugFaceNormalDensityGradient(const FaceInfo & fi,
                                                               const Moose::StateArg & time_arg) const
{
  return computeFaceNormalDensityGradient(&fi, time_arg);
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::debugFaceNormalDensityGradientOrthogonalPart(
    const FaceInfo & fi, const Moose::StateArg & time_arg) const
{
  return computeFaceNormalDensityGradientDebug(&fi, time_arg).orthogonal_part;
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::debugFaceNormalDensityGradientBasePart(
    const FaceInfo & fi, const Moose::StateArg & time_arg) const
{
  return computeFaceNormalDensityGradientDebug(&fi, time_arg).base_part;
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::debugFaceNormalDensityGradientCorrectionPart(
    const FaceInfo & fi, const Moose::StateArg & time_arg) const
{
  return computeFaceNormalDensityGradientDebug(&fi, time_arg).correction_part;
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::debugFaceNormalDensityGradientLimitedCorrectionPart(
    const FaceInfo & fi, const Moose::StateArg & time_arg) const
{
  return computeFaceNormalDensityGradientDebug(&fi, time_arg).limited_correction_part;
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::debugFaceNormalDensityWeightedAinv(const FaceInfo & fi) const
{
  const auto it = _Ainv.find(fi.id());
  if (it == _Ainv.end())
    return 0.0;

  return computeFaceNormalRawAinv(it->second, fi.normal());
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::debugFaceNormalRawAinv(const FaceInfo & fi,
                                                       const Moose::StateArg & time_arg) const
{
  std::vector<std::unique_ptr<NumericVector<Number>>> owned_raw_ainv;
  std::vector<PetscVectorReader> raw_ainv_readers;
  buildSharpFaceRawAinvReaders(owned_raw_ainv, raw_ainv_readers);
  const auto state = buildSharpFaceOperatorState(&fi, time_arg, raw_ainv_readers);
  return state.normal_raw_ainv;
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::debugHydrostaticGh(const FaceInfo & fi) const
{
  return _gravity * (fi.faceCentroid() - _reference_pressure_point);
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::debugElemAlpha(const FaceInfo & fi,
                                               const Moose::StateArg & time_arg) const
{
  if (!_volume_fraction || !fi.elemPtr())
    return 0.0;

  return (*_volume_fraction)(makeElemArg(fi.elemPtr()), time_arg);
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::debugNeighborAlpha(const FaceInfo & fi,
                                                   const Moose::StateArg & time_arg) const
{
  if (!_volume_fraction || !fi.neighborPtr())
    return 0.0;

  return (*_volume_fraction)(makeElemArg(fi.neighborPtr()), time_arg);
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::debugElemDensity(const FaceInfo & fi,
                                                 const Moose::StateArg & time_arg) const
{
  if (!fi.elemPtr())
    return 0.0;

  return _rho(makeElemArg(fi.elemPtr()), time_arg);
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::debugNeighborDensity(const FaceInfo & fi,
                                                     const Moose::StateArg & time_arg) const
{
  if (!fi.neighborPtr())
    return 0.0;

  return _rho(makeElemArg(fi.neighborPtr()), time_arg);
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::vofRhoPhiMassFlux(const FaceInfo & fi) const
{
  const Real face_measure = fi.faceArea() * fi.faceCoord();
  if (face_measure <= libMesh::TOLERANCE)
    return 0.0;

  return vofRhoPhiIntegrated(fi) / face_measure;
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::vofRhoPhiIntegrated(const FaceInfo & fi) const
{
  if (!_vof_rho_phi)
    return 0.0;

  return evaluateFaceScalarFunctor(_vof_rho_phi, &fi, Moose::currentState(), nullptr);
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::vofAlphaPhiLimitedIntegrated(const FaceInfo & fi) const
{
  if (!_vof_alpha_phi_limited)
    return 0.0;

  return evaluateFaceScalarFunctor(_vof_alpha_phi_limited, &fi, Moose::currentState(), nullptr);
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::debugGasDensityFace(const FaceInfo & fi,
                                                    const Moose::StateArg & time_arg) const
{
  if (!_gas_density)
    return 0.0;

  return evaluateCellBasedFaceScalarFunctor(_gas_density, &fi, time_arg);
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::debugLiquidDensityFace(const FaceInfo & fi,
                                                       const Moose::StateArg & time_arg) const
{
  if (!_liquid_density)
    return 0.0;

  return evaluateCellBasedFaceScalarFunctor(_liquid_density, &fi, time_arg);
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::vofBaseGasRhoPhiIntegrated(const FaceInfo & fi) const
{
  const auto time_arg = Moose::currentState();
  const Real face_measure = fi.faceArea() * fi.faceCoord();
  if (face_measure <= libMesh::TOLERANCE)
    return 0.0;

  const Real transport_phi = storedOuterIterationPhi(fi);

  return transport_phi * face_measure * debugGasDensityFace(fi, time_arg);
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::vofAlphaCorrectionRhoPhiIntegrated(const FaceInfo & fi) const
{
  const auto time_arg = Moose::currentState();
  return (debugLiquidDensityFace(fi, time_arg) - debugGasDensityFace(fi, time_arg)) *
         vofAlphaPhiLimitedIntegrated(fi);
}

void
ConservativeSharpInterfaceRhieChowMassFluxBase::dumpPressureCorrectorFaceDebugCSV(const std::string & path)
{
  if (!_pressure_equation_flux_valid)
    cachePressureEquationFlux();

  const auto time_arg = Moose::currentState();
  std::vector<std::unique_ptr<NumericVector<Number>>> owned_raw_ainv;
  std::vector<PetscVectorReader> raw_ainv_readers;
  buildSharpFaceRawAinvReaders(owned_raw_ainv, raw_ainv_readers);
  std::unique_ptr<FaceVectorField> predictor_body_force_face;
  std::unique_ptr<FaceVectorField> predictor_pressure_force_face;
  std::unique_ptr<FaceScalarField> predictor_body_force_scalar;
  std::unique_ptr<FaceScalarField> predictor_pressure_force_scalar;
  predictor_body_force_face =
      std::make_unique<FaceVectorField>(_moose_mesh,
                                        blockIDs(),
                                        "momentum_predictor_body_force_face_debug_csv");
  for (const auto * fi : _sharp_interface_face_info)
    (*predictor_body_force_face)[fi->id()] = RealVectorValue();
  populateMomentumPredictorBodyForceFaceField(*predictor_body_force_face, time_arg);
  predictor_body_force_scalar =
      std::make_unique<FaceScalarField>(_moose_mesh,
                                        blockIDs(),
                                        "momentum_predictor_body_force_scalar_debug_csv");
  for (const auto * fi : _sharp_interface_face_info)
    (*predictor_body_force_scalar)[fi->id()] = 0.0;

  predictor_pressure_force_face =
      std::make_unique<FaceVectorField>(_moose_mesh,
                                        blockIDs(),
                                        "momentum_predictor_pressure_force_face_debug_csv");
  for (const auto * fi : _sharp_interface_face_info)
    (*predictor_pressure_force_face)[fi->id()] = RealVectorValue();
  populateMomentumPredictorPressureForceFaceField(*predictor_pressure_force_face, time_arg);
  predictor_pressure_force_scalar =
      std::make_unique<FaceScalarField>(_moose_mesh,
                                        blockIDs(),
                                        "momentum_predictor_pressure_force_scalar_debug_csv");
  for (const auto * fi : _sharp_interface_face_info)
    (*predictor_pressure_force_scalar)[fi->id()] = 0.0;

  for (const auto * fi : _sharp_interface_face_info)
  {
    (*predictor_body_force_scalar)[fi->id()] =
        (*predictor_body_force_face)(makeCenteredFaceArg(fi), time_arg) * fi->normal();
    (*predictor_pressure_force_scalar)[fi->id()] =
        (*predictor_pressure_force_face)(makeCenteredFaceArg(fi), time_arg) * fi->normal();
  }

  std::ofstream out(path);
  if (!out)
    mooseError("Failed to open sharp-interface pressure debug CSV: ", path);

  out << std::setprecision(17);
  out << "face_id,x,y,z,is_boundary,has_predictor_hydrostatic_face_force,"
         "uses_discrete_hydrostatic_predictor_face_force,has_hydrostatic_face_accel,"
         "suppress_startup_pressure_predictor_flux_sources,"
         "suppress_explicit_hydrostatic_pressure_flux,"
         "normal_x,normal_y,normal_z,"
         "vof_alpha_phi_limited,vof_rho_phi_integrated,"
         "outer_iteration_phi,outer_iteration_rho_phi_integrated,"
         "predictor_convective_phi,predictor_convective_mass_flux,"
         "update_hydrostatic_branch_taken,"
         "pressure_predictor_base_phi,predictor_operator_phi,transient_projection_flux,"
         "capillary_hydrostatic_flux,phig_flux,"
         "pressure_equation_flux,"
         "generic_hbya_mass_flux,generic_hbya_volumetric_phi,generic_hbya_phi_mismatch,"
         "pressure_correction_phi,corrected_face_phi,"
         "interpolated_velocity_face_phi,interpolated_velocity_face_phi_mismatch,"
         "predictor_transport_phi,"
         "phiHbyA_openfoam_style,phiHbyA_current_style,phiHbyA_style_mismatch,"
         "corrected_face_phi_openfoam_style,corrected_face_phi_current_style,"
         "corrected_face_phi_style_mismatch,"
         "raw_rc_mass_flux,predictor_operator_mass_flux,pressure_writeback_mass_flux,"
         "reconstructed_pressure_writeback_mass_flux,pressure_writeback_mass_flux_mismatch,"
         "pressure_coupled_cell_reconstruction_scalar,"
         "elem_delta_u,elem_delta_v,elem_delta_w,"
         "neighbor_delta_u,neighbor_delta_v,neighbor_delta_w,"
         "face_delta_u,face_delta_v,face_delta_w,"
         "predictor_face_density,normal_ainv,normal_raw_ainv,"
         "mass_flux_density_to_volumetric_scale,"
         "update_hydrostatic_face_mass_flux_density_raw,"
         "update_physical_capillary_hydrostatic_flux,"
         "hydrostatic_face_mass_flux_density_raw,"
         "hydrostatic_face_flux_volumetric_raw,"
         "sn_grad_rho,sn_grad_rho_orthogonal_part,sn_grad_rho_base_part,"
         "sn_grad_rho_correction_part,sn_grad_rho_limited_correction_part,"
         "negative_sn_grad_p,"
         "predictor_pressure_force_x,predictor_pressure_force_y,predictor_pressure_force_z,"
         "predictor_pressure_force_scalar,"
         "predictor_body_force_x,predictor_body_force_y,predictor_body_force_z,"
         "predictor_body_force_scalar,"
         "predictor_total_force_x,predictor_total_force_y,predictor_total_force_z\n";

  for (const auto * fi : _sharp_interface_face_info)
  {
    const Point centroid = fi->faceCentroid();
    const RealVectorValue face_normal = fi->normal();
    const bool is_boundary = !_vel[0]->isInternalFace(*fi);
    const Real vof_alpha_phi_limited =
        _vof_alpha_phi_limited ? evaluateFaceScalarFunctor(_vof_alpha_phi_limited, fi, time_arg, nullptr)
                               : 0.0;
    const Real vof_rho_phi_integrated =
        _vof_rho_phi ? evaluateFaceScalarFunctor(_vof_rho_phi, fi, time_arg, nullptr) : 0.0;
    const auto & face_ainv = libmesh_map_find(_Ainv, fi->id());
    const RealVectorValue face_raw_ainv = interpolateFaceRawAinv(fi);

    Real normal_ainv = 0.0;
    for (const auto dim_i : make_range(_dim))
      normal_ainv += face_ainv(dim_i) * face_normal(dim_i) * face_normal(dim_i);
    const Real normal_raw_ainv = computeFaceNormalRawAinv(face_raw_ainv, face_normal);
    const Real mass_flux_density_to_volumetric_scale =
        massFluxDensityToVolumetricNormalFlux(fi, 1.0);
    const auto density_gradient_debug = computeFaceNormalDensityGradientDebug(fi, time_arg);

    const auto state = buildSharpFaceOperatorState(fi, time_arg, raw_ainv_readers);
    const Real negative_sn_grad_p = state.negative_sn_grad_p;

    const RealVectorValue predictor_pressure_force = negative_sn_grad_p * face_normal;
    const Real predictor_pressure_force_scalar_value =
        predictor_pressure_force_scalar
            ? (*predictor_pressure_force_scalar)(makeCenteredFaceArg(fi), time_arg)
            : negative_sn_grad_p;
    const RealVectorValue predictor_body_force =
        predictor_body_force_face
            ? (*predictor_body_force_face)(makeCenteredFaceArg(fi), time_arg)
            : RealVectorValue();
    const Real predictor_body_force_scalar_value =
        predictor_body_force_scalar
            ? (*predictor_body_force_scalar)(makeCenteredFaceArg(fi), time_arg)
            : predictor_body_force * face_normal;
    const RealVectorValue predictor_total_force =
        predictor_pressure_force + predictor_body_force;
    const Real pressure_equation_flux = libmesh_map_find(_pressure_equation_flux, fi->id());
    const Real predictor_operator_phi = storedPredictorOperatorPhi(*fi);
    const Real transient_projection_flux = libmesh_map_find(_transient_projection_flux, fi->id());
    const Real capillary_hydrostatic_flux =
        libmesh_map_find(_capillary_hydrostatic_flux, fi->id());
    const Real phig_flux = libmesh_map_find(_phig_flux, fi->id());
    const Real generic_hbya_mass_flux = libmesh_map_find(_HbyA_flux, fi->id());
    const Real generic_hbya_volumetric_phi =
        predictorFaceDensity(fi, time_arg) > libMesh::TOLERANCE
            ? generic_hbya_mass_flux / predictorFaceDensity(fi, time_arg)
            : 0.0;
    const Real generic_hbya_phi_mismatch = generic_hbya_volumetric_phi + predictor_operator_phi;
    const Real pressure_predictor_base_phi = storedPressurePredictorBasePhi(*fi);
    const Real pressure_correction_phi = pressure_equation_flux - phig_flux;
    const Real corrected_face_phi = libmesh_map_find(_corrected_face_phi, fi->id());
    const Real interpolated_velocity_face_phi = interpolatedPhysicalFaceFlux(fi, time_arg);
    const Real interpolated_velocity_face_phi_mismatch =
        corrected_face_phi - interpolated_velocity_face_phi;
    const Real predictor_transport_phi = -predictor_operator_phi;
    // The stored transient/capillary-hydrostatic branches are kept in the
    // solver's internal sign convention, i.e. as the negative of the physical
    // OpenFOAM-style source contribution. Convert them back here before forming
    // the OpenFOAM-style transported face flux surrogate.
    const Real phi_hbya_openfoam_style =
        predictor_transport_phi - transient_projection_flux - capillary_hydrostatic_flux;
    const Real phi_hbya_current_style = -libmesh_map_find(_phiHbyA_flux, fi->id());
    const Real phi_hbya_style_mismatch = phi_hbya_current_style - phi_hbya_openfoam_style;
    const Real corrected_face_phi_openfoam_style =
        phi_hbya_openfoam_style + pressure_equation_flux;
    const Real corrected_face_phi_current_style = corrected_face_phi;
    const Real corrected_face_phi_style_mismatch =
        corrected_face_phi_current_style - corrected_face_phi_openfoam_style;
    const auto debug_face_state = buildSharpFaceOperatorState(fi, time_arg, raw_ainv_readers);
    const Real hydrostatic_face_mass_flux_density_raw =
        debug_face_state.hydrostatic_mass_flux_density_raw;
    RealVectorValue elem_delta;
    RealVectorValue neighbor_delta;
    RealVectorValue face_delta;
    Real reconstructed_pressure_writeback_mass_flux = 0.0;
    Real pressure_writeback_mass_flux_mismatch = 0.0;
    if (_pressure_coupled_velocity_correction_valid && _vel[0]->isInternalFace(*fi))
    {
      elem_delta = reconstructPressureCoupledCellVelocityDelta(fi->elemInfo(), time_arg);
      neighbor_delta = reconstructPressureCoupledCellVelocityDelta(fi->neighborInfo(), time_arg);
      for (const auto dim_i : make_range(_dim))
        Moose::FV::interpolate(Moose::FV::InterpMethod::Average,
                               face_delta(dim_i),
                               elem_delta(dim_i),
                               neighbor_delta(dim_i),
                               *fi,
                               true);
      reconstructed_pressure_writeback_mass_flux =
          predictorFaceDensity(fi, time_arg) * (face_delta * face_normal);
      pressure_writeback_mass_flux_mismatch =
          pressureCoupledWritebackMassFlux(*fi) - reconstructed_pressure_writeback_mass_flux;
    }

    out << fi->id() << ',' << centroid(0) << ',' << centroid(1) << ',' << centroid(2) << ','
        << (is_boundary ? 1 : 0) << ',' << (hasHydrostaticPredictorFaceForce() ? 1 : 0) << ','
        << (useDiscreteHydrostaticPredictorFaceForce() ? 1 : 0) << ','
        << (_hydrostatic_density_gradient_face_acceleration ? 1 : 0) << ','
        << (_suppress_startup_pressure_predictor_flux_sources ? 1 : 0) << ','
        << (_suppress_explicit_hydrostatic_pressure_flux ? 1 : 0) << ',' << face_normal(0) << ','
        << face_normal(1) << ',' << face_normal(2) << ','
        << vof_alpha_phi_limited << ',' << vof_rho_phi_integrated << ','
        << storedOuterIterationPhi(*fi) << ','
        << storedOuterIterationRhoPhiIntegrated(*fi) << ','
        << storedPredictorConvectivePhi(*fi) << ','
        << storedPredictorConvectiveMassFlux(*fi) << ','
        << libmesh_map_find(_debug_update_hydrostatic_branch_taken, fi->id()) << ','
        << pressure_predictor_base_phi << ','
        << predictor_operator_phi << ','
        << transient_projection_flux << ','
        << capillary_hydrostatic_flux << ','
        << phig_flux << ',' << pressure_equation_flux << ','
        << generic_hbya_mass_flux << ','
        << generic_hbya_volumetric_phi << ','
        << generic_hbya_phi_mismatch << ','
        << pressure_correction_phi << ','
        << corrected_face_phi << ','
        << interpolated_velocity_face_phi << ','
        << interpolated_velocity_face_phi_mismatch << ','
        << predictor_transport_phi << ','
        << phi_hbya_openfoam_style << ','
        << phi_hbya_current_style << ','
        << phi_hbya_style_mismatch << ','
        << corrected_face_phi_openfoam_style << ','
        << corrected_face_phi_current_style << ','
        << corrected_face_phi_style_mismatch << ','
        << rawRhieChowMassFlux(*fi) << ','
        << predictorOperatorFaceMassFlux(*fi, time_arg) << ','
        << pressureCoupledWritebackMassFlux(*fi) << ','
        << reconstructed_pressure_writeback_mass_flux << ','
        << pressure_writeback_mass_flux_mismatch << ','
        << libmesh_map_find(_pressure_coupled_cell_reconstruction_scalar, fi->id()) << ','
        << elem_delta(0) << ',' << elem_delta(1) << ',' << elem_delta(2) << ','
        << neighbor_delta(0) << ',' << neighbor_delta(1) << ',' << neighbor_delta(2) << ','
        << face_delta(0) << ',' << face_delta(1) << ',' << face_delta(2) << ','
        << predictorFaceDensity(fi, time_arg)
        << ',' << normal_ainv << ',' << normal_raw_ainv << ','
        << mass_flux_density_to_volumetric_scale << ','
        << libmesh_map_find(_debug_update_hydrostatic_face_mass_flux_density_raw, fi->id()) << ','
        << libmesh_map_find(_debug_update_physical_capillary_hydrostatic_flux, fi->id()) << ','
        << hydrostatic_face_mass_flux_density_raw << ','
        << mass_flux_density_to_volumetric_scale * hydrostatic_face_mass_flux_density_raw << ','
        << density_gradient_debug.total << ','
        << density_gradient_debug.orthogonal_part << ','
        << density_gradient_debug.base_part << ','
        << density_gradient_debug.correction_part << ','
        << density_gradient_debug.limited_correction_part << ','
        << negative_sn_grad_p << ','
        << predictor_pressure_force(0) << ',' << predictor_pressure_force(1) << ','
        << predictor_pressure_force(2) << ',' << predictor_pressure_force_scalar_value << ','
        << predictor_body_force(0) << ',' << predictor_body_force(1) << ','
        << predictor_body_force(2) << ',' << predictor_body_force_scalar_value << ','
        << predictor_total_force(0) << ',' << predictor_total_force(1) << ','
        << predictor_total_force(2) << '\n';
  }
}

RealVectorValue
ConservativeSharpInterfaceRhieChowMassFluxBase::pressureCoupledCellVelocityDelta(
    const ElemInfo & elem_info, const Moose::StateArg & time_arg) const
{
  if (!_pressure_coupled_velocity_correction_valid)
    const_cast<ConservativeSharpInterfaceRhieChowMassFluxBase *>(this)
        ->updatePressureCoupledVelocityCorrectionFaceField(time_arg);

  return reconstructPressureCoupledCellVelocityDelta(&elem_info, time_arg);
}

ConservativeSharpInterfaceRhieChowMassFluxBase::PressureCorrectionReconstructionDebug
ConservativeSharpInterfaceRhieChowMassFluxBase::pressureCorrectionReconstructionDebug(
    const ElemInfo & elem_info, const Moose::StateArg & time_arg) const
{
  if (!_pressure_coupled_velocity_correction_valid)
    const_cast<ConservativeSharpInterfaceRhieChowMassFluxBase *>(this)
        ->updatePressureCoupledVelocityCorrectionFaceField(time_arg);

  auto debug = reconstructOpenFoamFaceScalarToCellVectorDebug(
      &elem_info, time_arg, _pressure_coupled_cell_reconstruction_scalar);

  const RealVectorValue openfoam_delta =
      reconstructOpenFoamStylePressureCoupledCellVelocityDelta(&elem_info, time_arg);
  for (const auto dim_i : make_range(_dim))
  {
    debug.openfoam_delta_velocity[dim_i] = openfoam_delta(dim_i);
    debug.smooth_delta_velocity[dim_i] = openfoam_delta(dim_i);
    debug.delta_velocity[dim_i] = openfoam_delta(dim_i);
  }

  return debug;
}

ConservativeSharpInterfaceRhieChowMassFluxBase::MomentumPredictorExplicitForceDebug
ConservativeSharpInterfaceRhieChowMassFluxBase::momentumPredictorExplicitForceDebug(
    const ElemInfo & elem_info, const Moose::StateArg & time_arg)
{
  MomentumPredictorExplicitForceDebug debug;

  if (!splitMomentumPredictorOperator() || !hasBlocks(elem_info.subdomain_id()))
    return debug;

  std::unique_ptr<FaceVectorField> predictor_body_force_face;
  bool have_face_based_predictor_body = false;
  predictor_body_force_face =
      std::make_unique<FaceVectorField>(_moose_mesh,
                                        blockIDs(),
                                        "momentum_predictor_body_force_face_debug");
  have_face_based_predictor_body =
      populateMomentumPredictorBodyForceFaceField(*predictor_body_force_face, time_arg);

  const Real cell_volume = elem_info.volume() * elem_info.coordFactor();

  const RealVectorValue pressure_force_density =
      evaluateCellMomentumPredictorPressureForceDensity(elem_info);
  RealVectorValue body_force_density;
  RealVectorValue cell_body_force_density;
  debug.face_based_pressure = false;
  if (_add_capillary_hydrostatic_flux)
    body_force_density =
        have_face_based_predictor_body
            ? reconstructFaceVectorFieldToCellSourceDensity(
                  &elem_info, time_arg, *predictor_body_force_face)
            : evaluateLegacyMomentumPredictorBodyForceDensity(&elem_info, time_arg);

  if (_add_capillary_hydrostatic_flux)
    cell_body_force_density = evaluateLegacyMomentumPredictorBodyForceDensity(&elem_info, time_arg);

  const auto cv_pressure_force_density = RealVectorValue();
  const auto cv_body_force_density = have_face_based_predictor_body
                                         ? reconstructFaceVectorFieldToCellSourceDensity(
                                               &elem_info, time_arg, *predictor_body_force_face)
                                         : RealVectorValue();
  for (const auto dim_i : make_range(_dim))
    debug.scalar_reconstructed_pressure_force_density[dim_i] = cv_pressure_force_density(dim_i);
  for (const auto dim_i : make_range(_dim))
    debug.scalar_reconstructed_body_force_density[dim_i] = cv_body_force_density(dim_i);

  // In the face-authoritative sharp reduced-pressure path, these branches live
  // in phi/BC/corrected-face state and are not injected back into the cell
  // predictor RHS as independent explicit forcing.
  const RealVectorValue applied_force_density;
  for (const auto dim_i : make_range(_dim))
  {
    debug.pressure_force_density[dim_i] = pressure_force_density(dim_i);
    debug.body_force_density[dim_i] = body_force_density(dim_i);
    debug.cell_body_force_density[dim_i] = cell_body_force_density(dim_i);
    debug.total_force_density[dim_i] = applied_force_density(dim_i);
    debug.rhs_contribution[dim_i] = applied_force_density(dim_i) * cell_volume;
  }

  return debug;
}

RealVectorValue
ConservativeSharpInterfaceRhieChowMassFluxBase::reducedPressureMomentumPredictorForceDensity(
    const ElemInfo & elem_info, const Moose::StateArg & time_arg) const
{
  if (!hasBlocks(elem_info.subdomain_id()))
    return RealVectorValue();

  const Elem * const elem = elem_info.elem();
  if (!elem)
    return RealVectorValue();

  DenseMatrix<Real> normal_matrix(_dim, _dim);
  DenseVector<Real> rhs(_dim);
  DenseVector<Real> solution(_dim);

  for (const auto i : make_range(_dim))
  {
    rhs(i) = 0.0;
    solution(i) = 0.0;
    for (const auto j : make_range(_dim))
      normal_matrix(i, j) = 0.0;
  }

  for (const auto side : make_range(elem->n_sides()))
  {
    const Elem * const loc_neighbor = elem->neighbor_ptr(side);
    const bool elem_has_fi = Moose::FV::elemHasFaceInfo(*elem, loc_neighbor);
    if (!elem_has_fi && !loc_neighbor)
      continue;

    const FaceInfo * const fi_loc =
        _moose_mesh.faceInfo(elem_has_fi ? elem : loc_neighbor,
                             elem_has_fi ? side : loc_neighbor->which_neighbor_am_i(elem));
    if (!fi_loc)
      continue;

    const Real face_measure = fi_loc->faceArea() * fi_loc->faceCoord();
    if (face_measure <= 0.0)
      continue;

    const Real orientation = fi_loc->elemPtr() == elem ? 1.0 : -1.0;
    const RealVectorValue outward_normal = orientation * fi_loc->normal();
    const bool is_internal = _vel[0]->isInternalFace(*fi_loc);
    if (!is_internal)
      continue;

    const Real pressure_only_force_density = -computeFaceNormalPressureGradient(fi_loc, time_arg);
    Real normal_force_density = pressure_only_force_density;
    Real hydrostatic_force_density = 0.0;

    if (_add_capillary_hydrostatic_flux)
    {
      const auto elem_arg = makeElemArg(elem);
      const Real cell_rho = _rho(elem_arg, time_arg);

      if (_surface_tension_face_acceleration)
        normal_force_density += cell_rho *
                                (evaluateBoundaryAwareVectorFunctor(
                                     _surface_tension_face_acceleration, fi_loc, time_arg) *
                                 fi_loc->normal());
      else if (_surface_tension_cell_acceleration)
        normal_force_density +=
            cell_rho *
            (MetaPhysicL::raw_value((*_surface_tension_cell_acceleration)(elem_arg, time_arg)) *
             fi_loc->normal());

      const bool use_explicit_hydrostatic = useExplicitHydrostaticPredictorForce();
      if (use_explicit_hydrostatic)
      {
        if (useDiscreteHydrostaticPredictorFaceForce())
          hydrostatic_force_density +=
              computeDiscreteHydrostaticPredictorFaceNormalForceDensity(fi_loc, time_arg);
        else if (_hydrostatic_density_gradient_face_acceleration)
        {
          hydrostatic_force_density +=
              cell_rho *
              (evaluateBoundaryAwareVectorFunctor(
                   _hydrostatic_density_gradient_face_acceleration, fi_loc, time_arg) *
               fi_loc->normal());
        }
        else if (_hydrostatic_density_gradient_cell_acceleration)
        {
          hydrostatic_force_density +=
              cell_rho *
              (MetaPhysicL::raw_value(
                   (*_hydrostatic_density_gradient_cell_acceleration)(elem_arg, time_arg)) *
               fi_loc->normal());
        }
        else
        {
          const Real ghf = _gravity * (fi_loc->faceCentroid() - _reference_pressure_point);
          hydrostatic_force_density += -ghf * computeFaceNormalDensityGradient(fi_loc, time_arg);
        }

        normal_force_density += hydrostatic_force_density;
      }
    }

    const Real psi_f = orientation * normal_force_density;
    for (const auto i : make_range(_dim))
    {
      rhs(i) += face_measure * outward_normal(i) * psi_f;
      for (const auto j : make_range(_dim))
        normal_matrix(i, j) += face_measure * outward_normal(i) * outward_normal(j);
    }
  }

  bool singular = false;
  if (_dim == 1)
    singular = std::abs(normal_matrix(0, 0)) <= libMesh::TOLERANCE;
  else if (_dim == 2)
    singular = std::abs(normal_matrix(0, 0) * normal_matrix(1, 1) -
                        normal_matrix(0, 1) * normal_matrix(1, 0)) <= libMesh::TOLERANCE;
  else if (_dim == 3)
  {
    const Real det =
        normal_matrix(0, 0) * (normal_matrix(1, 1) * normal_matrix(2, 2) -
                               normal_matrix(1, 2) * normal_matrix(2, 1)) -
        normal_matrix(0, 1) * (normal_matrix(1, 0) * normal_matrix(2, 2) -
                               normal_matrix(1, 2) * normal_matrix(2, 0)) +
        normal_matrix(0, 2) * (normal_matrix(1, 0) * normal_matrix(2, 1) -
                               normal_matrix(1, 1) * normal_matrix(2, 0));
    singular = std::abs(det) <= libMesh::TOLERANCE;
  }

  if (singular)
    return RealVectorValue();

  normal_matrix.lu_solve(rhs, solution);

  RealVectorValue reconstructed_source;
  for (const auto i : make_range(_dim))
    reconstructed_source(i) = solution(i);

  return reconstructed_source;
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::predictorVelocityComponent(const ElemInfo & elem_info,
                                                           const unsigned int component) const
{
  mooseAssert(component < _dim, "Momentum component index out of range.");

  const auto dof = elem_info.dofIndices()[_global_momentum_system_numbers[component]][0];
  return -(*_HbyA_raw[component])(dof);
}

bool
ConservativeSharpInterfaceRhieChowMassFluxBase::seedHydrostaticPressure(LinearSystem & pressure_system,
                                                        const dof_id_type pressure_pin_dof,
                                                        const Real pressure_pin_value) const
{
  static constexpr Real gravity_tol = 1e-12;
  static constexpr Real coordinate_tol = 1e-10;

  unsigned int dominant_component = libMesh::invalid_uint;
  Real dominant_magnitude = 0.0;
  for (const auto component : make_range(_dim))
  {
    const Real magnitude = std::abs(_gravity(component));
    if (magnitude > dominant_magnitude)
    {
      dominant_component = component;
      dominant_magnitude = magnitude;
    }
  }

  if (dominant_component == libMesh::invalid_uint || dominant_magnitude <= gravity_tol)
    return false;

  for (const auto component : make_range(_dim))
    if (component != dominant_component && std::abs(_gravity(component)) > gravity_tol)
      return false;

  struct LevelData
  {
    Real coordinate = 0.0;
    Real gh = 0.0;
    Real rho_sum = 0.0;
    unsigned int count = 0;
    std::vector<dof_id_type> dofs;
  };

  std::map<long long, LevelData> levels;
  long long pin_key = 0;
  bool found_pin = false;

  const auto time_arg = Moose::currentState();

  for (const auto & elem_info : _fe_problem.mesh().elemInfoVector())
  {
    if (!hasBlocks(elem_info->subdomain_id()))
      continue;

    const auto * const elem = elem_info->elem();
    const auto elem_arg = makeElemArg(elem);
    const Point centroid = elem->vertex_average();
    const Real coordinate = centroid(dominant_component);
    const long long key = std::llround(coordinate / coordinate_tol);
    auto & level = levels[key];
    if (level.count == 0)
    {
      level.coordinate = coordinate;
      level.gh = _gravity * (centroid - _reference_pressure_point);
    }

    level.rho_sum += _rho(elem_arg, time_arg);
    level.count++;

    const auto dof = elem_info->dofIndices()[_global_pressure_system_number][0];
    level.dofs.push_back(dof);

    if (dof == pressure_pin_dof)
    {
      pin_key = key;
      found_pin = true;
    }
  }

  if (!found_pin || levels.empty())
    return false;

  struct ReducedPressureLevel
  {
    Real gh = 0.0;
    Real rho = 0.0;
    Real reduced_pressure = 0.0;
    std::vector<dof_id_type> dofs;
  };

  std::vector<std::pair<long long, ReducedPressureLevel>> ordered_levels;
  ordered_levels.reserve(levels.size());
  for (const auto & [key, level] : levels)
    ordered_levels.push_back(
        {key, {level.gh, level.rho_sum / level.count, 0.0, level.dofs}});

  std::sort(ordered_levels.begin(),
            ordered_levels.end(),
            [](const auto & lhs, const auto & rhs) { return lhs.second.gh < rhs.second.gh; });

  const auto pin_it = std::find_if(
      ordered_levels.begin(),
      ordered_levels.end(),
      [pin_key](const auto & level) { return level.first == pin_key; });
  if (pin_it == ordered_levels.end())
    return false;

  const auto pin_index = std::distance(ordered_levels.begin(), pin_it);
  pin_it->second.reduced_pressure = pressure_pin_value;

  Real total_pressure = pressure_pin_value + pin_it->second.rho * pin_it->second.gh;
  for (auto level_index = pin_index + 1; level_index < ordered_levels.size(); ++level_index)
  {
    const auto & previous = ordered_levels[level_index - 1].second;
    auto & current = ordered_levels[level_index].second;
    total_pressure += 0.5 * (previous.rho + current.rho) * (current.gh - previous.gh);
    current.reduced_pressure = total_pressure - current.rho * current.gh;
  }

  total_pressure = pressure_pin_value + pin_it->second.rho * pin_it->second.gh;
  for (int level_index = static_cast<int>(pin_index) - 1; level_index >= 0; --level_index)
  {
    const auto & next = ordered_levels[level_index + 1].second;
    auto & current = ordered_levels[level_index].second;
    total_pressure += 0.5 * (next.rho + current.rho) * (current.gh - next.gh);
    current.reduced_pressure = total_pressure - current.rho * current.gh;
  }

  auto & current_local_solution = *(pressure_system.system().current_local_solution);
  for (const auto & level : ordered_levels)
    for (const auto dof : level.second.dofs)
      current_local_solution.set(dof, level.second.reduced_pressure);

  current_local_solution.close();
  pressure_system.setSolution(current_local_solution);

  return true;
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::getVolumetricFaceFlux(const FaceInfo & fi) const
{
  if (_corrected_face_phi_seeded)
    return libmesh_map_find(_corrected_face_phi, fi.id());

  return libmesh_map_find(_corrected_face_phi, fi.id());
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::getVOFTransportVolumetricFaceFlux(const FaceInfo & fi) const
{
  if (_vof_transport_phi_valid)
    return libmesh_map_find(_vof_transport_phi, fi.id());

  if (_corrected_face_phi_seeded)
    return libmesh_map_find(_corrected_face_phi, fi.id());

  return RhieChowMassFlux::getVolumetricFaceFlux(fi);
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::getVolumetricFaceFlux(const Moose::FV::InterpMethod m,
                                                      const FaceInfo & fi,
                                                      const Moose::StateArg & time,
                                                      const THREAD_ID /*tid*/,
                                                      bool subtract_mesh_velocity) const
{
  mooseAssert(!subtract_mesh_velocity,
              "ConservativeSharpInterfaceRhieChowMassFluxBase does not support moving meshes yet!");

  if (m != Moose::FV::InterpMethod::RhieChow)
    mooseError("Interpolation methods other than Rhie-Chow are not supported!");
  if (time.state != Moose::currentState().state)
    mooseError("Older interpolation times are not supported!");

  return getVolumetricFaceFlux(fi);
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::maxVolumeFractionCourant(const Real dt) const
{
  return RhieChowMassFlux::maxCourant(dt);
}

void
ConservativeSharpInterfaceRhieChowMassFluxBase::initializeAdditionalPressureFluxStorage(
    const bool preserve_corrected_face_phi)
{
  for (const auto * fi : _fe_problem.mesh().faceInfo())
  {
    _transient_projection_flux[fi->id()] = 0.0;
    _capillary_hydrostatic_flux[fi->id()] = 0.0;
    _pressure_equation_volumetric_flux[fi->id()] = 0.0;
    _pressure_correction_phi[fi->id()] = 0.0;
    if (!preserve_corrected_face_phi)
      _corrected_face_phi[fi->id()] = 0.0;
    _previous_timestep_corrected_face_phi[fi->id()] = 0.0;
    _vof_transport_phi[fi->id()] = 0.0;
    _debug_update_hydrostatic_face_mass_flux_density_raw[fi->id()] = 0.0;
    _debug_update_physical_capillary_hydrostatic_flux[fi->id()] = 0.0;
    _debug_update_hydrostatic_branch_taken[fi->id()] = 0.0;
    _pressure_coupled_cell_reconstruction_scalar[fi->id()] = 0.0;
    _pressure_coupled_cell_reconstruction_vector[fi->id()] = RealVectorValue();
    _pressure_predictor_flux[fi->id()] = 0.0;
    _pressure_predictor_mass_flux[fi->id()] = 0.0;
  }

  _vof_transport_phi_valid = false;
  _corrected_face_phi_seeded = preserve_corrected_face_phi && _corrected_face_phi_seeded;
  _pressure_coupled_velocity_correction_valid = false;
}

void
ConservativeSharpInterfaceRhieChowMassFluxBase::writeProvisionalVelocityToMomentumSolution(
    const Moose::StateArg & time_arg)
{
  std::vector<std::unique_ptr<NumericVector<Number>>> provisional_solution;
  provisional_solution.reserve(_momentum_implicit_systems.size());

  for (const auto system_i : index_range(_momentum_implicit_systems))
  {
    auto * momentum_system = _momentum_implicit_systems[system_i];
    mooseAssert(momentum_system && momentum_system->current_local_solution,
                "The requested momentum component is not linked to ConservativeSharpInterfaceRhieChowMassFluxBase.");
    provisional_solution.push_back(momentum_system->current_local_solution->zero_clone());
  }

  for (const auto & elem_info : _fe_problem.mesh().elemInfoVector())
  {
    if (!hasBlocks(elem_info->subdomain_id()))
      continue;

    const RealVectorValue pressure_delta =
        reconstructPressureCoupledCellVelocityDelta(elem_info, time_arg);

    for (const auto system_i : index_range(_momentum_implicit_systems))
    {
      const auto & dof_indices =
          elem_info->dofIndices()[_global_momentum_system_numbers[system_i]];
      if (dof_indices.empty())
        continue;

      provisional_solution[system_i]->set(
          dof_indices[0], predictorVelocityComponent(*elem_info, system_i) + pressure_delta(system_i));
    }
  }

  for (const auto system_i : index_range(_momentum_implicit_systems))
  {
    provisional_solution[system_i]->close();
    *(_momentum_implicit_systems[system_i]->solution) = *provisional_solution[system_i];
    _momentum_implicit_systems[system_i]->update();
    _momentum_systems[system_i]->setSolution(
        *_momentum_implicit_systems[system_i]->current_local_solution);
  }
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::transportMassFluxDensityFromVolumetricPhi(
    const FaceInfo * fi, const Real volumetric_phi, const Moose::StateArg & time_arg) const
{
  const Real face_rho = interpolateFaceDensity(fi, time_arg);
  return face_rho * volumetric_phi;
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::transportIntegratedRhoPhiFromVolumetricPhi(
    const FaceInfo * fi, const Real volumetric_phi, const Moose::StateArg & time_arg) const
{
  if (!fi)
    return 0.0;

  const Real face_measure = fi->faceArea() * fi->faceCoord();
  return transportMassFluxDensityFromVolumetricPhi(fi, volumetric_phi, time_arg) * face_measure;
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::pressureBoundaryTargetFlux(const FaceInfo * fi,
                                                           const Moose::StateArg & time_arg) const
{
  return boundaryPhysicalVolumetricFluxTarget(fi, time_arg);
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::pressureBoundaryNormalAinv(const FaceInfo * fi) const
{
  if (!fi)
    return 0.0;

  const auto it = _pressure_Ainv.find(fi->id());
  if (it == _pressure_Ainv.end())
    return RhieChowMassFlux::pressureBoundaryNormalAinv(fi);

  return computeFaceNormalRawAinv(it->second, fi->normal());
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::cellPhysicalVelocityComponent(
    const ElemInfo & elem_info, const unsigned int component, const Moose::StateArg & time_arg) const
{
  mooseAssert(component < _vel.size(), "Velocity component index out of range.");
  return _vel[component]->getElemValue(elem_info, time_arg);
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::boundaryPhysicalVelocityComponent(
    const FaceInfo * fi, const unsigned int component, const Moose::StateArg & time_arg) const
{
  return boundaryVelocityValue(fi, component, time_arg);
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::boundaryPhysicalVolumetricFluxTarget(
    const FaceInfo * fi, const Moose::StateArg & time_arg) const
{
  mooseAssert(fi && !_vel[0]->isInternalFace(*fi),
              "boundaryPhysicalVolumetricFluxTarget should only be called on boundary faces.");

  const bool elem_is_fluid = hasBlocks(fi->elemPtr()->subdomain_id());
  const Real boundary_normal_multiplier = elem_is_fluid ? 1.0 : -1.0;

  RealVectorValue face_velocity;
  for (const auto component : index_range(_vel))
    face_velocity(component) =
        boundary_normal_multiplier * boundaryPhysicalVelocityComponent(fi, component, time_arg);

  return face_velocity * fi->normal();
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::pressureFaceScalarDiffusionCoefficient(
    const FaceInfo * fi, const Moose::StateArg & time_arg) const
{
  if (!fi)
    return 0.0;

  if (!_vel[0]->isInternalFace(*fi))
    return pressureBoundaryNormalAinv(fi);

  // The pressure writeback must divide by the same face coefficient consumed
  // by the pressure diffusion kernel. Read that coefficient back through the
  // pressure-space face functor rather than projecting the cached raw Ainv map
  // directly, so the writeback stays aligned with the operator contract if the
  // pressure face interpolation changes.
  const RealVectorValue pressure_face_diffusion_tensor =
      _pressure_Ainv(makeCenteredFaceArg(fi), time_arg);
  return computeFaceNormalRawAinv(pressure_face_diffusion_tensor, fi->normal());
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::transportVolumetricPhiFromMassFluxDensity(
    const FaceInfo * fi, const Real mass_flux_density) const
{
  if (!fi)
    return 0.0;

  const Real face_rho = interpolateFaceDensity(fi, Moose::currentState());
  return std::abs(face_rho) > libMesh::TOLERANCE ? mass_flux_density / face_rho : 0.0;
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::transportVolumetricPhiFromIntegratedRhoPhi(
    const FaceInfo * fi, const Real integrated_rho_phi) const
{
  if (!fi)
    return 0.0;

  const Real face_measure = fi->faceArea() * fi->faceCoord();
  if (face_measure <= libMesh::TOLERANCE)
    return 0.0;

  return transportVolumetricPhiFromMassFluxDensity(fi, integrated_rho_phi / face_measure);
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::publishedVOFRhoPhiIntegrated(const FaceInfo * fi,
                                                             const Moose::StateArg & time_arg) const
{
  if (!fi || !_vof_rho_phi)
    return 0.0;

  return evaluateFaceScalarFunctor(_vof_rho_phi, fi, time_arg, nullptr);
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::publishedVOFTransportVolumetricFaceFlux(
    const FaceInfo * fi, const Moose::StateArg & time_arg) const
{
  if (!fi)
    return 0.0;

  const Real integrated_rho_phi = publishedVOFRhoPhiIntegrated(fi, time_arg);
  if (!_vof_alpha_phi_limited || !_liquid_density || !_gas_density)
    return transportVolumetricPhiFromIntegratedRhoPhi(fi, integrated_rho_phi);

  const Real face_measure = fi->faceArea() * fi->faceCoord();
  if (face_measure <= libMesh::TOLERANCE)
    return 0.0;

  const Real gas_density = debugGasDensityFace(*fi, time_arg);
  const Real liquid_density = debugLiquidDensityFace(*fi, time_arg);
  const Real limited_alpha_flux =
      evaluateFaceScalarFunctor(_vof_alpha_phi_limited, fi, time_arg, nullptr);
  const Real gas_mass_flux =
      integrated_rho_phi - (liquid_density - gas_density) * limited_alpha_flux;

  if (std::abs(gas_density) <= libMesh::TOLERANCE)
    return transportVolumetricPhiFromIntegratedRhoPhi(fi, integrated_rho_phi);

  return gas_mass_flux / (gas_density * face_measure);
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::faceAlphaRho(const FaceInfo * fi,
                                             const Moose::StateArg & time_arg) const
{
  if (!fi)
    return 0.0;

  if (_vel[0]->isInternalFace(*fi))
  {
    if (const auto * owner_info = sharpInterfaceOneSidedInterpolationOwner(fi, time_arg))
    {
      const Real owner_rho = _rho(makeElemArg(owner_info->elem()), time_arg);
      const Real owner_alpha = (*_volume_fraction)(makeElemArg(owner_info->elem()), time_arg);
      return owner_alpha * owner_rho;
    }

    const Real elem_rho = _rho(makeElemArg(fi->elemPtr()), time_arg);
    const Real neighbor_rho = _rho(makeElemArg(fi->neighborPtr()), time_arg);
    const Real elem_alpha =
        _volume_fraction ? (*_volume_fraction)(makeElemArg(fi->elemPtr()), time_arg) : 1.0;
    const Real neighbor_alpha =
        _volume_fraction ? (*_volume_fraction)(makeElemArg(fi->neighborPtr()), time_arg) : 1.0;
    return 0.5 * (elem_alpha * elem_rho + neighbor_alpha * neighbor_rho);
  }

  const bool elem_is_fluid = hasBlocks(fi->elemPtr()->subdomain_id());
  const Elem * const fluid_elem = elem_is_fluid ? fi->elemPtr() : fi->neighborPtr();
  const Moose::FaceArg boundary_face{
      fi, Moose::FV::LimiterType::CentralDifference, true, false, fluid_elem, nullptr};
  const Real face_rho = _rho(boundary_face, time_arg);
  const Real face_alpha = _volume_fraction ? (*_volume_fraction)(boundary_face, time_arg) : 1.0;
  return face_alpha * face_rho;
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::interpolatedAlphaRhoVelocityFlux(const FaceInfo * fi,
                                                                 const Moose::StateArg & time_arg) const
{
  if (!fi)
    return 0.0;

  RealVectorValue face_alpha_rho_velocity;

  if (_vel[0]->isInternalFace(*fi))
  {
    if (const auto * owner_info = sharpInterfaceOneSidedInterpolationOwner(fi, time_arg))
    {
      const Real owner_rho = _rho(makeElemArg(owner_info->elem()), time_arg);
      const Real owner_alpha = (*_volume_fraction)(makeElemArg(owner_info->elem()), time_arg);
      for (const auto dim_i : make_range(_dim))
      {
        const Real owner_velocity =
            std::abs(owner_rho) > libMesh::TOLERANCE
                ? _vel[dim_i]->getElemValue(*owner_info, time_arg) / owner_rho
                : 0.0;
        face_alpha_rho_velocity(dim_i) = owner_alpha * owner_rho * owner_velocity;
      }
      return face_alpha_rho_velocity * fi->normal();
    }

    const auto & elem_info = *fi->elemInfo();
    const auto & neighbor_info = *fi->neighborInfo();
    const Real elem_rho = _rho(makeElemArg(fi->elemPtr()), time_arg);
    const Real neighbor_rho = _rho(makeElemArg(fi->neighborPtr()), time_arg);
    const Real elem_alpha =
        _volume_fraction ? (*_volume_fraction)(makeElemArg(fi->elemPtr()), time_arg) : 1.0;
    const Real neighbor_alpha =
        _volume_fraction ? (*_volume_fraction)(makeElemArg(fi->neighborPtr()), time_arg) : 1.0;

    for (const auto dim_i : make_range(_dim))
    {
      // Replace the generic face-velocity accessor with cell-owned alpha*rho*U
      // interpolation so the transient projection matches OpenFOAM's
      // dotInterpolate(Sf, alpha*rho*U.old()) primitive.
      const Real elem_velocity =
          std::abs(elem_rho) > libMesh::TOLERANCE
              ? _vel[dim_i]->getElemValue(elem_info, time_arg) / elem_rho
              : 0.0;
      const Real neighbor_velocity =
          std::abs(neighbor_rho) > libMesh::TOLERANCE
              ? _vel[dim_i]->getElemValue(neighbor_info, time_arg) / neighbor_rho
              : 0.0;
      face_alpha_rho_velocity(dim_i) =
          0.5 * (elem_alpha * elem_rho * elem_velocity +
                 neighbor_alpha * neighbor_rho * neighbor_velocity);
    }
  }
  else
  {
    const Real face_alpha_rho = faceAlphaRho(fi, time_arg);
    for (const auto dim_i : make_range(_dim))
      face_alpha_rho_velocity(dim_i) =
          face_alpha_rho * facePhysicalVelocityComponent(fi, dim_i, time_arg);
  }

  return face_alpha_rho_velocity * fi->normal();
}

void
ConservativeSharpInterfaceRhieChowMassFluxBase::cacheCurrentCorrectedVolumetricFlux()
{
  // The pressure equation consumes _phiHbyA_flux in the native
  // pressure-correction sign convention, matching the stock RC contract:
  //   face_flux = -phiHbyA + pressure_equation_flux
  // The accepted transport volumetric flux must therefore negate the cached
  // predictor branch before adding the solved pressure-equation face flux.
  const auto time_arg = Moose::currentState();
  for (const auto * fi : flowFaceInfo())
  {
    if (_pressure_predictor_face_state_valid)
    {
      const Real predictor_transport_phi = -libmesh_map_find(_phiHbyA_flux, fi->id());
      _pressure_equation_volumetric_flux[fi->id()] =
          _pressure_equation_flux_valid ? libmesh_map_find(_pressure_equation_flux, fi->id()) : 0.0;
      _pressure_correction_phi[fi->id()] =
          _pressure_equation_flux_valid
              ? _pressure_equation_volumetric_flux[fi->id()] - libmesh_map_find(_phig_flux, fi->id())
              : 0.0;
      _corrected_face_phi[fi->id()] =
          predictor_transport_phi +
          (_apply_pressure_face_flux_correction ? _pressure_equation_volumetric_flux[fi->id()]
                                                : 0.0);
    }
    else
    {
      _pressure_equation_volumetric_flux[fi->id()] = 0.0;
      _pressure_correction_phi[fi->id()] = 0.0;
      const Real face_rho = predictorFaceDensity(fi, time_arg);
      _corrected_face_phi[fi->id()] =
          std::abs(face_rho) > libMesh::TOLERANCE
              ? libmesh_map_find(_face_mass_flux, fi->id()) / face_rho
              : 0.0;
    }
  }
  _corrected_face_phi_seeded = true;
}

void
ConservativeSharpInterfaceRhieChowMassFluxBase::freezeVOFTransportState(const bool use_previous_timestep_flux)
{
  if (!_corrected_face_phi_seeded)
    cacheCurrentCorrectedVolumetricFlux();

  for (const auto * fi : flowFaceInfo())
    _vof_transport_phi[fi->id()] =
        use_previous_timestep_flux ? libmesh_map_find(_previous_timestep_corrected_face_phi, fi->id())
                                   : libmesh_map_find(_corrected_face_phi, fi->id());

  _vof_transport_phi_valid = true;
}

void
ConservativeSharpInterfaceRhieChowMassFluxBase::adoptPublishedVOFTransportState()
{
  // Match interFoam ownership more closely: alpha subcycling updates alphaPhi
  // and rhoPhi, but does not replace the active transport phi with a
  // reconstructed volumetric flux recovered from rhoPhi. The face flux used to
  // advect alpha remains the authoritative transport phi for the rest of the
  // outer correction.
  if (!_vof_transport_phi_valid)
    freezeVOFTransportState(false);
}

void
ConservativeSharpInterfaceRhieChowMassFluxBase::clearVOFTransportState()
{
  for (auto & pair : _vof_transport_phi)
    pair.second = 0.0;

  _vof_transport_phi_valid = false;
}

void
ConservativeSharpInterfaceRhieChowMassFluxBase::rebuildSharpInterfaceFaceInfo()
{
  _sharp_interface_face_info.clear();
  for (auto & fi : _fe_problem.mesh().faceInfo())
    if (hasBlocks(fi->elemPtr()->subdomain_id()) ||
        (fi->neighborPtr() && hasBlocks(fi->neighborPtr()->subdomain_id())))
      _sharp_interface_face_info.push_back(fi);

  initializeAdditionalPressureFluxStorage();
}

void
ConservativeSharpInterfaceRhieChowMassFluxBase::meshChanged()
{
  RhieChowMassFlux::meshChanged();
  rebuildSharpInterfaceFaceInfo();
}

void
ConservativeSharpInterfaceRhieChowMassFluxBase::initialSetup()
{
  const_cast<MooseLinearVariableFVReal *>(_p)->computeCellGradients();
  RhieChowMassFlux::initialSetup();
  rebuildSharpInterfaceFaceInfo();
  if (!_vof_rho_phi_name.empty() &&
      UserObject::_subproblem.hasFunctorWithType<Real>(_vof_rho_phi_name, _tid))
    _vof_rho_phi = &getFunctor<Real>(_vof_rho_phi_name);
  if (!_volume_fraction_name.empty() &&
      UserObject::_subproblem.hasFunctorWithType<Real>(_volume_fraction_name, _tid))
    _volume_fraction = &getFunctor<Real>(_volume_fraction_name);
  if (!_vof_alpha_phi_limited_name.empty() &&
      UserObject::_subproblem.hasFunctorWithType<Real>(_vof_alpha_phi_limited_name, _tid))
    _vof_alpha_phi_limited = &getFunctor<Real>(_vof_alpha_phi_limited_name);
  if (!_liquid_density_name.empty() &&
      UserObject::_subproblem.hasFunctorWithType<Real>(_liquid_density_name, _tid))
    _liquid_density = &getFunctor<Real>(_liquid_density_name);
  if (!_gas_density_name.empty() &&
      UserObject::_subproblem.hasFunctorWithType<Real>(_gas_density_name, _tid))
    _gas_density = &getFunctor<Real>(_gas_density_name);
}

void
ConservativeSharpInterfaceRhieChowMassFluxBase::initialize()
{
  RhieChowMassFlux::initialize();

  initializeAdditionalPressureFluxStorage(/*preserve_corrected_face_phi=*/true);
  if (!_corrected_face_phi_seeded)
    cacheCurrentCorrectedVolumetricFlux();

  for (const auto * fi : _sharp_interface_face_info)
    _previous_timestep_corrected_face_phi[fi->id()] = libmesh_map_find(_corrected_face_phi, fi->id());
}

void
ConservativeSharpInterfaceRhieChowMassFluxBase::initFaceMassFlux()
{
  RhieChowMassFlux::initFaceMassFlux();
}

void
ConservativeSharpInterfaceRhieChowMassFluxBase::computeFaceMassFlux()
{
  if (!_pressure_predictor_face_state_valid)
    updatePressurePredictorFaceState();

  if (!_pressure_equation_flux_valid)
    cachePressureEquationFlux();

  cacheCurrentCorrectedVolumetricFlux();

  const auto time_arg = Moose::currentState();
  for (const auto * fi : _sharp_interface_face_info)
    _face_mass_flux[fi->id()] =
        transportMassFluxDensityFromVolumetricPhi(
            fi, libmesh_map_find(_corrected_face_phi, fi->id()), time_arg);
}

Moose::FaceArg
ConservativeSharpInterfaceRhieChowMassFluxBase::makeCenteredFaceArg(const FaceInfo * fi,
                                                    const Moose::StateArg * limiter_state) const
{
  return Moose::FaceArg{
      fi, Moose::FV::LimiterType::CentralDifference, true, false, nullptr, limiter_state};
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::interpolateFaceDensity(const FaceInfo * fi,
                                                       const Moose::StateArg & time_arg) const
{
  using namespace Moose::FV;

  if (_vel[0]->isInternalFace(*fi))
  {
    if (const auto * owner_info = sharpInterfaceOneSidedInterpolationOwner(fi, time_arg))
      return _rho(makeElemArg(owner_info->elem()), time_arg);

    const Real elem_rho = _rho(makeElemArg(fi->elemPtr()), time_arg);
    const Real neighbor_rho = _rho(makeElemArg(fi->neighborPtr()), time_arg);

    Real face_rho = 0.0;
    interpolate(InterpMethod::Average, face_rho, elem_rho, neighbor_rho, *fi, true);
    return face_rho;
  }

  const bool elem_is_fluid = hasBlocks(fi->elemPtr()->subdomain_id());
  const ElemInfo & elem_info = elem_is_fluid ? *fi->elemInfo() : *fi->neighborInfo();

  if (useConstrainedBoundaryPredictorState(fi))
  {
    const Moose::FaceArg boundary_face{
        fi, Moose::FV::LimiterType::CentralDifference, true, false, elem_info.elem(), nullptr};
    return _rho(boundary_face, time_arg);
  }

  return _rho(makeElemArg(elem_info.elem()), time_arg);
}

const ElemInfo *
ConservativeSharpInterfaceRhieChowMassFluxBase::sharpInterfaceOneSidedInterpolationOwner(
    const FaceInfo * fi, const Moose::StateArg & time_arg) const
{
  if (!fi || !_vel[0]->isInternalFace(*fi) || !_volume_fraction || !fi->elemInfo() || !fi->neighborInfo())
    return nullptr;

  const Real elem_alpha = (*_volume_fraction)(makeElemArg(fi->elemPtr()), time_arg);
  const Real neighbor_alpha = (*_volume_fraction)(makeElemArg(fi->neighborPtr()), time_arg);
  const bool elem_liquid = elem_alpha >= _near_interface_upper;
  const bool elem_gas = elem_alpha <= _near_interface_lower;
  const bool neighbor_liquid = neighbor_alpha >= _near_interface_upper;
  const bool neighbor_gas = neighbor_alpha <= _near_interface_lower;

  if (elem_liquid && neighbor_gas)
    return fi->elemInfo();

  if (neighbor_liquid && elem_gas)
    return fi->neighborInfo();

  return nullptr;
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::predictorFaceDensity(const FaceInfo * fi,
                                                     const Moose::StateArg & time_arg) const
{
  // In the sharp-interface VOF path, rhoPhi is assembled from the primary volumetric
  // flux plus the limited alpha transport flux. It is therefore not generally equal
  // to rho_f * phi, and recovering a predictor "face density" from rhoPhi / phi
  // injects a non-physical variable-density mismatch into the pressure predictor and
  // velocity writeback chain.
  return interpolateFaceDensity(fi, time_arg);
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::facePhysicalVelocityComponent(
    const FaceInfo * fi, const unsigned int component, const Moose::StateArg & time_arg) const
{
  mooseAssert(fi, "FaceInfo should not be null when evaluating a face velocity component.");
  mooseAssert(component < _vel.size(), "Velocity component index out of range.");

  if (_vel[component]->isInternalFace(*fi))
  {
    if (const auto * owner_info = sharpInterfaceOneSidedInterpolationOwner(fi, time_arg))
      return cellPhysicalVelocityComponent(*owner_info, component, time_arg);

    const auto & elem_info = *fi->elemInfo();
    const auto & neighbor_info = *fi->neighborInfo();
    return 0.5 * (cellPhysicalVelocityComponent(elem_info, component, time_arg) +
                  cellPhysicalVelocityComponent(neighbor_info, component, time_arg));
  }

  return boundaryPhysicalVelocityComponent(fi, component, time_arg);
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::interpolatedPhysicalFaceFlux(const FaceInfo * fi,
                                                             const Moose::StateArg & time_arg) const
{
  if (!fi)
    return 0.0;

  RealVectorValue face_velocity;
  for (const auto dim_i : make_range(_dim))
    face_velocity(dim_i) = facePhysicalVelocityComponent(fi, dim_i, time_arg);

  return face_velocity * fi->normal();
}

RealVectorValue
ConservativeSharpInterfaceRhieChowMassFluxBase::interpolateFaceRawAinv(const FaceInfo * fi) const
{
  std::vector<std::unique_ptr<NumericVector<Number>>> owned_raw_ainv;
  std::vector<PetscVectorReader> raw_ainv_readers;
  buildSharpFaceRawAinvReaders(owned_raw_ainv, raw_ainv_readers);
  return interpolateFaceRawAinv(fi, raw_ainv_readers);
}

RealVectorValue
ConservativeSharpInterfaceRhieChowMassFluxBase::interpolateFaceRawAinv(
    const FaceInfo * fi, const std::vector<PetscVectorReader> & raw_ainv_readers) const
{
  using namespace Moose::FV;

  RealVectorValue face_ainv;

  if (raw_ainv_readers.size() < _dim)
    return face_ainv;

  if (_vel[0]->isInternalFace(*fi))
  {
    if (const auto * owner_info = sharpInterfaceOneSidedInterpolationOwner(fi, Moose::currentState()))
      for (const auto dim_i : make_range(_dim))
      {
        const auto dof = owner_info->dofIndices()[_global_momentum_system_numbers[dim_i]][0];
        face_ainv(dim_i) = raw_ainv_readers[dim_i](dof);
      }
    else
    {
      const auto & elem_info = *fi->elemInfo();
      const auto & neighbor_info = *fi->neighborInfo();

      for (const auto dim_i : make_range(_dim))
      {
      const auto elem_dof = elem_info.dofIndices()[_global_momentum_system_numbers[dim_i]][0];
      const auto neighbor_dof =
          neighbor_info.dofIndices()[_global_momentum_system_numbers[dim_i]][0];
      interpolate(InterpMethod::Average,
                  face_ainv(dim_i),
                  raw_ainv_readers[dim_i](elem_dof),
                  raw_ainv_readers[dim_i](neighbor_dof),
                  *fi,
                  true);
      }
    }
  }
  else
  {
    const bool elem_is_fluid = hasBlocks(fi->elemPtr()->subdomain_id());
    const ElemInfo & elem_info = elem_is_fluid ? *fi->elemInfo() : *fi->neighborInfo();

    for (const auto dim_i : make_range(_dim))
    {
      const auto elem_dof = elem_info.dofIndices()[_global_momentum_system_numbers[dim_i]][0];
      face_ainv(dim_i) = raw_ainv_readers[dim_i](elem_dof);
    }
  }

  return face_ainv;
}

RealVectorValue
ConservativeSharpInterfaceRhieChowMassFluxBase::interpolatePressureFaceRawAinv(
    const FaceInfo * fi, const std::vector<PetscVectorReader> & raw_ainv_readers) const
{
  using namespace Moose::FV;

  RealVectorValue face_ainv;

  if (!fi || raw_ainv_readers.size() < _dim)
    return face_ainv;

  if (_vel[0]->isInternalFace(*fi))
  {
    const auto & elem_info = *fi->elemInfo();
    const auto & neighbor_info = *fi->neighborInfo();

    for (const auto dim_i : make_range(_dim))
    {
      const auto elem_dof = elem_info.dofIndices()[_global_momentum_system_numbers[dim_i]][0];
      const auto neighbor_dof =
          neighbor_info.dofIndices()[_global_momentum_system_numbers[dim_i]][0];
      interpolate(InterpMethod::Average,
                  face_ainv(dim_i),
                  raw_ainv_readers[dim_i](elem_dof),
                  raw_ainv_readers[dim_i](neighbor_dof),
                  *fi,
                  true);
    }
  }
  else
  {
    const bool elem_is_fluid = hasBlocks(fi->elemPtr()->subdomain_id());
    const ElemInfo & elem_info = elem_is_fluid ? *fi->elemInfo() : *fi->neighborInfo();

    for (const auto dim_i : make_range(_dim))
    {
      const auto elem_dof = elem_info.dofIndices()[_global_momentum_system_numbers[dim_i]][0];
      face_ainv(dim_i) = raw_ainv_readers[dim_i](elem_dof);
    }
  }

  return face_ainv;
}

void
ConservativeSharpInterfaceRhieChowMassFluxBase::buildSharpFaceRawAinvReaders(
    std::vector<std::unique_ptr<NumericVector<Number>>> & owned_raw_ainv,
    std::vector<PetscVectorReader> & raw_ainv_readers) const
{
  owned_raw_ainv.clear();
  raw_ainv_readers.clear();

  const bool have_stored_raw_ainv =
      _Ainv_raw.size() >= _dim &&
      std::all_of(_Ainv_raw.begin(),
                  _Ainv_raw.begin() + _dim,
                  [](const auto & vec) { return static_cast<bool>(vec); });

  if (have_stored_raw_ainv)
  {
    raw_ainv_readers.reserve(_dim);
    for (const auto dim_i : make_range(_dim))
      raw_ainv_readers.emplace_back(*_Ainv_raw[dim_i]);
    return;
  }

  const bool have_cached_predictor_diagonal =
      _cached_predictor_diagonal_raw.size() >= _dim &&
      std::all_of(_cached_predictor_diagonal_raw.begin(),
                  _cached_predictor_diagonal_raw.begin() + _dim,
                  [](const auto & vec) { return static_cast<bool>(vec); });

  if (!have_cached_predictor_diagonal)
    return;

  owned_raw_ainv.reserve(_dim);
  raw_ainv_readers.reserve(_dim);

  for (const auto dim_i : make_range(_dim))
  {
    auto * momentum_system = _momentum_implicit_systems[dim_i];
    mooseAssert(momentum_system,
                "Momentum system must be linked before building startup sharp face operators.");

    NumericVector<Number> & current_local_solution = *(momentum_system->current_local_solution);
    owned_raw_ainv.push_back(current_local_solution.zero_clone());
    NumericVector<Number> & raw_ainv = *owned_raw_ainv.back();
    raw_ainv = *_cached_predictor_diagonal_raw[dim_i];

    auto working_vector = current_local_solution.zero_clone();
    auto * working_vector_petsc = dynamic_cast<PetscVector<Number> *>(working_vector.get());
    mooseAssert(working_vector_petsc,
                "The sharp startup face operator requires PETSc-backed momentum vectors.");
    *working_vector_petsc = 1.0;

    raw_ainv.pointwise_divide(*working_vector_petsc, raw_ainv);
    raw_ainv.pointwise_mult(raw_ainv, *_cell_volumes);
    raw_ainv.close();
    raw_ainv_readers.emplace_back(raw_ainv);
  }
}

void
ConservativeSharpInterfaceRhieChowMassFluxBase::buildSelectedPressureGradientReaders(
    const bool with_updated_pressure, std::vector<PetscVectorReader> & pressure_gradient_readers)
{
  pressure_gradient_readers.clear();
  auto & pressure_gradient = selectPressureGradient(with_updated_pressure);
  pressure_gradient_readers.reserve(pressure_gradient.size());
  for (const auto & component : pressure_gradient)
    pressure_gradient_readers.emplace_back(*component);
}

ConservativeSharpInterfaceRhieChowMassFluxBase::SharpFaceOperatorState
ConservativeSharpInterfaceRhieChowMassFluxBase::buildSharpFaceOperatorState(
    const FaceInfo * fi,
    const Moose::StateArg & time_arg,
    const std::vector<PetscVectorReader> & raw_ainv_readers,
    const std::vector<PetscVectorReader> * pressure_gradient_readers) const
{
  SharpFaceOperatorState state;
  state.face_normal = fi->normal();
  state.face_rho = interpolateFaceDensity(fi, time_arg);
  state.face_raw_ainv = interpolateFaceRawAinv(fi, raw_ainv_readers);
  state.normal_raw_ainv = computeFaceNormalRawAinv(state.face_raw_ainv, state.face_normal);
  state.negative_sn_grad_p =
      -(pressure_gradient_readers
            ? computeFaceNormalPressureGradient(fi, *pressure_gradient_readers)
            : computeFaceNormalPressureGradient(fi, time_arg));
  if (!_suppress_startup_pressure_predictor_flux_sources &&
      !_suppress_explicit_hydrostatic_pressure_flux)
  {
    const Real ghf = _gravity * (fi->faceCentroid() - _reference_pressure_point);
    const Real sn_grad_rho = computeFaceNormalDensityGradient(fi, time_arg);
    state.hydrostatic_mass_flux_density_raw = -ghf * sn_grad_rho * state.normal_raw_ainv;
  }

  return state;
}

RealVectorValue
ConservativeSharpInterfaceRhieChowMassFluxBase::interpolateFaceRau(const FaceInfo * fi) const
{
  using namespace Moose::FV;

  RealVectorValue face_rau;
  std::vector<PetscVectorReader> ainv_reader;
  ainv_reader.reserve(_Ainv_raw.size());
  for (const auto dim_i : index_range(_Ainv_raw))
    ainv_reader.emplace_back(*_Ainv_raw[dim_i]);

  auto cell_rau = [this, &ainv_reader](const ElemInfo & elem_info, const unsigned int dim_i)
  {
    const auto dof = elem_info.dofIndices()[_global_momentum_system_numbers[dim_i]][0];
    const Real cell_volume = elem_info.volume() * elem_info.coordFactor();
    return cell_volume > libMesh::TOLERANCE ? ainv_reader[dim_i](dof) / cell_volume : 0.0;
  };

  if (_vel[0]->isInternalFace(*fi))
  {
    if (const auto * owner_info = sharpInterfaceOneSidedInterpolationOwner(fi, Moose::currentState()))
      for (const auto dim_i : make_range(_dim))
        face_rau(dim_i) = cell_rau(*owner_info, dim_i);
    else
    {
      const auto & elem_info = *fi->elemInfo();
      const auto & neighbor_info = *fi->neighborInfo();

      for (const auto dim_i : make_range(_dim))
      interpolate(InterpMethod::Average,
                  face_rau(dim_i),
                  cell_rau(elem_info, dim_i),
                  cell_rau(neighbor_info, dim_i),
                  *fi,
                  true);
    }
  }
  else
  {
    const bool elem_is_fluid = hasBlocks(fi->elemPtr()->subdomain_id());
    const ElemInfo & elem_info = elem_is_fluid ? *fi->elemInfo() : *fi->neighborInfo();

    for (const auto dim_i : make_range(_dim))
      face_rau(dim_i) = cell_rau(elem_info, dim_i);
  }

  return face_rau;
}

RealVectorValue
ConservativeSharpInterfaceRhieChowMassFluxBase::evaluateFaceVectorFunctor(
    const Moose::Functor<RealVectorValue> * functor,
    const FaceInfo * fi,
    const Moose::StateArg & time_arg,
    const Moose::StateArg * limiter_state) const
{
  if (!functor)
    return RealVectorValue();

  return MetaPhysicL::raw_value((*functor)(makeCenteredFaceArg(fi, limiter_state), time_arg));
}

RealVectorValue
ConservativeSharpInterfaceRhieChowMassFluxBase::evaluateBoundaryAwareVectorFunctor(
    const Moose::Functor<RealVectorValue> * functor,
    const FaceInfo * fi,
    const Moose::StateArg & time_arg) const
{
  if (!functor)
    return RealVectorValue();

  return MetaPhysicL::raw_value((*functor)(makeCenteredFaceArg(fi), time_arg));
}

RealVectorValue
ConservativeSharpInterfaceRhieChowMassFluxBase::interpolateCellVectorFunctorToFace(
    const Moose::Functor<RealVectorValue> * functor,
    const FaceInfo * fi,
    const Moose::StateArg & time_arg) const
{
  if (!functor)
    return RealVectorValue();

  if (_vel[0]->isInternalFace(*fi))
  {
    if (const auto * owner_info = sharpInterfaceOneSidedInterpolationOwner(fi, time_arg))
      return MetaPhysicL::raw_value((*functor)(makeElemArg(owner_info->elem()), time_arg));

    const auto elem_value = MetaPhysicL::raw_value((*functor)(makeElemArg(fi->elemPtr()), time_arg));
    const auto neighbor_value =
        MetaPhysicL::raw_value((*functor)(makeElemArg(fi->neighborPtr()), time_arg));
    return 0.5 * (elem_value + neighbor_value);
  }

  const bool elem_is_fluid = hasBlocks(fi->elemPtr()->subdomain_id());
  const Elem * const fluid_elem = elem_is_fluid ? fi->elemPtr() : fi->neighborPtr();
  return MetaPhysicL::raw_value((*functor)(makeElemArg(fluid_elem), time_arg));
}

RealVectorValue
ConservativeSharpInterfaceRhieChowMassFluxBase::interpolateCellBodyForceDensityToFace(
    const Moose::Functor<RealVectorValue> * acceleration_functor,
    const FaceInfo * fi,
    const Moose::StateArg & time_arg) const
{
  if (!acceleration_functor)
    return RealVectorValue();

  const auto evaluate_cell_body_force = [this, &time_arg, acceleration_functor](const Elem * elem)
  {
    const auto elem_arg = makeElemArg(elem);
    const Real rho = _rho(elem_arg, time_arg);
    return rho * MetaPhysicL::raw_value((*acceleration_functor)(elem_arg, time_arg));
  };

  if (_vel[0]->isInternalFace(*fi))
  {
    if (const auto * owner_info = sharpInterfaceOneSidedInterpolationOwner(fi, time_arg))
      return evaluate_cell_body_force(owner_info->elem());

    return 0.5 * (evaluate_cell_body_force(fi->elemPtr()) +
                  evaluate_cell_body_force(fi->neighborPtr()));
  }

  const bool elem_is_fluid = hasBlocks(fi->elemPtr()->subdomain_id());
  const Elem * const fluid_elem = elem_is_fluid ? fi->elemPtr() : fi->neighborPtr();
  return evaluate_cell_body_force(fluid_elem);
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::evaluateFaceScalarFunctor(const Moose::Functor<Real> * functor,
                                                          const FaceInfo * fi,
                                                          const Moose::StateArg & time_arg,
                                                          const Moose::StateArg * limiter_state) const
{
  if (!functor)
    return 0.0;

  return MetaPhysicL::raw_value((*functor)(makeCenteredFaceArg(fi, limiter_state), time_arg));
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::evaluateBoundaryAwareScalarFunctor(
    const Moose::Functor<Real> * functor,
    const FaceInfo * fi,
    const Moose::StateArg & time_arg) const
{
  if (!functor)
    return 0.0;

  return MetaPhysicL::raw_value((*functor)(makeCenteredFaceArg(fi), time_arg));
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::evaluateCellBasedFaceScalarFunctor(
    const Moose::Functor<Real> * functor,
    const FaceInfo * fi,
    const Moose::StateArg & time_arg) const
{
  if (!functor || !fi)
    return 0.0;

  if (_vel[0]->isInternalFace(*fi))
  {
    const Real elem_value = MetaPhysicL::raw_value((*functor)(makeElemArg(fi->elemPtr()), time_arg));
    const Real neighbor_value =
        MetaPhysicL::raw_value((*functor)(makeElemArg(fi->neighborPtr()), time_arg));
    return 0.5 * (elem_value + neighbor_value);
  }

  const bool elem_is_fluid = hasBlocks(fi->elemPtr()->subdomain_id());
  const Elem * const fluid_elem = elem_is_fluid ? fi->elemPtr() : fi->neighborPtr();
  return MetaPhysicL::raw_value((*functor)(makeElemArg(fluid_elem), time_arg));
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::projectPhysicalMassFluxDensity(const Real face_rho,
                                                               const RealVectorValue & face_ainv_raw,
                                                               const RealVectorValue & face_acceleration,
                                                               const RealVectorValue & face_normal) const
{
  Real projected_flux = 0.0;
  for (const auto dim_i : make_range(_dim))
    projected_flux += face_rho * face_ainv_raw(dim_i) * face_acceleration(dim_i) * face_normal(dim_i);

  return projected_flux;
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::computeFaceNormalRawAinv(
    const RealVectorValue & face_ainv_raw, const RealVectorValue & face_normal) const
{
  Real normal_ainv = 0.0;
  for (const auto dim_i : make_range(_dim))
    normal_ainv += face_ainv_raw(dim_i) * face_normal(dim_i) * face_normal(dim_i);

  return normal_ainv;
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::computeDefaultTransientProjectionVolumetricFlux(
    const FaceInfo * fi,
    const Moose::StateArg & time_arg,
    const SharpFaceOperatorState & state) const
{
  if (!fi || time_arg.state != Moose::currentState().state)
    return 0.0;

  const Real dt = _fe_problem.dt();
  if (dt <= libMesh::TOLERANCE)
    return 0.0;

  if (!_vel[0]->isInternalFace(*fi) && useConstrainedBoundaryPredictorState(fi))
    return 0.0;

  const Real old_corrected_phi = libmesh_map_find(_previous_timestep_corrected_face_phi, fi->id());
  const Real old_interpolated_face_phi = interpolatedPhysicalFaceFlux(fi, Moose::oldState());
  const Real phi_corr = old_corrected_phi - old_interpolated_face_phi;
  const Real phi_scale = std::abs(old_corrected_phi) + std::numeric_limits<Real>::epsilon();
  const Real ddt_coupling_coeff =
      std::max(0.0, 1.0 - std::min(std::abs(phi_corr) / phi_scale, 1.0));

  return ddt_coupling_coeff * state.face_rho * state.normal_raw_ainv * phi_corr / dt;
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::massFluxDensityToVolumetricNormalFlux(
    const FaceInfo * fi, const Real mass_flux_density) const
{
  if (!fi)
    return 0.0;

  const RealVectorValue face_normal = fi->normal();
  const RealVectorValue face_raw_ainv = interpolateFaceRawAinv(fi);
  const RealVectorValue face_density_weighted_ainv = libmesh_map_find(_Ainv, fi->id());
  const Real normal_raw_ainv = computeFaceNormalRawAinv(face_raw_ainv, face_normal);
  const Real normal_density_weighted_ainv =
      computeFaceNormalRawAinv(face_density_weighted_ainv, face_normal);

  if (std::abs(normal_density_weighted_ainv) <= libMesh::TOLERANCE)
  {
    const Real face_rho = predictorFaceDensity(fi, Moose::currentState());
    return std::abs(face_rho) > libMesh::TOLERANCE ? mass_flux_density / face_rho : 0.0;
  }

  return mass_flux_density * normal_raw_ainv / normal_density_weighted_ainv;
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::volumetricNormalFluxToPressureMassFluxDensity(
    const FaceInfo * fi, const Real volumetric_flux) const
{
  if (!fi)
    return 0.0;

  const RealVectorValue face_normal = fi->normal();
  const RealVectorValue face_raw_ainv = interpolateFaceRawAinv(fi);
  const RealVectorValue face_density_weighted_ainv = libmesh_map_find(_Ainv, fi->id());
  const Real normal_raw_ainv = computeFaceNormalRawAinv(face_raw_ainv, face_normal);
  const Real normal_density_weighted_ainv =
      computeFaceNormalRawAinv(face_density_weighted_ainv, face_normal);

  if (std::abs(normal_raw_ainv) <= libMesh::TOLERANCE)
    return transportMassFluxDensityFromVolumetricPhi(fi, volumetric_flux, Moose::currentState());

  return volumetric_flux * normal_density_weighted_ainv / normal_raw_ainv;
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::computeDiscretePressureFaceVolumetricFlux(const FaceInfo * fi) const
{
  if (!fi)
    return 0.0;

  PetscVectorReader p_reader(*_pressure_system->system().current_local_solution);

  _p_diffusion_kernel->setupFaceData(fi);
  _p_diffusion_kernel->setCurrentFaceArea(1.0);

  if (_p->isInternalFace(*fi))
  {
    const auto & elem_info = *fi->elemInfo();
    const auto & neighbor_info = *fi->neighborInfo();
    const auto elem_dof = elem_info.dofIndices()[_global_pressure_system_number][0];
    const auto neighbor_dof = neighbor_info.dofIndices()[_global_pressure_system_number][0];
    const auto p_elem_value = p_reader(elem_dof);
    const auto p_neighbor_value = p_reader(neighbor_dof);
    const auto elem_matrix_contribution = _p_diffusion_kernel->computeElemMatrixContribution();
    const auto neighbor_matrix_contribution =
        _p_diffusion_kernel->computeNeighborMatrixContribution();
    const auto elem_rhs_contribution =
        _p_diffusion_kernel->computeElemRightHandSideContribution();

    // The pressure equation now uses the raw face Ainv functor (`pressure_Ainv`),
    // so the diffusion kernel already returns the solved volumetric face flux
    // primitive directly. Do not reinterpret it as a density-weighted pressure
    // mass-flux density here.
    return p_neighbor_value * neighbor_matrix_contribution +
           p_elem_value * elem_matrix_contribution - elem_rhs_contribution;
  }

  if (!fi->boundaryIDs().empty())
  {
    mooseAssert(fi->boundaryIDs().size() == 1, "We should only have one boundary on every face.");
    if (auto * bc_pointer = _p->getBoundaryCondition(*fi->boundaryIDs().begin()))
    {
      bc_pointer->setupFaceData(
          fi, fi->faceType(std::make_pair(_p->number(), _global_pressure_system_number)));

      const ElemInfo & elem_info =
          hasBlocks(fi->elemPtr()->subdomain_id()) ? *fi->elemInfo() : *fi->neighborInfo();
      const auto elem_dof = elem_info.dofIndices()[_global_pressure_system_number][0];
      const auto p_elem_value = p_reader(elem_dof);
      const auto matrix_contribution =
          _p_diffusion_kernel->computeBoundaryMatrixContribution(*bc_pointer);
      const auto rhs_contribution =
          _p_diffusion_kernel->computeBoundaryRHSContribution(*bc_pointer);

      // Boundary pressure diffusion is in the same volumetric face-flux space
      // as the interior operator once `pressure_Ainv` is used.
      return p_elem_value * matrix_contribution - rhs_contribution;
    }
  }

  return 0.0;
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::computeDiscretePressureFaceFlux(const FaceInfo * fi) const
{
  return computeDiscretePressureFaceVolumetricFlux(fi);
}

ConservativeSharpInterfaceRhieChowMassFluxBase::DensityNormalGradientDebug
ConservativeSharpInterfaceRhieChowMassFluxBase::computeFaceNormalDensityGradientDebug(
    const FaceInfo * fi, const Moose::StateArg & time_arg) const
{
  DensityNormalGradientDebug debug;

  if (!fi)
    return debug;

  if (_vel[0]->isInternalFace(*fi))
  {
    const Real elem_rho = _rho(makeElemArg(fi->elemPtr()), time_arg);
    const Real neighbor_rho = _rho(makeElemArg(fi->neighborPtr()), time_arg);
    const Point delta = fi->dCN();
    const Real delta_mag = delta.norm();

    if (delta_mag <= libMesh::TOLERANCE)
      return debug;

    const Real rho_jump = neighbor_rho - elem_rho;
    debug.orthogonal_part = rho_jump / delta_mag;

    const Real non_orth_delta_coeff =
        1.0 / std::max(fi->normal() * delta, 0.05 * delta_mag);
    debug.base_part = non_orth_delta_coeff * rho_jump;

    const Point non_orth_correction_vector = fi->normal() - delta * non_orth_delta_coeff;
    const RealVectorValue elem_grad_rho =
        MetaPhysicL::raw_value(_rho.gradient(makeElemArg(fi->elemPtr()), time_arg));
    const RealVectorValue neighbor_grad_rho =
        MetaPhysicL::raw_value(_rho.gradient(makeElemArg(fi->neighborPtr()), time_arg));

    RealVectorValue face_grad_rho = 0;
    Moose::FV::interpolate(
        Moose::FV::InterpMethod::Average, face_grad_rho, elem_grad_rho, neighbor_grad_rho, *fi, true);
    debug.correction_part = non_orth_correction_vector * face_grad_rho;

    if (_density_sn_grad_scheme == "orthogonal")
    {
      debug.total = debug.orthogonal_part;
      return debug;
    }

    if (_density_sn_grad_scheme == "corrected")
    {
      debug.limited_correction_part = debug.correction_part;
      debug.total = debug.base_part + debug.limited_correction_part;
      return debug;
    }

    const Real correction_magnitude = std::abs(debug.correction_part);
    const Real base_magnitude = std::abs(debug.base_part);
    const Real limiter = std::min(
        _density_sn_grad_limiter_coefficient * base_magnitude /
            ((1.0 - _density_sn_grad_limiter_coefficient) * correction_magnitude +
             libMesh::TOLERANCE),
        1.0);
    debug.limited_correction_part = limiter * debug.correction_part;
    debug.total = debug.base_part + debug.limited_correction_part;
    return debug;
  }

  const bool elem_is_fluid = hasBlocks(fi->elemPtr()->subdomain_id());
  const Elem * const fluid_elem = elem_is_fluid ? fi->elemPtr() : fi->neighborPtr();
  const Moose::FaceArg boundary_face{
      fi, Moose::FV::LimiterType::CentralDifference, true, false, fluid_elem, nullptr};

  // On uncoupled boundary patches OpenFOAM's explicit non-orthogonal correction
  // is zero and the operative snGrad comes from the boundary patch field.
  debug.orthogonal_part =
      MetaPhysicL::raw_value(_rho.gradient(boundary_face, time_arg)) * fi->normal();
  debug.base_part = debug.orthogonal_part;
  debug.total = debug.orthogonal_part;
  return debug;
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::computeFaceNormalDensityGradient(
    const FaceInfo * fi, const Moose::StateArg & time_arg) const
{
  return computeFaceNormalDensityGradientDebug(fi, time_arg).total;
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::computeFaceNormalPressureGradient(
    const FaceInfo * fi, const Moose::StateArg & time_arg) const
{
  if (!fi)
    return 0.0;

  if (_vel[0]->isInternalFace(*fi))
  {
    const Real elem_p = _p->getElemValue(*fi->elemInfo(), time_arg);
    const Real neighbor_p = _p->getElemValue(*fi->neighborInfo(), time_arg);
    const Real normal_spacing = std::abs(fi->dCN() * fi->normal());

    return normal_spacing > libMesh::TOLERANCE ? (neighbor_p - elem_p) / normal_spacing : 0.0;
  }

  if (_pressure_boundary_normal_gradient_valid)
    return libmesh_map_find(_pressure_boundary_normal_gradient, fi->id());

  const bool elem_is_fluid = hasBlocks(fi->elemPtr()->subdomain_id());
  const Point fluid_centroid = elem_is_fluid ? fi->elemCentroid() : fi->neighborCentroid();
  const Point cell_to_face = fi->faceCentroid() - fluid_centroid;
  const Real normal_spacing = std::abs(cell_to_face * fi->normal());

  if (normal_spacing <= libMesh::TOLERANCE)
    return 0.0;

  const Real elem_p =
      _p->getElemValue(elem_is_fluid ? *fi->elemInfo() : *fi->neighborInfo(), time_arg);

  if (!fi->boundaryIDs().empty())
  {
    mooseAssert(fi->boundaryIDs().size() == 1,
                "Expected a single boundary id on a FV pressure boundary face.");

    if (auto * bc_pointer = _p->getBoundaryCondition(*fi->boundaryIDs().begin()))
    {
      bc_pointer->setupFaceData(
          fi, fi->faceType(std::make_pair(_p->number(), _global_pressure_system_number)));

      const Real bc_sn_grad = bc_pointer->computeBoundaryNormalGradient();
      if (std::isfinite(bc_sn_grad))
        return bc_sn_grad;

      const Real bc_value = bc_pointer->computeBoundaryValue();
      if (std::isfinite(bc_value))
        return (bc_value - elem_p) / normal_spacing;
    }
  }

  return 0.0;
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::computeFaceNormalPressureGradient(
    const FaceInfo * fi, const std::vector<PetscVectorReader> & pressure_gradient_readers) const
{
  if (!fi)
    return 0.0;

  auto cell_grad_dot_normal = [this, &pressure_gradient_readers, fi](const ElemInfo & elem_info)
  {
    RealVectorValue grad_p;
    for (const auto dim_i : make_range(_dim))
    {
      const auto dof = elem_info.dofIndices()[_global_pressure_system_number][0];
      grad_p(dim_i) = pressure_gradient_readers[dim_i](dof);
    }
    return grad_p * fi->normal();
  };

  if (_vel[0]->isInternalFace(*fi))
  {
    const Real elem_grad = cell_grad_dot_normal(*fi->elemInfo());
    const Real neighbor_grad = cell_grad_dot_normal(*fi->neighborInfo());
    Real face_grad = 0.0;
    Moose::FV::interpolate(
        Moose::FV::InterpMethod::Average, face_grad, elem_grad, neighbor_grad, *fi, true);
    return face_grad;
  }

  if (_pressure_boundary_normal_gradient_valid)
    return libmesh_map_find(_pressure_boundary_normal_gradient, fi->id());

  const bool elem_is_fluid = hasBlocks(fi->elemPtr()->subdomain_id());
  const ElemInfo & elem_info = elem_is_fluid ? *fi->elemInfo() : *fi->neighborInfo();
  return cell_grad_dot_normal(elem_info);
}

bool
ConservativeSharpInterfaceRhieChowMassFluxBase::useExplicitHydrostaticPredictorForce() const
{
  return _add_capillary_hydrostatic_flux && !_suppress_startup_pressure_predictor_flux_sources &&
         !_suppress_explicit_hydrostatic_pressure_flux;
}

bool
ConservativeSharpInterfaceRhieChowMassFluxBase::useDiscreteHydrostaticPredictorFaceForce() const
{
  return useExplicitHydrostaticPredictorForce() &&
         _hydrostatic_predictor_discretization == "discrete_density_sn_grad";
}

bool
ConservativeSharpInterfaceRhieChowMassFluxBase::useLegacyHydrostaticPredictorFaceAcceleration() const
{
  return useExplicitHydrostaticPredictorForce() &&
         _hydrostatic_predictor_discretization == "legacy_acceleration" &&
         _hydrostatic_density_gradient_face_acceleration;
}

bool
ConservativeSharpInterfaceRhieChowMassFluxBase::hasHydrostaticPredictorFaceForce() const
{
  return useDiscreteHydrostaticPredictorFaceForce() ||
         useLegacyHydrostaticPredictorFaceAcceleration();
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::computeDiscreteHydrostaticPredictorFaceNormalForceDensity(
    const FaceInfo * fi, const Moose::StateArg & time_arg) const
{
  if (!fi || !useDiscreteHydrostaticPredictorFaceForce())
    return 0.0;

  const Real ghf = _gravity * (fi->faceCentroid() - _reference_pressure_point);
  return -ghf * computeFaceNormalDensityGradient(fi, time_arg);
}

bool
ConservativeSharpInterfaceRhieChowMassFluxBase::populateMomentumPredictorBodyForceFaceField(
    FaceVectorField & face_field, const Moose::StateArg & time_arg) const
{
  const bool have_surface_face_force = _surface_tension_face_acceleration;
  const bool have_hydrostatic_face_force = hasHydrostaticPredictorFaceForce();

  if (!have_surface_face_force && !have_hydrostatic_face_force)
    return false;

  std::vector<std::unique_ptr<NumericVector<Number>>> owned_raw_ainv;
  std::vector<PetscVectorReader> raw_ainv_readers;
  buildSharpFaceRawAinvReaders(owned_raw_ainv, raw_ainv_readers);

  for (const auto * fi : _sharp_interface_face_info)
  {
    const auto state = buildSharpFaceOperatorState(fi, time_arg, raw_ainv_readers);
    Real total_normal_force_density = 0.0;
    bool have_face_force = false;

    if (have_surface_face_force)
    {
      const auto surface_accel =
          evaluateBoundaryAwareVectorFunctor(_surface_tension_face_acceleration, fi, time_arg);
      const Real surface_mass_flux_density = projectPhysicalMassFluxDensity(
          state.face_rho, state.face_raw_ainv, surface_accel, state.face_normal);
      if (std::abs(state.normal_raw_ainv) > libMesh::TOLERANCE)
      {
        total_normal_force_density += surface_mass_flux_density / state.normal_raw_ainv;
        have_face_force = true;
      }
    }

    if (have_hydrostatic_face_force)
    {
      if (useDiscreteHydrostaticPredictorFaceForce())
      {
        total_normal_force_density +=
            computeDiscreteHydrostaticPredictorFaceNormalForceDensity(fi, time_arg);
        have_face_force = true;
      }
      else if (useLegacyHydrostaticPredictorFaceAcceleration())
      {
        const auto hydrostatic_accel = evaluateBoundaryAwareVectorFunctor(
            _hydrostatic_density_gradient_face_acceleration, fi, time_arg);
        const Real hydrostatic_mass_flux_density = projectPhysicalMassFluxDensity(
            state.face_rho, state.face_raw_ainv, hydrostatic_accel, state.face_normal);
        if (std::abs(state.normal_raw_ainv) > libMesh::TOLERANCE)
        {
          total_normal_force_density += hydrostatic_mass_flux_density / state.normal_raw_ainv;
          have_face_force = true;
        }
      }
    }

    face_field[fi->id()] =
        have_face_force ? total_normal_force_density * state.face_normal : RealVectorValue();
  }

  return true;
}

RealVectorValue
ConservativeSharpInterfaceRhieChowMassFluxBase::evaluateCellMomentumPredictorPressureForceDensity(
    const ElemInfo & elem_info) const
{
  RealVectorValue pressure_force_density;

  for (const auto dim_i : make_range(_dim))
    pressure_force_density(dim_i) = -_p->gradSlnComponent(elem_info, dim_i);

  return pressure_force_density;
}

RealVectorValue
ConservativeSharpInterfaceRhieChowMassFluxBase::evaluateCellHydrostaticMomentumPredictorBodyForceDensity(
    const ElemInfo * elem_info, const Moose::StateArg & time_arg) const
{
  RealVectorValue body_force_density;

  if (!elem_info)
    return body_force_density;

  if (!useExplicitHydrostaticPredictorForce())
    return body_force_density;

  const auto * const elem = elem_info->elem();
  const auto elem_arg = makeElemArg(elem);

  if (_hydrostatic_predictor_discretization == "legacy_acceleration" &&
      _hydrostatic_density_gradient_cell_acceleration)
  {
    const Real rho = _rho(elem_arg, time_arg);
    body_force_density +=
        rho *
        MetaPhysicL::raw_value((*_hydrostatic_density_gradient_cell_acceleration)(elem_arg, time_arg));
    return body_force_density;
  }

  const Real gh = _gravity * (elem->vertex_average() - _reference_pressure_point);
  const RealVectorValue grad_rho = MetaPhysicL::raw_value(_rho.gradient(elem_arg, time_arg));
  body_force_density += -gh * grad_rho;
  return body_force_density;
}

RealVectorValue
ConservativeSharpInterfaceRhieChowMassFluxBase::evaluateLegacyMomentumPredictorBodyForceDensity(
    const ElemInfo * elem_info, const Moose::StateArg & time_arg) const
{
  RealVectorValue body_force_density;

  if (!elem_info)
    return body_force_density;

  const auto * const elem = elem_info->elem();
  const auto elem_arg = makeElemArg(elem);
  const Real rho = _rho(elem_arg, time_arg);

  if (_surface_tension_cell_acceleration)
    body_force_density +=
        rho * MetaPhysicL::raw_value((*_surface_tension_cell_acceleration)(elem_arg, time_arg));

  body_force_density += evaluateCellHydrostaticMomentumPredictorBodyForceDensity(elem_info, time_arg);

  return body_force_density;
}

bool
ConservativeSharpInterfaceRhieChowMassFluxBase::populateMomentumPredictorPressureForceFaceField(
    FaceVectorField & face_field, const Moose::StateArg & time_arg) const
{
  bool found_nonzero_force = false;
  std::vector<std::unique_ptr<NumericVector<Number>>> owned_raw_ainv;
  std::vector<PetscVectorReader> raw_ainv_readers;
  buildSharpFaceRawAinvReaders(owned_raw_ainv, raw_ainv_readers);
  for (const auto * fi : _sharp_interface_face_info)
  {
    const auto state = buildSharpFaceOperatorState(fi, time_arg, raw_ainv_readers);
    const Real normal_force_density = state.negative_sn_grad_p;

    if (std::abs(normal_force_density) > libMesh::TOLERANCE)
      found_nonzero_force = true;

    face_field[fi->id()] = normal_force_density * state.face_normal;
  }

  return found_nonzero_force;
}

RealVectorValue
ConservativeSharpInterfaceRhieChowMassFluxBase::evaluateFaceBasedMomentumPredictorPressureForceDensity(
    const ElemInfo * elem_info,
    const Moose::StateArg & time_arg,
    const FaceVectorField * face_field) const
{
  if (!elem_info || !face_field)
    return RealVectorValue();

  return (*face_field)(makeElemArg(elem_info->elem()), time_arg);
}

RealVectorValue
ConservativeSharpInterfaceRhieChowMassFluxBase::reconstructFaceVectorFieldToCellSourceDensity(
    const ElemInfo * elem_info,
    const Moose::StateArg & time_arg,
    const FaceVectorField & face_field) const
{
  FaceScalarField face_normal_scalar(
      _moose_mesh, blockIDs(), "momentum_predictor_reconstructed_face_normal_scalar");

  for (const auto * fi : _sharp_interface_face_info)
    face_normal_scalar[fi->id()] =
        face_field(makeCenteredFaceArg(fi), time_arg) * fi->normal();

  return reconstructFaceNormalScalarToCellVector(elem_info, time_arg, face_normal_scalar);
}

RealVectorValue
ConservativeSharpInterfaceRhieChowMassFluxBase::evaluateFaceBasedMomentumPredictorBodyForceDensity(
    const ElemInfo * elem_info,
    const Moose::StateArg & time_arg,
    const FaceVectorField * face_field) const
{
  RealVectorValue body_force_density;

  if (!elem_info)
    return body_force_density;

  const auto * const elem = elem_info->elem();
  const auto elem_arg = makeElemArg(elem);
  const Real rho = _rho(elem_arg, time_arg);
  const bool have_face_hydrostatic = hasHydrostaticPredictorFaceForce();

  if (face_field)
    body_force_density += (*face_field)(elem_arg, time_arg);

  if (!_surface_tension_face_acceleration && _surface_tension_cell_acceleration)
    body_force_density +=
        rho * MetaPhysicL::raw_value((*_surface_tension_cell_acceleration)(elem_arg, time_arg));

  if (!have_face_hydrostatic)
    body_force_density += evaluateCellHydrostaticMomentumPredictorBodyForceDensity(elem_info, time_arg);

  return body_force_density;
}

RealVectorValue
ConservativeSharpInterfaceRhieChowMassFluxBase::evaluateMomentumPredictorBodyForceDensity(
    const ElemInfo * elem_info,
    const Moose::StateArg & time_arg,
    const FaceVectorField * face_field) const
{
  if (face_field)
    return evaluateFaceBasedMomentumPredictorBodyForceDensity(elem_info, time_arg, face_field);

  return evaluateLegacyMomentumPredictorBodyForceDensity(elem_info, time_arg);
}

RealVectorValue
ConservativeSharpInterfaceRhieChowMassFluxBase::computeDefaultTransientProjectionFaceAcceleration(
    const FaceInfo * fi, const Moose::StateArg & time_arg) const
{
  (void)fi;
  (void)time_arg;

  // Keep the explicit transient_projection_face_acceleration hook for callers
  // that provide a discretized face acceleration. The default OpenFOAM-style
  // phi-based ddtCorr analog is synthesized directly in
  // computeDefaultTransientProjectionVolumetricFlux() because it is a face-flux
  // correction, not a unique physical acceleration field.
  return RealVectorValue();
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::pressureVelocityWritebackFluxDensity(const FaceInfo * fi) const
{
  const Real phig_mass_flux =
      volumetricNormalFluxToPressureMassFluxDensity(
          fi, libmesh_map_find(_phig_flux, fi->id()));
  const Real pressure_equation_mass_flux =
      volumetricNormalFluxToPressureMassFluxDensity(
          fi, libmesh_map_find(_pressure_equation_flux, fi->id()));
  return pressure_equation_mass_flux - phig_mass_flux;
}

namespace
{
Real
pressureVelocityWritebackVolumetricFlux(const FaceInfo * fi,
                                        const std::unordered_map<dof_id_type, Real> & phig_flux,
                                        const std::unordered_map<dof_id_type, Real> & pressure_equation_flux)
{
  // The writeback reconstruction must use the same signed face correction that
  // acts on the transport predictor flux (-phiHbyA) when forming the accepted
  // corrected transport flux.
  return libmesh_map_find(pressure_equation_flux, fi->id()) - libmesh_map_find(phig_flux, fi->id());
}
}

void
ConservativeSharpInterfaceRhieChowMassFluxBase::updateAdditionalPressureFluxFunctors(const bool with_updated_pressure,
                                                                     const bool verbose)
{
  _pressure_coupled_velocity_correction_valid = false;
  _pressure_predictor_face_state_valid = false;

  const auto time_arg = Moose::currentState();

  std::vector<std::unique_ptr<NumericVector<Number>>> owned_raw_ainv;
  std::vector<PetscVectorReader> raw_ainv_readers;
  buildSharpFaceRawAinvReaders(owned_raw_ainv, raw_ainv_readers);
  std::vector<PetscVectorReader> pressure_gradient_readers;
  buildSelectedPressureGradientReaders(with_updated_pressure, pressure_gradient_readers);

  for (const auto * fi : _sharp_interface_face_info)
  {
    const Real predictor_operator_mass_flux = pressurePredictorFlux(fi);
    const auto state =
        buildSharpFaceOperatorState(fi, time_arg, raw_ainv_readers, &pressure_gradient_readers);
    const RealVectorValue pressure_face_raw_ainv =
        interpolatePressureFaceRawAinv(fi, raw_ainv_readers);
    const Real normal_pressure_ainv =
        computeFaceNormalRawAinv(pressure_face_raw_ainv, state.face_normal);
    Real transient_projection_flux = 0.0;
    Real surface_tension_flux = 0.0;
    Real hydrostatic_flux = 0.0;

    if (_add_transient_projection_flux && !_suppress_startup_pressure_predictor_flux_sources)
    {
      if (_transient_projection_face_acceleration)
      {
        const RealVectorValue transient_acceleration =
            evaluateBoundaryAwareVectorFunctor(_transient_projection_face_acceleration, fi, time_arg);
        if (transient_acceleration != RealVectorValue())
        {
          const Real transient_mass_flux_density = projectPhysicalMassFluxDensity(
              state.face_rho, state.face_raw_ainv, transient_acceleration, state.face_normal);
          transient_projection_flux =
              -massFluxDensityToVolumetricNormalFlux(fi, transient_mass_flux_density);
        }
      }
      else
        transient_projection_flux =
            -computeDefaultTransientProjectionVolumetricFlux(fi, time_arg, state);
    }

    _transient_projection_flux[fi->id()] = transient_projection_flux;

    if (_add_capillary_hydrostatic_flux && !_suppress_startup_pressure_predictor_flux_sources)
    {
      RealVectorValue surface_acceleration;
      if (_surface_tension_face_acceleration)
        surface_acceleration =
            evaluateBoundaryAwareVectorFunctor(_surface_tension_face_acceleration, fi, time_arg);
      else if (_surface_tension_cell_acceleration)
        surface_acceleration =
            interpolateCellVectorFunctorToFace(_surface_tension_cell_acceleration, fi, time_arg);

      if (surface_acceleration != RealVectorValue())
      {
        const Real surface_mass_flux_density = projectPhysicalMassFluxDensity(
            state.face_rho, state.face_raw_ainv, surface_acceleration, state.face_normal);
        surface_tension_flux =
            massFluxDensityToVolumetricNormalFlux(fi, surface_mass_flux_density);
      }

      if (!_suppress_explicit_hydrostatic_pressure_flux)
      {
        const Real ghf = _gravity * (fi->faceCentroid() - _reference_pressure_point);
        const Real sn_grad_rho = computeFaceNormalDensityGradient(fi, time_arg);
        hydrostatic_flux = -ghf * sn_grad_rho * normal_pressure_ainv;
      }
    }

    const Real phig_flux = surface_tension_flux + hydrostatic_flux;
    const Real predictor_operator_volumetric_flux =
        massFluxDensityToVolumetricNormalFlux(fi, predictor_operator_mass_flux);
    const Real pressure_predictor_base_flux =
        predictor_operator_volumetric_flux + transient_projection_flux;

    _debug_update_hydrostatic_branch_taken[fi->id()] = std::abs(phig_flux) > libMesh::TOLERANCE;
    _debug_update_hydrostatic_face_mass_flux_density_raw[fi->id()] = hydrostatic_flux;
    _debug_update_physical_capillary_hydrostatic_flux[fi->id()] = phig_flux;
    _capillary_hydrostatic_flux[fi->id()] = phig_flux;
    _phig_flux[fi->id()] = phig_flux;
    _pressure_Ainv[fi->id()] = pressure_face_raw_ainv;
    _pressure_predictor_base_flux[fi->id()] = pressure_predictor_base_flux;
    _phiHbyA_flux[fi->id()] = pressure_predictor_base_flux + phig_flux;
    _pressure_predictor_flux[fi->id()] = _phiHbyA_flux[fi->id()];
    _pressure_predictor_mass_flux[fi->id()] =
        transportMassFluxDensityFromVolumetricPhi(fi, _pressure_predictor_flux[fi->id()], time_arg);

    if (verbose)
    {
      _console << "Sharp-interface predictor on face " << fi->id() << ": transient_source_flux="
               << _transient_projection_flux[fi->id()]
               << ", capillary_hydrostatic_source_flux="
               << _capillary_hydrostatic_flux[fi->id()]
               << ", phig_flux=" << _phig_flux[fi->id()]
               << ", pressure_predictor_base_flux="
               << _pressure_predictor_base_flux[fi->id()]
               << ", phiHbyA_flux=" << _phiHbyA_flux[fi->id()]
               << std::endl;
    }
  }

  _pressure_predictor_face_state_valid = true;
}

void
ConservativeSharpInterfaceRhieChowMassFluxBase::updatePressureCoupledVelocityCorrectionFaceField(
    const Moose::StateArg & time_arg)
{
  if (!_apply_pressure_velocity_writeback)
  {
    for (const auto * fi : flowFaceInfo())
    {
      _pressure_coupled_cell_reconstruction_scalar[fi->id()] = 0.0;
      _pressure_coupled_cell_reconstruction_vector[fi->id()] = RealVectorValue();
    }

    _pressure_coupled_velocity_correction_valid = true;
    return;
  }

  if (!_pressure_equation_flux_valid)
    cachePressureEquationFlux();

  Real max_abs_normal_pressure_ainv = 0.0;
  for (const auto * fi : flowFaceInfo())
  {
    const Real normal_pressure_ainv = pressureFaceScalarDiffusionCoefficient(fi, time_arg);
    if (std::isfinite(normal_pressure_ainv))
      max_abs_normal_pressure_ainv =
          std::max(max_abs_normal_pressure_ainv, std::abs(normal_pressure_ainv));
  }

  // The tolerance is documented as a fraction of the active-face maximum.
  // Using max(1, max_abs_normal_pressure_ainv) turns it into an unintended
  // absolute cutoff whenever the live face-normal Ainv values are << 1, which
  // suppresses writeback reconstruction on the very interface faces we need to
  // correct. Keep the cutoff relative to the active maximum instead.
  const Real degenerate_normal_pressure_ainv_tol =
      std::max(std::numeric_limits<Real>::min(),
               _pressure_writeback_face_ainv_relative_tolerance * max_abs_normal_pressure_ainv);

  for (const auto * fi : flowFaceInfo())
  {
    // Use the same primitive as interFoam:
    //   U = HbyA + rAU * reconstruct((phig - pEqn.flux())/rAUf)
    // For the cell velocity correction we therefore normalize the accepted
    // volumetric pressure-correction face flux, not the density-weighted
    // audit/writeback mass-flux-density view of that same correction.
    const Real pressure_writeback_volumetric_flux =
        pressureVelocityWritebackVolumetricFlux(fi, _phig_flux, _pressure_equation_flux);
    const Real normal_pressure_ainv = pressureFaceScalarDiffusionCoefficient(fi, time_arg);

    // Keep the mass-flux-density branch for audits, but publish a single
    // normalized volumetric face primitive for all faces. The post-pEqn cell
    // writeback then uses one face-to-cell reconstruction path everywhere
    // instead of mixing a smooth reconstruction with a sharp overlay.
    const Real reconstruction_scalar =
        (std::isfinite(normal_pressure_ainv) &&
                 std::abs(normal_pressure_ainv) > degenerate_normal_pressure_ainv_tol
             ? pressure_writeback_volumetric_flux / normal_pressure_ainv
             : 0.0);
    _pressure_coupled_cell_reconstruction_scalar[fi->id()] = reconstruction_scalar;
    // FaceCenteredMapFunctor reconstructs vector-valued face data using faceArea() only.
    // Fold the coordinate transform factor into the stored face primitive so the
    // resulting cell writeback matches the existing FV geometry weighting.
    _pressure_coupled_cell_reconstruction_vector[fi->id()] =
        fi->normal() * (reconstruction_scalar * fi->faceCoord());
  }

  _pressure_coupled_velocity_correction_valid = true;
}

bool
ConservativeSharpInterfaceRhieChowMassFluxBase::useConstrainedBoundaryPredictorState(const FaceInfo * fi) const
{
  if (!fi || _vel[0]->isInternalFace(*fi))
    return false;

  bool use_constrained_boundary_state = _vel[0]->isDirichletBoundaryFace(*fi);
  if (!use_constrained_boundary_state && !fi->boundaryIDs().empty())
  {
    mooseAssert(fi->boundaryIDs().size() == 1,
                "Expected at most one physical boundary id on a FV boundary face.");
    const auto boundary_id = *fi->boundaryIDs().begin();

    for (const auto dim_i : make_range(_dim))
      if (auto * bc_pointer = _vel[dim_i]->getBoundaryCondition(boundary_id))
        if (auto * adv_diff_bc = dynamic_cast<LinearFVAdvectionDiffusionBC *>(bc_pointer))
        {
          adv_diff_bc->setupFaceData(
              fi,
              fi->faceType(std::make_pair(_vel[dim_i]->number(), _vel[dim_i]->sys().number())));
          if (adv_diff_bc->computeBoundaryGradientMatrixContribution() > 0.0)
          {
            use_constrained_boundary_state = true;
            break;
          }
        }
  }

  return use_constrained_boundary_state;
}

void
ConservativeSharpInterfaceRhieChowMassFluxBase::updateVelocityBoundaryState()
{
  const auto time_arg = Moose::currentState();

  _velocity_boundary_state_valid = false;
  for (auto & component_face_values : _boundary_velocity_face_values)
    component_face_values.clear();

  for (const auto * fi : flowFaceInfo())
  {
    if (_vel[0]->isInternalFace(*fi))
      continue;

    RealVectorValue density_times_velocity;
    const bool elem_is_fluid = hasBlocks(fi->elemPtr()->subdomain_id());
    const Elem * const boundary_elem = elem_is_fluid ? fi->elemPtr() : fi->neighborPtr();
    const Real boundary_normal_multiplier = elem_is_fluid ? 1.0 : -1.0;
    const Moose::FaceArg boundary_face{
        fi, Moose::FV::LimiterType::CentralDifference, true, false, boundary_elem, nullptr};
    const Real face_rho = _rho(boundary_face, time_arg);

    for (const auto component : index_range(_vel))
    {
      const Real boundary_velocity = boundaryPhysicalVelocityComponent(fi, component, time_arg);
      _boundary_velocity_face_values[component][fi->id()] = boundary_velocity;
      density_times_velocity(component) = boundary_normal_multiplier * face_rho * boundary_velocity;
    }

    if (!_pressure_equation_flux_valid)
      _face_mass_flux[fi->id()] = density_times_velocity * fi->normal();
  }

  _velocity_boundary_state_valid = true;
  cacheCurrentCorrectedVolumetricFlux();
}

ConservativeSharpInterfaceRhieChowMassFluxBase::PressureCorrectionReconstructionDebug
ConservativeSharpInterfaceRhieChowMassFluxBase::reconstructOpenFoamFaceScalarToCellVectorDebug(
    const ElemInfo * elem_info,
    const Moose::StateArg & time_arg,
    const FaceScalarField & scalar_field) const
{
  PressureCorrectionReconstructionDebug debug;

  if (!elem_info)
    return debug;

  const Elem * const elem = elem_info->elem();
  if (!elem)
    return debug;

  DenseMatrix<Real> normal_matrix(_dim, _dim);
  DenseVector<Real> rhs(_dim);
  DenseVector<Real> solution(_dim);

  for (const auto i : make_range(_dim))
  {
    rhs(i) = 0.0;
    solution(i) = 0.0;
    for (const auto j : make_range(_dim))
      normal_matrix(i, j) = 0.0;
  }

  for (const auto side : make_range(elem->n_sides()))
  {
    const Elem * const loc_neighbor = elem->neighbor_ptr(side);
    const bool elem_has_fi = Moose::FV::elemHasFaceInfo(*elem, loc_neighbor);
    const FaceInfo * const fi_loc =
        _moose_mesh.faceInfo(elem_has_fi ? elem : loc_neighbor,
                             elem_has_fi ? side : loc_neighbor->which_neighbor_am_i(elem));
    if (!fi_loc)
      continue;

    ++debug.contributing_faces;

    const Real face_measure = fi_loc->faceArea() * fi_loc->faceCoord();
    if (face_measure <= 0.0)
      continue;

    // OpenFOAM fvc::reconstruct uses
    //   inv(surfaceSum(SfHat*Sf)) & surfaceSum(SfHat*ssf)
    // with SfHat = Sf/|Sf|. In the local cell frame, this becomes a sum over
    // face measures of n_out \otimes n_out and n_out * ssf_cell, where the
    // scalar face field is sign-adjusted onto the current cell's outward
    // normal convention on internal faces.
    const Real orientation = fi_loc->elemPtr() == elem ? 1.0 : -1.0;
    const RealVectorValue outward_normal = orientation * fi_loc->normal();
    const Real psi_f = orientation * scalar_field(makeCenteredFaceArg(fi_loc), time_arg);

    for (const auto i : make_range(_dim))
    {
      rhs(i) += face_measure * outward_normal(i) * psi_f;
      for (const auto j : make_range(_dim))
        normal_matrix(i, j) += face_measure * outward_normal(i) * outward_normal(j);
    }
  }

  if (_dim == 1)
    debug.singular = std::abs(normal_matrix(0, 0)) <= libMesh::TOLERANCE;
  else if (_dim == 2)
    debug.singular = std::abs(normal_matrix(0, 0) * normal_matrix(1, 1) -
                              normal_matrix(0, 1) * normal_matrix(1, 0)) <= libMesh::TOLERANCE;
  else if (_dim == 3)
  {
    const Real det =
        normal_matrix(0, 0) * (normal_matrix(1, 1) * normal_matrix(2, 2) -
                               normal_matrix(1, 2) * normal_matrix(2, 1)) -
        normal_matrix(0, 1) * (normal_matrix(1, 0) * normal_matrix(2, 2) -
                               normal_matrix(1, 2) * normal_matrix(2, 0)) +
        normal_matrix(0, 2) * (normal_matrix(1, 0) * normal_matrix(2, 1) -
                               normal_matrix(1, 1) * normal_matrix(2, 0));
    debug.singular = std::abs(det) <= libMesh::TOLERANCE;
  }

  if (!debug.singular)
    normal_matrix.lu_solve(rhs, solution);

  for (const auto i : make_range(3))
  {
    debug.rhs[i] = i < _dim ? rhs(i) : 0.0;
    debug.solution[i] = i < _dim ? solution(i) : 0.0;
    for (const auto j : make_range(3))
      debug.normal_matrix[3 * i + j] = (i < _dim && j < _dim) ? normal_matrix(i, j) : 0.0;
  }

  return debug;
}

RealVectorValue
ConservativeSharpInterfaceRhieChowMassFluxBase::reconstructFaceNormalScalarToCellVector(
    const ElemInfo * elem_info,
    const Moose::StateArg & time_arg,
    const FaceScalarField & scalar_field) const
{
  const auto debug =
      reconstructOpenFoamFaceScalarToCellVectorDebug(elem_info, time_arg, scalar_field);
  if (debug.singular)
    return RealVectorValue();

  RealVectorValue reconstructed_source;
  for (const auto i : make_range(_dim))
    reconstructed_source(i) = debug.solution[i];

  return reconstructed_source;
}

RealVectorValue
ConservativeSharpInterfaceRhieChowMassFluxBase::reconstructOpenFoamStylePressureCoupledCellVelocityDelta(
    const ElemInfo * elem_info, const Moose::StateArg & time_arg) const
{
  if (!elem_info || !_pressure_coupled_velocity_correction_valid)
    return RealVectorValue();

  // OpenFOAM-like writeback:
  //   U = HbyA + Ainv_cell * reconstruct(phi_correction / Ainv_face)
  // Publish the normalized face primitive as a face-vector functor and let the
  // built-in MOOSE FaceCenteredMapFunctor perform the face-to-cell
  // reconstruction on the active element geometry.
  const RealVectorValue reconstructed_operator =
      _pressure_coupled_cell_reconstruction_vector(makeElemArg(elem_info->elem()), time_arg);

  RealVectorValue pressure_delta;
  for (const auto system_i : index_range(_momentum_implicit_systems))
  {
    const auto & dof_indices =
        elem_info->dofIndices()[_global_momentum_system_numbers[system_i]];
    if (dof_indices.empty())
      continue;

    pressure_delta(system_i) = cellAinvRaw(system_i, dof_indices[0]) *
                               reconstructed_operator(system_i);
  }

  return pressure_delta;
}

RealVectorValue
ConservativeSharpInterfaceRhieChowMassFluxBase::reconstructPressureCoupledCellVelocityDelta(
    const ElemInfo * elem_info, const Moose::StateArg & time_arg) const
{
  if (!elem_info || !_pressure_coupled_velocity_correction_valid || !_apply_pressure_velocity_writeback)
    return RealVectorValue();

  return reconstructOpenFoamStylePressureCoupledCellVelocityDelta(elem_info, time_arg);
}

void
ConservativeSharpInterfaceRhieChowMassFluxBase::applyAdditionalFaceMassFluxCorrection()
{
  // computeFaceMassFlux() now uses the explicit phiHbyA face state, which in the
  // sharp-interface path already includes the transient and capillary/hydrostatic
  // predictor-source fluxes.
  // There is therefore no additional post-solve face-flux correction to apply here.
}

void
ConservativeSharpInterfaceRhieChowMassFluxBase::computeProvisionalCellVelocity()
{
  const auto time_arg = Moose::currentState();
  updatePressureCoupledVelocityCorrectionFaceField(time_arg);
  writeProvisionalVelocityToMomentumSolution(time_arg);
  _velocity_boundary_state_valid = false;
}

void
ConservativeSharpInterfaceRhieChowMassFluxBase::computeCellVelocity()
{
  computeProvisionalCellVelocity();
}
