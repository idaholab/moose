#include "ConservativeSharpInterfaceRhieChowMassFluxBase.h"

#include "LinearFVAdvectionDiffusionBC.h"
#include "LinearFVPressureInletOutletMomentumBC.h"
#include "LinearFVPressureInletOutletVelocityBC.h"
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
#include <cmath>
#include <fstream>
#include <iomanip>
#include <limits>
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
      "reference-solver-style velocity writeback. Faces whose pressure-space normal Ainv falls "
      "below "
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
  params.addParam<std::string>(
      "sharp_flux_diagnostic_file_base",
      "",
      "Optional file base for a one-time CSV dump of the sharp-interface face-flux "
      "decomposition.");
  params.addParam<Real>(
      "sharp_flux_diagnostic_time",
      -1.0,
      "Simulation time at which to dump the sharp-interface face-flux decomposition. A negative "
      "value disables the dump.");
  params.addRangeCheckedParam<Real>(
      "sharp_flux_diagnostic_front_band_width",
      0.01,
      "sharp_flux_diagnostic_front_band_width>=0",
      "Width behind the current alpha=0.5 front used to select faces for the diagnostic dump.");
  params.addParam<bool>(
      "sharp_flux_diagnostic_all_faces",
      false,
      "Whether to dump every flow face instead of only faces in the front diagnostic band.");
  params.addParam<bool>(
      "require_vof_rho_phi_functor",
      false,
      "Require the alpha-owned VOF rhoPhi and alphaPhi functors. Enable this for OpenFOAM "
      "incompressibleVoF parity runs so momentum advection cannot fall back to rho_f * phi.");
  params.addParam<bool>(
      "enforce_vof_rho_phi_contract",
      false,
      "Check after each alpha solve that the published VOF rhoPhi satisfies "
      "rhoPhi = rho_g * vof_transport_phi * area + (rho_l-rho_g) * alphaPhi_limited.");
  params.addRangeCheckedParam<Real>("vof_rho_phi_contract_abs_tol",
                                    1e-10,
                                    "vof_rho_phi_contract_abs_tol>=0",
                                    "Absolute tolerance for the VOF rhoPhi contract check.");
  params.addRangeCheckedParam<Real>("vof_rho_phi_contract_rel_tol",
                                    1e-10,
                                    "vof_rho_phi_contract_rel_tol>=0",
                                    "Relative tolerance for the VOF rhoPhi contract check.");
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

ConservativeSharpInterfaceRhieChowMassFluxBase::ConservativeSharpInterfaceRhieChowMassFluxBase(
    const InputParameters & params)
  : RhieChowMassFlux(params),
    _transient_projection_flux(_moose_mesh, blockIDs(), "transient_projection_flux"),
    _capillary_hydrostatic_flux(_moose_mesh, blockIDs(), "capillary_hydrostatic_flux"),
    _pressure_equation_volumetric_flux(
        _moose_mesh, blockIDs(), "pressure_equation_volumetric_flux"),
    _pressure_correction_phi(_moose_mesh, blockIDs(), "pressure_correction_phi"),
    _corrected_face_phi(_moose_mesh, blockIDs(), "corrected_face_phi"),
    _previous_timestep_corrected_face_phi(
        _moose_mesh, blockIDs(), "previous_timestep_corrected_face_phi"),
    _vof_transport_phi(_moose_mesh, blockIDs(), "vof_transport_phi"),
    _pressure_coupled_cell_reconstruction_scalar(
        _moose_mesh, blockIDs(), "pressure_coupled_cell_reconstruction_scalar"),
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
    _density_sn_grad_limiter_coefficient(getParam<Real>("density_sn_grad_limiter_coefficient")),
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
    _sharp_flux_diagnostic_file_base(getParam<std::string>("sharp_flux_diagnostic_file_base")),
    _sharp_flux_diagnostic_time(getParam<Real>("sharp_flux_diagnostic_time")),
    _sharp_flux_diagnostic_front_band_width(
        getParam<Real>("sharp_flux_diagnostic_front_band_width")),
    _sharp_flux_diagnostic_all_faces(getParam<bool>("sharp_flux_diagnostic_all_faces")),
    _require_vof_rho_phi_functor(getParam<bool>("require_vof_rho_phi_functor")),
    _enforce_vof_rho_phi_contract(getParam<bool>("enforce_vof_rho_phi_contract")),
    _vof_rho_phi_contract_abs_tol(getParam<Real>("vof_rho_phi_contract_abs_tol")),
    _vof_rho_phi_contract_rel_tol(getParam<Real>("vof_rho_phi_contract_rel_tol")),
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
    UserObject::_subproblem.addFunctor(
        "transient_projection_flux", _transient_projection_flux, tid);
    UserObject::_subproblem.addFunctor(
        "capillary_hydrostatic_flux", _capillary_hydrostatic_flux, tid);
    UserObject::_subproblem.addFunctor(
        "pressure_equation_volumetric_flux", _pressure_equation_volumetric_flux, tid);
    UserObject::_subproblem.addFunctor("pressure_correction_phi", _pressure_correction_phi, tid);
    UserObject::_subproblem.addFunctor("corrected_face_phi", _corrected_face_phi, tid);
    UserObject::_subproblem.addFunctor(
        "previous_timestep_corrected_face_phi", _previous_timestep_corrected_face_phi, tid);
    UserObject::_subproblem.addFunctor("vof_transport_phi", _vof_transport_phi, tid);
    UserObject::_subproblem.addFunctor("pressure_coupled_cell_reconstruction_scalar",
                                       _pressure_coupled_cell_reconstruction_scalar,
                                       tid);
  }

  if (!dynamic_cast<SIMPLE *>(getMooseApp().getExecutioner()) &&
      !dynamic_cast<PIMPLE *>(getMooseApp().getExecutioner()))
    mooseError(this->name(),
               " should only be used with a linear segregated thermal-hydraulics solver!");

  rebuildSharpInterfaceFaceInfo();
  initializeAdditionalPressureFluxStorage();
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::getMassFlux(const FaceInfo & fi) const
{
  if (_vof_rho_phi)
  {
    const Real face_measure = fi.faceArea() * fi.faceCoord();
    return face_measure > 0.0 ? vofRhoPhiIntegrated(fi) / face_measure : 0.0;
  }

  if (_require_vof_rho_phi_functor)
    mooseError(name(),
               " requires alpha-owned VOF rhoPhi, but the functor '",
               _vof_rho_phi_name,
               "' is not available.");

  return RhieChowMassFlux::getMassFlux(fi);
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::rawRhieChowMassFlux(const FaceInfo & fi) const
{
  return RhieChowMassFlux::getMassFlux(fi);
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
ConservativeSharpInterfaceRhieChowMassFluxBase::storedPredictorOperatorPhi(
    const FaceInfo & fi) const
{
  const Real face_measure = fi.faceArea() * fi.faceCoord();
  if (face_measure <= libMesh::TOLERANCE)
    return 0.0;

  // _pressure_predictor_base_flux and _transient_projection_flux are cached as
  // OpenFOAM-style integrated fluxes. Convert back to MOOSE's normal-flux
  // transport convention for diagnostics/accessors.
  return (-libmesh_map_find(_pressure_predictor_base_flux, fi.id()) -
          libmesh_map_find(_transient_projection_flux, fi.id())) /
         face_measure;
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::storedPressureCorrectionPhi(
    const FaceInfo & fi) const
{
  return libmesh_map_find(_pressure_correction_phi, fi.id());
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::storedOuterIterationPhi(const FaceInfo & fi) const
{
  return libmesh_map_find(_previous_timestep_corrected_face_phi, fi.id());
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::storedOuterIterationRhoPhiIntegrated(
    const FaceInfo & fi) const
{
  return transportIntegratedRhoPhiFromVolumetricPhi(
      &fi, libmesh_map_find(_previous_timestep_corrected_face_phi, fi.id()), Moose::currentState());
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::storedPredictorConvectiveMassFlux(
    const FaceInfo & fi) const
{
  return libmesh_map_find(_HbyA_flux, fi.id());
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::vofRhoPhiIntegrated(const FaceInfo & fi) const
{
  if (!_vof_rho_phi)
    return 0.0;

  return evaluateFaceScalarFunctor(_vof_rho_phi, &fi, Moose::currentState(), nullptr);
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::vofAlphaPhiLimitedIntegrated(
    const FaceInfo & fi) const
{
  if (!_vof_alpha_phi_limited)
    return 0.0;

  return evaluateFaceScalarFunctor(_vof_alpha_phi_limited, &fi, Moose::currentState(), nullptr);
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::vofBaseGasRhoPhiIntegrated(
    const FaceInfo & fi) const
{
  const auto time_arg = Moose::currentState();
  const Real face_measure = fi.faceArea() * fi.faceCoord();
  if (face_measure <= libMesh::TOLERANCE)
    return 0.0;

  const Real transport_phi = _vof_transport_phi_valid
                                 ? libmesh_map_find(_vof_transport_phi, fi.id())
                                 : storedOuterIterationPhi(fi);
  const Real gas_density =
      _gas_density ? evaluateCellBasedFaceScalarFunctor(_gas_density, &fi, time_arg) : 0.0;

  return transport_phi * face_measure * gas_density;
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::vofAlphaCorrectionRhoPhiIntegrated(
    const FaceInfo & fi) const
{
  const auto time_arg = Moose::currentState();
  const Real gas_density =
      _gas_density ? evaluateCellBasedFaceScalarFunctor(_gas_density, &fi, time_arg) : 0.0;
  const Real liquid_density =
      _liquid_density ? evaluateCellBasedFaceScalarFunctor(_liquid_density, &fi, time_arg) : 0.0;

  return (liquid_density - gas_density) * vofAlphaPhiLimitedIntegrated(fi);
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::vofRhoPhiReconstructedVolumetricPhi(
    const FaceInfo & fi) const
{
  if (!_vof_rho_phi || !_vof_alpha_phi_limited || !_liquid_density || !_gas_density)
    return 0.0;

  const auto time_arg = Moose::currentState();
  const Real face_measure = fi.faceArea() * fi.faceCoord();
  const Real gas_density = evaluateCellBasedFaceScalarFunctor(_gas_density, &fi, time_arg);
  const Real liquid_density = evaluateCellBasedFaceScalarFunctor(_liquid_density, &fi, time_arg);
  const Real rho_phi = vofRhoPhiIntegrated(fi);
  const Real alpha_phi = vofAlphaPhiLimitedIntegrated(fi);
  const Real gas_mass_flux = rho_phi - (liquid_density - gas_density) * alpha_phi;

  return std::abs(gas_density * face_measure) > libMesh::TOLERANCE
             ? gas_mass_flux / (gas_density * face_measure)
             : 0.0;
}

void
ConservativeSharpInterfaceRhieChowMassFluxBase::checkVOFRhoPhiContract() const
{
  if (!_enforce_vof_rho_phi_contract)
    return;

  if (!_vof_transport_phi_valid || !_vof_rho_phi || !_vof_alpha_phi_limited || !_liquid_density ||
      !_gas_density)
    return;

  const auto time_arg = Moose::currentState();
  Real max_error = 0.0;
  Real max_allowed = 0.0;
  dof_id_type max_face_id = std::numeric_limits<dof_id_type>::max();

  for (const auto * fi : flowFaceInfo())
  {
    const Real face_measure = fi->faceArea() * fi->faceCoord();
    const Real gas_density = evaluateCellBasedFaceScalarFunctor(_gas_density, fi, time_arg);
    const Real liquid_density = evaluateCellBasedFaceScalarFunctor(_liquid_density, fi, time_arg);
    const Real transport_phi = libmesh_map_find(_vof_transport_phi, fi->id());
    const Real alpha_phi = vofAlphaPhiLimitedIntegrated(*fi);
    const Real expected =
        gas_density * transport_phi * face_measure + (liquid_density - gas_density) * alpha_phi;
    const Real actual = vofRhoPhiIntegrated(*fi);
    const Real allowed =
        _vof_rho_phi_contract_abs_tol +
        _vof_rho_phi_contract_rel_tol * std::max({1.0, std::abs(expected), std::abs(actual)});
    const Real error = std::abs(actual - expected);

    if (error > max_error)
    {
      max_error = error;
      max_allowed = allowed;
      max_face_id = fi->id();
    }
  }

  if (max_error > max_allowed)
    mooseError(name(),
               " detected an inconsistent VOF rhoPhi handoff. The alpha-owned rhoPhi must satisfy "
               "rhoPhi = rho_g * vof_transport_phi * area + (rho_l-rho_g) * alphaPhi_limited. "
               "max_error=",
               max_error,
               ", allowed=",
               max_allowed,
               ", face_id=",
               max_face_id);
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
        normal_force_density +=
            cell_rho * (evaluateBoundaryAwareVectorFunctor(
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
              cell_rho * (evaluateBoundaryAwareVectorFunctor(
                              _hydrostatic_density_gradient_face_acceleration, fi_loc, time_arg) *
                          fi_loc->normal());
        }
        else if (_hydrostatic_density_gradient_cell_acceleration)
        {
          hydrostatic_force_density +=
              cell_rho * (MetaPhysicL::raw_value((*_hydrostatic_density_gradient_cell_acceleration)(
                              elem_arg, time_arg)) *
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
    const Real det = normal_matrix(0, 0) * (normal_matrix(1, 1) * normal_matrix(2, 2) -
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
ConservativeSharpInterfaceRhieChowMassFluxBase::predictorVelocityComponent(
    const ElemInfo & elem_info, const unsigned int component) const
{
  mooseAssert(component < _dim, "Momentum component index out of range.");

  const auto dof = elem_info.dofIndices()[_global_momentum_system_numbers[component]][0];
  return -(*_HbyA_raw[component])(dof);
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::getVolumetricFaceFlux(const FaceInfo & fi) const
{
  return libmesh_map_find(_corrected_face_phi, fi.id());
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::getVolumetricFaceFlux(
    const Moose::FV::InterpMethod m,
    const FaceInfo & fi,
    const Moose::StateArg & time,
    const THREAD_ID /*tid*/,
    bool libmesh_dbg_var(subtract_mesh_velocity)) const
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
    _pressure_coupled_cell_reconstruction_scalar[fi->id()] = 0.0;
    _pressure_predictor_flux[fi->id()] = 0.0;
    _pressure_predictor_mass_flux[fi->id()] = 0.0;
  }

  _vof_transport_phi_valid = false;
  _corrected_face_phi_seeded = preserve_corrected_face_phi && _corrected_face_phi_seeded;
  _pressure_coupled_velocity_correction_valid = false;
}

void
ConservativeSharpInterfaceRhieChowMassFluxBase::writePressureCorrectedVelocityToMomentumSolution(
    const Moose::StateArg & time_arg)
{
  std::vector<std::unique_ptr<NumericVector<Number>>> pressure_corrected_solution;
  pressure_corrected_solution.reserve(_momentum_implicit_systems.size());

  for (const auto system_i : index_range(_momentum_implicit_systems))
  {
    auto * momentum_system = _momentum_implicit_systems[system_i];
    mooseAssert(momentum_system && momentum_system->current_local_solution,
                "The requested momentum component is not linked to "
                "ConservativeSharpInterfaceRhieChowMassFluxBase.");
    pressure_corrected_solution.push_back(momentum_system->current_local_solution->zero_clone());
  }

  for (const auto & elem_info : _fe_problem.mesh().elemInfoVector())
  {
    if (!hasBlocks(elem_info->subdomain_id()))
      continue;

    const RealVectorValue pressure_delta =
        reconstructPressureCoupledCellVelocityDelta(elem_info, time_arg);

    for (const auto system_i : index_range(_momentum_implicit_systems))
    {
      const auto & dof_indices = elem_info->dofIndices()[_global_momentum_system_numbers[system_i]];
      if (dof_indices.empty())
        continue;

      pressure_corrected_solution[system_i]->set(dof_indices[0],
                                                 predictorVelocityComponent(*elem_info, system_i) +
                                                     pressure_delta(system_i));
    }
  }

  for (const auto system_i : index_range(_momentum_implicit_systems))
  {
    pressure_corrected_solution[system_i]->close();
    *(_momentum_implicit_systems[system_i]->solution) = *pressure_corrected_solution[system_i];
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
ConservativeSharpInterfaceRhieChowMassFluxBase::pressureBoundaryTargetFlux(
    const FaceInfo * fi, const Moose::StateArg & time_arg) const
{
  if (!fi)
    return 0.0;

  return boundaryPhysicalVolumetricFluxTarget(fi, time_arg) * fi->faceArea() * fi->faceCoord();
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::pressureBoundaryNormalAinv(
    const FaceInfo * fi) const
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
    const ElemInfo & elem_info,
    const unsigned int component,
    const Moose::StateArg & time_arg) const
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
ConservativeSharpInterfaceRhieChowMassFluxBase::pressureFaceNormalAinv(
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
  const RealVectorValue pressure_face_rau = _pressure_Ainv(makeCenteredFaceArg(fi), time_arg);
  return computeFaceNormalRawAinv(pressure_face_rau, fi->normal());
}

ConservativeSharpInterfaceRhieChowMassFluxBase::PressureVelocityFaceState
ConservativeSharpInterfaceRhieChowMassFluxBase::pressureVelocityFaceState(
    const FaceInfo * fi,
    const Moose::StateArg & time_arg,
    const Real degenerate_normal_pressure_ainv_tol) const
{
  PressureVelocityFaceState state;
  if (!fi)
    return state;

  state.predictor_valid = _pressure_predictor_face_state_valid;
  if (state.predictor_valid)
  {
    const Real face_measure = fi->faceArea() * fi->faceCoord();
    auto integratedToVolumetricPhi = [face_measure](const Real integrated_phi)
    { return face_measure > libMesh::TOLERANCE ? integrated_phi / face_measure : 0.0; };

    // The sharp pressure equation stores OpenFOAM-style area-integrated phi.
    // Convert back to MOOSE's normal-flux-density transport convention only at
    // the publication/writeback boundary.
    state.predictor_transport_phi =
        integratedToVolumetricPhi(-libmesh_map_find(_phiHbyA_flux, fi->id()));
    state.pressure_equation_phi =
        _pressure_equation_flux_valid
            ? integratedToVolumetricPhi(libmesh_map_find(_pressure_equation_flux, fi->id()))
            : 0.0;
    const Real phig_phi = integratedToVolumetricPhi(libmesh_map_find(_phig_flux, fi->id()));
    state.pressure_writeback_phi =
        _pressure_equation_flux_valid ? state.pressure_equation_phi + phig_phi : 0.0;
    state.corrected_transport_phi =
        state.predictor_transport_phi +
        (_apply_pressure_face_flux_correction ? state.pressure_equation_phi : 0.0);

    state.normal_pressure_ainv = pressureFaceNormalAinv(fi, time_arg);
    state.writeback_reconstruction_scalar =
        (std::isfinite(state.normal_pressure_ainv) &&
                 std::abs(state.normal_pressure_ainv) > degenerate_normal_pressure_ainv_tol
             ? state.pressure_writeback_phi / state.normal_pressure_ainv
             : 0.0);
  }
  else
  {
    const Real face_rho = predictorFaceDensity(fi, time_arg);
    state.corrected_transport_phi = std::abs(face_rho) > libMesh::TOLERANCE
                                        ? libmesh_map_find(_face_mass_flux, fi->id()) / face_rho
                                        : 0.0;
  }

  return state;
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::maxPressureFaceNormalAinv(
    const Moose::StateArg & time_arg) const
{
  Real max_abs_normal_pressure_ainv = 0.0;
  for (const auto * fi : flowFaceInfo())
  {
    const Real normal_pressure_ainv = pressureFaceNormalAinv(fi, time_arg);
    if (std::isfinite(normal_pressure_ainv))
      max_abs_normal_pressure_ainv =
          std::max(max_abs_normal_pressure_ainv, std::abs(normal_pressure_ainv));
  }

  return max_abs_normal_pressure_ainv;
}

void
ConservativeSharpInterfaceRhieChowMassFluxBase::cacheCurrentCorrectedVolumetricFlux(
    const Real degenerate_normal_pressure_ainv_tol)
{
  const auto time_arg = Moose::currentState();
  for (const auto * fi : flowFaceInfo())
  {
    const auto state = pressureVelocityFaceState(fi, time_arg, degenerate_normal_pressure_ainv_tol);
    _pressure_equation_volumetric_flux[fi->id()] = state.pressure_equation_phi;
    _pressure_correction_phi[fi->id()] = state.pressure_writeback_phi;
    // OpenFOAM parity contract:
    //   phi = phiHbyA + pEqn.flux()
    // This face flux is authoritative for continuity/transport. The reconstructed
    // cell velocity below is only the cell-centered U writeback branch and is not
    // projected back to overwrite this face flux.
    _corrected_face_phi[fi->id()] = state.corrected_transport_phi;
    _pressure_coupled_cell_reconstruction_scalar[fi->id()] = state.writeback_reconstruction_scalar;
  }
  _corrected_face_phi_seeded = true;
}

void
ConservativeSharpInterfaceRhieChowMassFluxBase::freezeVOFTransportState(
    const bool use_previous_timestep_flux)
{
  if (!_corrected_face_phi_seeded)
    cacheCurrentCorrectedVolumetricFlux();

  for (const auto * fi : flowFaceInfo())
    _vof_transport_phi[fi->id()] =
        use_previous_timestep_flux
            ? libmesh_map_find(_previous_timestep_corrected_face_phi, fi->id())
            : libmesh_map_find(_corrected_face_phi, fi->id());

  _vof_transport_phi_valid = true;

  dumpSharpFluxDiagnostic();
}

void
ConservativeSharpInterfaceRhieChowMassFluxBase::dumpSharpFluxDiagnostic()
{
  if (_sharp_flux_diagnostic_file_base.empty() || _sharp_flux_diagnostic_time < 0.0 ||
      _t + libMesh::TOLERANCE < _sharp_flux_diagnostic_time)
    return;

  const auto current_t_step = static_cast<unsigned int>(_fe_problem.timeStep());
  if (_last_sharp_flux_diagnostic_t_step == current_t_step)
    return;

  const auto time_arg = Moose::currentState();
  auto face_alpha = [this, &time_arg](const FaceInfo * const fi, const bool elem_side)
  {
    if (!_volume_fraction || !fi)
      return 0.0;

    const Elem * const elem = elem_side ? fi->elemPtr() : fi->neighborPtr();
    if (!elem || !hasBlocks(elem->subdomain_id()))
      return 0.0;

    return std::max(0.0,
                    std::min(1.0,
                             MetaPhysicL::raw_value(
                                 (*_volume_fraction)(Moose::ElemArg{elem, false}, time_arg))));
  };

  Real front_x = -std::numeric_limits<Real>::max();
  if (_volume_fraction)
    for (const auto * const fi : flowFaceInfo())
    {
      const Real elem_alpha = face_alpha(fi, true);
      const Real neighbor_alpha = face_alpha(fi, false);
      if ((elem_alpha - 0.5) * (neighbor_alpha - 0.5) <= 0.0 &&
          std::max(elem_alpha, neighbor_alpha) >= _near_interface_lower &&
          std::min(elem_alpha, neighbor_alpha) <= _near_interface_upper)
        front_x = std::max(front_x, fi->faceCentroid()(0));
    }

  if (front_x == -std::numeric_limits<Real>::max())
    for (const auto * const fi : flowFaceInfo())
      front_x = std::max(front_x, fi->faceCentroid()(0));

  const std::string file_name = _sharp_flux_diagnostic_file_base + ".csv";
  std::ofstream out(file_name);
  out << std::setprecision(17);
  out << "time,t_step,front_x,face_id,x,y,nx,ny,area,coord,elem_id,neighbor_id,elem_alpha,"
         "neighbor_alpha,hbya_phi,transient_phi,phig_phi,predictor_phi,pEqn_phi,recovery_phi,"
         "corrected_phi,vof_transport_phi,vof_rho_phi,vof_rho_phi_density,vof_alpha_phi,"
         "vof_reconstructed_phi,vof_rho_phi_contract_error,pressure_predictor_base_phi,"
         "internal_phiHbyA\n";

  for (const auto * const fi : flowFaceInfo())
  {
    if (!_sharp_flux_diagnostic_all_faces && fi->faceCentroid()(0) + libMesh::TOLERANCE <
                                                 front_x - _sharp_flux_diagnostic_front_band_width)
      continue;

    const Real elem_alpha = face_alpha(fi, true);
    const Real neighbor_alpha = face_alpha(fi, false);
    const Real face_measure = fi->faceArea() * fi->faceCoord();
    auto integratedToVolumetricPhi = [face_measure](const Real integrated_phi)
    { return face_measure > libMesh::TOLERANCE ? integrated_phi / face_measure : 0.0; };
    const Real hbya_phi = storedPredictorOperatorPhi(*fi);
    const Real transient_phi =
        integratedToVolumetricPhi(libmesh_map_find(_transient_projection_flux, fi->id()));
    const Real phig_phi = integratedToVolumetricPhi(libmesh_map_find(_phig_flux, fi->id()));
    const Real p_eqn_phi =
        _pressure_equation_flux_valid
            ? integratedToVolumetricPhi(libmesh_map_find(_pressure_equation_flux, fi->id()))
            : 0.0;
    const Real predictor_phi = hbya_phi + transient_phi + phig_phi;
    const Real recovery_phi = transient_phi + phig_phi + p_eqn_phi;
    const Real corrected_phi = libmesh_map_find(_corrected_face_phi, fi->id());
    const Real vof_transport_phi = libmesh_map_find(_vof_transport_phi, fi->id());
    const Real vof_rho_phi = vofRhoPhiIntegrated(*fi);
    const Real vof_rho_phi_density = face_measure > 0.0 ? vof_rho_phi / face_measure : 0.0;
    const Real vof_alpha_phi = vofAlphaPhiLimitedIntegrated(*fi);
    const Real vof_reconstructed_phi = vofRhoPhiReconstructedVolumetricPhi(*fi);
    Real vof_contract_error = 0.0;
    if (_liquid_density && _gas_density)
    {
      const Real gas_density = evaluateCellBasedFaceScalarFunctor(_gas_density, fi, time_arg);
      const Real liquid_density = evaluateCellBasedFaceScalarFunctor(_liquid_density, fi, time_arg);
      vof_contract_error = vof_rho_phi - (gas_density * vof_transport_phi * face_measure +
                                          (liquid_density - gas_density) * vof_alpha_phi);
    }
    const Real pressure_predictor_base_phi = hbya_phi + transient_phi;
    const Real internal_phi_hbya =
        integratedToVolumetricPhi(libmesh_map_find(_phiHbyA_flux, fi->id()));

    out << _t << ',' << current_t_step << ',' << front_x << ',' << fi->id() << ','
        << fi->faceCentroid()(0) << ',' << fi->faceCentroid()(1) << ',' << fi->normal()(0) << ','
        << fi->normal()(1) << ',' << fi->faceArea() << ',' << fi->faceCoord() << ','
        << fi->elemPtr()->id() << ','
        << (fi->neighborPtr() ? std::to_string(fi->neighborPtr()->id()) : std::string("")) << ','
        << elem_alpha << ',' << neighbor_alpha << ',' << hbya_phi << ',' << transient_phi << ','
        << phig_phi << ',' << predictor_phi << ',' << p_eqn_phi << ',' << recovery_phi << ','
        << corrected_phi << ',' << vof_transport_phi << ',' << vof_rho_phi << ','
        << vof_rho_phi_density << ',' << vof_alpha_phi << ',' << vof_reconstructed_phi << ','
        << vof_contract_error << ',' << pressure_predictor_base_phi << ',' << internal_phi_hbya
        << '\n';
  }

  _last_sharp_flux_diagnostic_t_step = current_t_step;
}

void
ConservativeSharpInterfaceRhieChowMassFluxBase::adoptPublishedVOFTransportState()
{
  if (!_vof_transport_phi_valid)
    freezeVOFTransportState(false);

  checkVOFRhoPhiContract();
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

  if (_require_vof_rho_phi_functor || _enforce_vof_rho_phi_contract)
  {
    if (!_vof_rho_phi)
      paramError("vof_rho_phi_functor",
                 "The conservative sharp-interface OpenFOAM-parity path requires the VOF-owned "
                 "rhoPhi functor.");
    if (!_vof_alpha_phi_limited)
      paramError("vof_alpha_phi_limited_functor",
                 "The conservative sharp-interface OpenFOAM-parity path requires the VOF-owned "
                 "limited alphaPhi functor.");
    if (!_liquid_density)
      paramError("liquid_density_functor",
                 "The VOF rhoPhi contract check requires the liquid density functor.");
    if (!_gas_density)
      paramError("gas_density_functor",
                 "The VOF rhoPhi contract check requires the gas density functor.");
  }
}

void
ConservativeSharpInterfaceRhieChowMassFluxBase::initialize()
{
  RhieChowMassFlux::initialize();

  initializeAdditionalPressureFluxStorage(/*preserve_corrected_face_phi=*/true);
  if (!_corrected_face_phi_seeded)
    cacheCurrentCorrectedVolumetricFlux();

  for (const auto * fi : _sharp_interface_face_info)
    _previous_timestep_corrected_face_phi[fi->id()] =
        libmesh_map_find(_corrected_face_phi, fi->id());
}

void
ConservativeSharpInterfaceRhieChowMassFluxBase::initFaceMassFlux()
{
  RhieChowMassFlux::initFaceMassFlux();
}

void
ConservativeSharpInterfaceRhieChowMassFluxBase::cachePressureEquationFlux()
{
  const_cast<MooseLinearVariableFVReal *>(_p)->computeCellGradients();

  for (auto & fi : flowFaceInfo())
    _pressure_equation_flux[fi->id()] = computeDiscretePressureFaceFlux(fi);

  _pressure_equation_flux_valid = true;

  const auto time_arg = Moose::currentState();
  const Real max_abs_normal_pressure_ainv = maxPressureFaceNormalAinv(time_arg);
  const Real degenerate_normal_pressure_ainv_tol =
      std::max(std::numeric_limits<Real>::min(),
               _pressure_writeback_face_ainv_relative_tolerance * max_abs_normal_pressure_ainv);
  cacheCurrentCorrectedVolumetricFlux(degenerate_normal_pressure_ainv_tol);
  _pressure_coupled_velocity_correction_valid = false;
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
  {
    // Keep the pressure-corrected face flux as the source of truth, matching
    // OpenFOAM's direct phi update. Do not reconstruct cell U and project it
    // back to faces here; that round trip is not an exact inverse.
    _face_mass_flux[fi->id()] = transportMassFluxDensityFromVolumetricPhi(
        fi, libmesh_map_find(_corrected_face_phi, fi->id()), time_arg);
  }
}

Moose::FaceArg
ConservativeSharpInterfaceRhieChowMassFluxBase::makeCenteredFaceArg(
    const FaceInfo * fi, const Moose::StateArg * limiter_state) const
{
  return Moose::FaceArg{
      fi, Moose::FV::LimiterType::CentralDifference, true, false, nullptr, limiter_state};
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::interpolateFaceDensity(
    const FaceInfo * fi, const Moose::StateArg & time_arg) const
{
  using namespace Moose::FV;

  if (_vel[0]->isInternalFace(*fi))
  {
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

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::predictorFaceDensity(
    const FaceInfo * fi, const Moose::StateArg & time_arg) const
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
    const auto & elem_info = *fi->elemInfo();
    const auto & neighbor_info = *fi->neighborInfo();
    Real face_velocity = 0.0;
    Moose::FV::interpolate(Moose::FV::InterpMethod::Average,
                           face_velocity,
                           cellPhysicalVelocityComponent(elem_info, component, time_arg),
                           cellPhysicalVelocityComponent(neighbor_info, component, time_arg),
                           *fi,
                           true);
    return face_velocity;
  }

  return boundaryPhysicalVelocityComponent(fi, component, time_arg);
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::interpolatedPhysicalFaceFlux(
    const FaceInfo * fi, const Moose::StateArg & time_arg) const
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

  bool have_cached_predictor_diagonal = true;
  for (const auto dim_i : make_range(_dim))
    have_cached_predictor_diagonal = have_cached_predictor_diagonal &&
                                     static_cast<bool>(cachedMomentumPredictorDiagonalRaw(dim_i));

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
    raw_ainv = *cachedMomentumPredictorDiagonalRaw(dim_i);

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
  state.negative_sn_grad_p = -(
      pressure_gradient_readers ? computeFaceNormalPressureGradient(fi, *pressure_gradient_readers)
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
    const auto elem_value =
        MetaPhysicL::raw_value((*functor)(makeElemArg(fi->elemPtr()), time_arg));
    const auto neighbor_value =
        MetaPhysicL::raw_value((*functor)(makeElemArg(fi->neighborPtr()), time_arg));
    RealVectorValue face_value;
    Moose::FV::interpolate(
        Moose::FV::InterpMethod::Average, face_value, elem_value, neighbor_value, *fi, true);
    return face_value;
  }

  const bool elem_is_fluid = hasBlocks(fi->elemPtr()->subdomain_id());
  const Elem * const fluid_elem = elem_is_fluid ? fi->elemPtr() : fi->neighborPtr();
  return MetaPhysicL::raw_value((*functor)(makeElemArg(fluid_elem), time_arg));
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::evaluateFaceScalarFunctor(
    const Moose::Functor<Real> * functor,
    const FaceInfo * fi,
    const Moose::StateArg & time_arg,
    const Moose::StateArg * limiter_state) const
{
  if (!functor)
    return 0.0;

  return MetaPhysicL::raw_value((*functor)(makeCenteredFaceArg(fi, limiter_state), time_arg));
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
    const Real elem_value =
        MetaPhysicL::raw_value((*functor)(makeElemArg(fi->elemPtr()), time_arg));
    const Real neighbor_value =
        MetaPhysicL::raw_value((*functor)(makeElemArg(fi->neighborPtr()), time_arg));
    Real face_value = 0.0;
    Moose::FV::interpolate(
        Moose::FV::InterpMethod::Average, face_value, elem_value, neighbor_value, *fi, true);
    return face_value;
  }

  const bool elem_is_fluid = hasBlocks(fi->elemPtr()->subdomain_id());
  const Elem * const fluid_elem = elem_is_fluid ? fi->elemPtr() : fi->neighborPtr();
  return MetaPhysicL::raw_value((*functor)(makeElemArg(fluid_elem), time_arg));
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::projectPhysicalMassFluxDensity(
    const Real face_rho,
    const RealVectorValue & face_ainv_raw,
    const RealVectorValue & face_acceleration,
    const RealVectorValue & face_normal) const
{
  Real projected_flux = 0.0;
  for (const auto dim_i : make_range(_dim))
    projected_flux +=
        face_rho * face_ainv_raw(dim_i) * face_acceleration(dim_i) * face_normal(dim_i);

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
    const FaceInfo * fi, const Moose::StateArg & time_arg, const SharpFaceOperatorState & state)
{
  if (!fi || time_arg.state != Moose::currentState().state)
    return 0.0;

  const Real dt = _fe_problem.dt();
  if (dt <= libMesh::TOLERANCE)
    return 0.0;

  if (!_vel[0]->isInternalFace(*fi))
    return 0.0;

  const Real old_corrected_phi = libmesh_map_find(_previous_timestep_corrected_face_phi, fi->id());
  const Real old_interpolated_face_phi = interpolatedPhysicalFaceFlux(fi, Moose::oldState());
  const Real phi_corr = old_corrected_phi - old_interpolated_face_phi;
  const Real phi_scale = std::abs(old_corrected_phi) + std::numeric_limits<Real>::epsilon();
  const Real ddt_coupling_coeff =
      std::max(0.0, 1.0 - std::min(std::abs(phi_corr) / phi_scale, 1.0));

  // OpenFOAM's Euler ddtCorr contribution is
  // fvcDdtPhiCoeff(U.oldTime(), phi.oldTime(), phiCorr) * phiCorr / dt, and
  // pressureCorrector adds interpolate(rho*rAU()) times that value to phiHbyA.
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
ConservativeSharpInterfaceRhieChowMassFluxBase::computeDiscretePressureFaceVolumetricFlux(
    const FaceInfo * fi) const
{
  if (!fi)
    return 0.0;

  PetscVectorReader p_reader(*_pressure_system->system().current_local_solution);

  _p_diffusion_kernel->setupFaceData(fi);
  const Real face_measure = fi->faceArea() * fi->faceCoord();
  _p_diffusion_kernel->setCurrentFaceArea(face_measure);

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
    const auto elem_rhs_contribution = _p_diffusion_kernel->computeElemRightHandSideContribution();

    // The sharp pressure projection stores the solved OpenFOAM-style pEqn.flux
    // including face area. MOOSE transport converts back to a normal flux
    // density when publishing corrected_face_phi.
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

      // Boundary pressure diffusion follows the same integrated-flux contract
      // as the interior pressure operator.
      return p_elem_value * matrix_contribution - rhs_contribution;
    }
  }

  return 0.0;
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::computeDiscretePressureFaceFlux(
    const FaceInfo * fi) const
{
  return computeDiscretePressureFaceVolumetricFlux(fi);
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::computeFaceNormalDensityGradient(
    const FaceInfo * fi, const Moose::StateArg & time_arg) const
{
  if (!fi)
    return 0.0;

  if (_vel[0]->isInternalFace(*fi))
  {
    const Real elem_rho = _rho(makeElemArg(fi->elemPtr()), time_arg);
    const Real neighbor_rho = _rho(makeElemArg(fi->neighborPtr()), time_arg);
    const Point delta = fi->dCN();
    const Real delta_mag = delta.norm();

    if (delta_mag <= libMesh::TOLERANCE)
      return 0.0;

    const Real rho_jump = neighbor_rho - elem_rho;
    const Real orthogonal_part = rho_jump / delta_mag;

    const Real non_orth_delta_coeff = 1.0 / std::max(fi->normal() * delta, 0.05 * delta_mag);
    const Real base_part = non_orth_delta_coeff * rho_jump;

    const Point non_orth_correction_vector = fi->normal() - delta * non_orth_delta_coeff;
    const RealVectorValue elem_grad_rho =
        MetaPhysicL::raw_value(_rho.gradient(makeElemArg(fi->elemPtr()), time_arg));
    const RealVectorValue neighbor_grad_rho =
        MetaPhysicL::raw_value(_rho.gradient(makeElemArg(fi->neighborPtr()), time_arg));

    RealVectorValue face_grad_rho = 0;
    Moose::FV::interpolate(Moose::FV::InterpMethod::Average,
                           face_grad_rho,
                           elem_grad_rho,
                           neighbor_grad_rho,
                           *fi,
                           true);
    const Real correction_part = non_orth_correction_vector * face_grad_rho;

    if (_density_sn_grad_scheme == "orthogonal")
      return orthogonal_part;

    if (_density_sn_grad_scheme == "corrected")
      return base_part + correction_part;

    const Real correction_magnitude = std::abs(correction_part);
    const Real base_magnitude = std::abs(base_part);
    const Real limiter =
        std::min(_density_sn_grad_limiter_coefficient * base_magnitude /
                     ((1.0 - _density_sn_grad_limiter_coefficient) * correction_magnitude +
                      libMesh::TOLERANCE),
                 1.0);
    return base_part + limiter * correction_part;
  }

  const bool elem_is_fluid = hasBlocks(fi->elemPtr()->subdomain_id());
  const Elem * const fluid_elem = elem_is_fluid ? fi->elemPtr() : fi->neighborPtr();
  const Moose::FaceArg boundary_face{
      fi, Moose::FV::LimiterType::CentralDifference, true, false, fluid_elem, nullptr};

  // On uncoupled boundary patches reference solver's explicit non-orthogonal correction
  // is zero and the operative snGrad comes from the boundary patch field.
  return MetaPhysicL::raw_value(_rho.gradient(boundary_face, time_arg)) * fi->normal();
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

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::
    computeDiscreteHydrostaticPredictorFaceNormalForceDensity(
        const FaceInfo * fi, const Moose::StateArg & time_arg) const
{
  if (!fi || !useDiscreteHydrostaticPredictorFaceForce())
    return 0.0;

  const Real ghf = _gravity * (fi->faceCentroid() - _reference_pressure_point);
  return -ghf * computeFaceNormalDensityGradient(fi, time_arg);
}

void
ConservativeSharpInterfaceRhieChowMassFluxBase::updateAdditionalPressureFluxFunctors(
    const bool with_updated_pressure, const bool verbose)
{
  _pressure_coupled_velocity_correction_valid = false;
  _pressure_predictor_face_state_valid = false;

  const auto time_arg = Moose::currentState();

  std::vector<std::unique_ptr<NumericVector<Number>>> owned_raw_ainv;
  std::vector<PetscVectorReader> raw_ainv_readers;
  buildSharpFaceRawAinvReaders(owned_raw_ainv, raw_ainv_readers);
  std::vector<PetscVectorReader> pressure_gradient_readers;
  buildSelectedPressureGradientReaders(with_updated_pressure, pressure_gradient_readers);

  std::vector<PetscVectorReader> hbya_readers;
  hbya_readers.reserve(_HbyA_raw.size());
  for (const auto & hbya_raw : _HbyA_raw)
    hbya_readers.emplace_back(*hbya_raw);

  for (const auto * fi : _sharp_interface_face_info)
  {
    RealVectorValue face_hbya;

    if (_vel[0]->isInternalFace(*fi))
    {
      const auto & elem_info = *fi->elemInfo();
      const auto & neighbor_info = *fi->neighborInfo();

      for (const auto dim_i : index_range(_vel))
      {
        const auto elem_dof = elem_info.dofIndices()[_global_momentum_system_numbers[dim_i]][0];
        const auto neighbor_dof =
            neighbor_info.dofIndices()[_global_momentum_system_numbers[dim_i]][0];

        interpolate(Moose::FV::InterpMethod::Average,
                    face_hbya(dim_i),
                    -hbya_readers[dim_i](elem_dof),
                    -hbya_readers[dim_i](neighbor_dof),
                    *fi,
                    true);
      }
    }
    else
    {
      const bool elem_is_fluid = hasBlocks(fi->elemPtr()->subdomain_id());
      const Real boundary_normal_multiplier = elem_is_fluid ? 1.0 : -1.0;
      const ElemInfo & elem_info = elem_is_fluid ? *fi->elemInfo() : *fi->neighborInfo();
      const Moose::FaceArg boundary_face{
          fi, Moose::FV::LimiterType::CentralDifference, true, false, elem_info.elem(), nullptr};

      const bool use_constrained_boundary_state = useConstrainedBoundaryPredictorState(fi);

      if (use_constrained_boundary_state)
      {
        for (const auto dim_i : make_range(_dim))
        {
          const Real boundary_value = boundaryPhysicalVelocityComponent(fi, dim_i, time_arg);
          face_hbya(dim_i) =
              std::isfinite(boundary_value)
                  ? -boundary_value
                  : -MetaPhysicL::raw_value((*_vel[dim_i])(boundary_face, Moose::currentState()));
          face_hbya(dim_i) *= boundary_normal_multiplier;
        }
      }
      else
      {
        for (const auto dim_i : index_range(_vel))
        {
          const auto elem_dof = elem_info.dofIndices()[_global_momentum_system_numbers[dim_i]][0];
          face_hbya(dim_i) = -boundary_normal_multiplier * hbya_readers[dim_i](elem_dof);
        }
      }
    }

    const Real face_measure = fi->faceArea() * fi->faceCoord();
    const Real predictor_operator_volumetric_flux = face_hbya * fi->normal();
    const auto state =
        buildSharpFaceOperatorState(fi, time_arg, raw_ainv_readers, &pressure_gradient_readers);
    const RealVectorValue pressure_face_raw_ainv = state.face_raw_ainv;
    const Real normal_pressure_ainv =
        computeFaceNormalRawAinv(pressure_face_raw_ainv, state.face_normal);
    Real transient_projection_flux = 0.0;
    Real surface_tension_flux = 0.0;
    Real hydrostatic_flux = 0.0;

    if (_add_transient_projection_flux && !_suppress_startup_pressure_predictor_flux_sources)
    {
      if (_transient_projection_face_acceleration)
      {
        const RealVectorValue transient_acceleration = evaluateBoundaryAwareVectorFunctor(
            _transient_projection_face_acceleration, fi, time_arg);
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
        surface_tension_flux = massFluxDensityToVolumetricNormalFlux(fi, surface_mass_flux_density);
      }

      if (!_suppress_explicit_hydrostatic_pressure_flux)
      {
        const Real ghf = _gravity * (fi->faceCentroid() - _reference_pressure_point);
        const Real sn_grad_rho = computeFaceNormalDensityGradient(fi, time_arg);
        hydrostatic_flux = -ghf * sn_grad_rho * normal_pressure_ainv;
      }
    }

    const Real phig_flux = (surface_tension_flux + hydrostatic_flux) * face_measure;
    const Real transient_projection_integrated_flux = transient_projection_flux * face_measure;
    const Real pressure_predictor_base_flux =
        (predictor_operator_volumetric_flux + transient_projection_flux) * face_measure;

    _transient_projection_flux[fi->id()] = transient_projection_integrated_flux;
    _capillary_hydrostatic_flux[fi->id()] = phig_flux;
    _phig_flux[fi->id()] = phig_flux;
    _pressure_Ainv[fi->id()] = pressure_face_raw_ainv;
    _pressure_predictor_base_flux[fi->id()] = -pressure_predictor_base_flux;
    _phiHbyA_flux[fi->id()] = -(pressure_predictor_base_flux + phig_flux);
    _pressure_predictor_flux[fi->id()] = _phiHbyA_flux[fi->id()];
    _pressure_predictor_mass_flux[fi->id()] =
        face_measure > libMesh::TOLERANCE
            ? transportMassFluxDensityFromVolumetricPhi(
                  fi, (pressure_predictor_base_flux + phig_flux) / face_measure, time_arg)
            : 0.0;

    if (verbose)
    {
      _console << "Sharp-interface velocity predictor on face " << fi->id()
               << ": HbyA_flux=" << predictor_operator_volumetric_flux
               << ", transient_source_flux=" << _transient_projection_flux[fi->id()]
               << ", capillary_hydrostatic_source_flux=" << _capillary_hydrostatic_flux[fi->id()]
               << ", phig_flux=" << _phig_flux[fi->id()]
               << ", pressure_predictor_base_flux=" << pressure_predictor_base_flux
               << ", internal_pressure_predictor_base_flux="
               << _pressure_predictor_base_flux[fi->id()]
               << ", phiHbyA_flux=" << _phiHbyA_flux[fi->id()] << std::endl;
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
      _pressure_coupled_cell_reconstruction_scalar[fi->id()] = 0.0;

    _pressure_coupled_velocity_correction_valid = true;
    return;
  }

  if (!_pressure_equation_flux_valid)
    cachePressureEquationFlux();

  // The tolerance is documented as a fraction of the active-face maximum.
  // Using max(1, max_abs_normal_pressure_ainv) turns it into an unintended
  // absolute cutoff whenever the live face-normal Ainv values are << 1, which
  // suppresses writeback reconstruction on the very interface faces we need to
  // correct. Keep the cutoff relative to the active maximum instead.
  const Real max_abs_normal_pressure_ainv = maxPressureFaceNormalAinv(time_arg);
  const Real degenerate_normal_pressure_ainv_tol =
      std::max(std::numeric_limits<Real>::min(),
               _pressure_writeback_face_ainv_relative_tolerance * max_abs_normal_pressure_ainv);

  cacheCurrentCorrectedVolumetricFlux(degenerate_normal_pressure_ainv_tol);

  _pressure_coupled_velocity_correction_valid = true;
}

bool
ConservativeSharpInterfaceRhieChowMassFluxBase::useConstrainedBoundaryPredictorState(
    const FaceInfo * fi) const
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
      {
        if (dynamic_cast<LinearFVPressureInletOutletMomentumBC *>(bc_pointer) ||
            dynamic_cast<LinearFVPressureInletOutletVelocityBC *>(bc_pointer))
          continue;

        if (auto * adv_diff_bc = dynamic_cast<LinearFVAdvectionDiffusionBC *>(bc_pointer))
        {
          adv_diff_bc->setupFaceData(
              fi, fi->faceType(std::make_pair(_vel[dim_i]->number(), _vel[dim_i]->sys().number())));
          if (adv_diff_bc->computeBoundaryGradientMatrixContribution() > 0.0)
          {
            use_constrained_boundary_state = true;
            break;
          }
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

RealVectorValue
ConservativeSharpInterfaceRhieChowMassFluxBase::reconstructFaceNormalScalarToCellVector(
    const ElemInfo * elem_info,
    const Moose::StateArg & time_arg,
    const FaceScalarField & scalar_field) const
{
  if (!elem_info)
    return RealVectorValue();

  const Elem * const elem = elem_info->elem();
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
    const FaceInfo * const fi_loc =
        _moose_mesh.faceInfo(elem_has_fi ? elem : loc_neighbor,
                             elem_has_fi ? side : loc_neighbor->which_neighbor_am_i(elem));
    if (!fi_loc)
      continue;

    const Real face_measure = fi_loc->faceArea() * fi_loc->faceCoord();
    if (face_measure <= 0.0)
      continue;

    // reference solver fvc::reconstruct uses
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

  bool singular = false;
  if (_dim == 1)
    singular = std::abs(normal_matrix(0, 0)) <= libMesh::TOLERANCE;
  else if (_dim == 2)
    singular = std::abs(normal_matrix(0, 0) * normal_matrix(1, 1) -
                        normal_matrix(0, 1) * normal_matrix(1, 0)) <= libMesh::TOLERANCE;
  else if (_dim == 3)
  {
    const Real det = normal_matrix(0, 0) * (normal_matrix(1, 1) * normal_matrix(2, 2) -
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

RealVectorValue
ConservativeSharpInterfaceRhieChowMassFluxBase::
    reconstructReferenceStylePressureCoupledCellVelocityDelta(
        const ElemInfo * elem_info, const Moose::StateArg & time_arg) const
{
  if (!elem_info || !_pressure_coupled_velocity_correction_valid)
    return RealVectorValue();

  const RealVectorValue reconstructed_operator = reconstructFaceNormalScalarToCellVector(
      elem_info, time_arg, _pressure_coupled_cell_reconstruction_scalar);

  RealVectorValue pressure_delta;
  for (const auto system_i : index_range(_momentum_implicit_systems))
  {
    const auto & dof_indices = elem_info->dofIndices()[_global_momentum_system_numbers[system_i]];
    if (dof_indices.empty())
      continue;

    pressure_delta(system_i) =
        cellAinvRaw(system_i, dof_indices[0]) * reconstructed_operator(system_i);
  }

  return pressure_delta;
}

RealVectorValue
ConservativeSharpInterfaceRhieChowMassFluxBase::reconstructPressureCoupledCellVelocityDelta(
    const ElemInfo * elem_info, const Moose::StateArg & time_arg) const
{
  if (!elem_info || !_pressure_coupled_velocity_correction_valid ||
      !_apply_pressure_velocity_writeback)
    return RealVectorValue();

  return reconstructReferenceStylePressureCoupledCellVelocityDelta(elem_info, time_arg);
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
ConservativeSharpInterfaceRhieChowMassFluxBase::computePressureCorrectedCellVelocity()
{
  const auto time_arg = Moose::currentState();
  updatePressureCoupledVelocityCorrectionFaceField(time_arg);
  writePressureCorrectedVelocityToMomentumSolution(time_arg);
  _velocity_boundary_state_valid = false;
}

void
ConservativeSharpInterfaceRhieChowMassFluxBase::computeCellVelocity()
{
  computePressureCorrectedCellVelocity();
}
