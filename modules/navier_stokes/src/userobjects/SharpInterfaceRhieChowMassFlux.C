#include "SharpInterfaceRhieChowMassFlux.h"

#include "LinearFVPressureInletOutletVelocityBC.h"
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

registerMooseObject("NavierStokesApp", SharpInterfaceRhieChowMassFlux);

InputParameters
SharpInterfaceRhieChowMassFlux::validParams()
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
      "use_face_based_predictor_body_force",
      true,
      "Whether to reconstruct the reduced-pressure momentum-predictor explicit forcing from "
      "face-based sharp-interface operators instead of the legacy cell-gradient / cell-"
      "acceleration path.");

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
      "contribution, expressed in acceleration-like units.");
  params.addParam<MooseFunctorName>(
      "hydrostatic_density_gradient_cell_acceleration",
      "",
      "Optional cell-vector functor containing the reduced-pressure hydrostatic density-gradient "
      "contribution, expressed in acceleration-like units.");
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

SharpInterfaceRhieChowMassFlux::SharpInterfaceRhieChowMassFlux(const InputParameters & params)
  : RhieChowMassFlux(params),
    _transient_projection_flux(_moose_mesh, blockIDs(), "transient_projection_flux"),
    _capillary_hydrostatic_flux(_moose_mesh, blockIDs(), "capillary_hydrostatic_flux"),
    _pressure_Ainv(_moose_mesh, blockIDs(), "sharp_pressure_Ainv"),
    _predictor_convective_mass_flux(_moose_mesh, blockIDs(), "predictor_convective_mass_flux"),
    _predictor_convective_phi(_moose_mesh, blockIDs(), "predictor_convective_phi"),
    _predictor_operator_phi(_moose_mesh, blockIDs(), "predictor_operator_phi"),
    _pressure_predictor_base_phi(_moose_mesh, blockIDs(), "pressure_predictor_base_phi"),
    _pressure_equation_volumetric_flux(_moose_mesh, blockIDs(), "pressure_equation_volumetric_flux"),
    _pressure_correction_phi(_moose_mesh, blockIDs(), "pressure_correction_phi"),
    _sharp_pressure_predictor_flux(_moose_mesh, blockIDs(), "sharp_pressure_predictor_flux"),
    _reference_face_mass_flux_for_writeback(
        _moose_mesh, blockIDs(), "reference_face_mass_flux_for_writeback"),
    _corrected_face_phi(_moose_mesh, blockIDs(), "corrected_face_phi"),
    _outer_iteration_rho_phi(_moose_mesh, blockIDs(), "outer_iteration_rho_phi"),
    _outer_iteration_phi(_moose_mesh, blockIDs(), "outer_iteration_phi"),
    _debug_update_hydrostatic_face_mass_flux_density_raw(
        _moose_mesh, blockIDs(), "debug_update_hydrostatic_face_mass_flux_density_raw"),
    _debug_update_physical_capillary_hydrostatic_flux(
        _moose_mesh, blockIDs(), "debug_update_physical_capillary_hydrostatic_flux"),
    _debug_update_hydrostatic_branch_taken(
        _moose_mesh, blockIDs(), "debug_update_hydrostatic_branch_taken"),
    _pressure_coupled_velocity_correction_scalar(
        _moose_mesh, blockIDs(), "pressure_coupled_velocity_correction_scalar"),
    _pressure_coupled_velocity_correction_face(
        _moose_mesh, blockIDs(), "pressure_coupled_velocity_correction_face"),
    _corrected_face_velocity(_moose_mesh, blockIDs(), "corrected_face_velocity"),
    _add_transient_projection_flux(getParam<bool>("add_transient_projection_flux")),
    _add_capillary_hydrostatic_flux(getParam<bool>("add_capillary_hydrostatic_flux")),
    _use_face_based_predictor_body_force(getParam<bool>("use_face_based_predictor_body_force")),
    _gravity(getParam<RealVectorValue>("gravity")),
    _reference_pressure_point(getParam<Point>("reference_pressure_point")),
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
    _vof_rho_phi(nullptr),
    _vof_alpha_phi_limited(nullptr),
    _liquid_density(nullptr),
    _gas_density(nullptr)
{
  for (const auto tid : make_range(libMesh::n_threads()))
  {
    UserObject::_subproblem.addFunctor("pressure_predictor_flux", _phiHbyA_flux, tid);
    UserObject::_subproblem.addFunctor("transient_projection_flux", _transient_projection_flux, tid);
    UserObject::_subproblem.addFunctor(
        "capillary_hydrostatic_flux", _capillary_hydrostatic_flux, tid);
    UserObject::_subproblem.addFunctor("sharp_pressure_Ainv", _pressure_Ainv, tid);
    UserObject::_subproblem.addFunctor(
        "predictor_convective_mass_flux", _predictor_convective_mass_flux, tid);
    UserObject::_subproblem.addFunctor("predictor_convective_phi", _predictor_convective_phi, tid);
    UserObject::_subproblem.addFunctor("predictor_operator_phi", _predictor_operator_phi, tid);
    UserObject::_subproblem.addFunctor(
        "pressure_predictor_base_phi", _pressure_predictor_base_phi, tid);
    UserObject::_subproblem.addFunctor(
        "pressure_equation_volumetric_flux", _pressure_equation_volumetric_flux, tid);
    UserObject::_subproblem.addFunctor("pressure_correction_phi", _pressure_correction_phi, tid);
    UserObject::_subproblem.addFunctor(
        "sharp_pressure_predictor_flux", _sharp_pressure_predictor_flux, tid);
    UserObject::_subproblem.addFunctor("corrected_face_phi", _corrected_face_phi, tid);
    UserObject::_subproblem.addFunctor("pressure_coupled_velocity_correction_scalar",
                                       _pressure_coupled_velocity_correction_scalar,
                                       tid);
  }

  if (!dynamic_cast<SIMPLE *>(getMooseApp().getExecutioner()) &&
      !dynamic_cast<PIMPLE *>(getMooseApp().getExecutioner()))
    mooseError(this->name(),
               " should only be used with a linear segregated thermal-hydraulics solver!");

  if (splitMomentumPredictorOperator() &&
      !dynamic_cast<ReducedPressurePIMPLE *>(getMooseApp().getExecutioner()))
    mooseError(this->name(),
               ": split_momentum_predictor_operator is only supported with the "
               "ReducedPressurePIMPLE executioner, because that executioner is the one that "
               "adds the removed pressure/capillary momentum forcing back into the predictor. "
               "Use ReducedPressurePIMPLE or set split_momentum_predictor_operator = false.");

  rebuildSharpInterfaceFaceInfo();
  initializeAdditionalPressureFluxStorage();
}

Real
SharpInterfaceRhieChowMassFlux::getMassFlux(const FaceInfo & fi) const
{
  const Real face_measure = fi.faceArea() * fi.faceCoord();
  const auto normalize_vof_rho_phi = [face_measure](const Real rho_phi)
  {
    return face_measure > libMesh::TOLERANCE ? rho_phi / face_measure : 0.0;
  };

  if (_use_vof_rho_phi)
  {
    if (_outer_iteration_convective_state_valid)
      return normalize_vof_rho_phi(libmesh_map_find(_outer_iteration_rho_phi, fi.id()));
    if (_vof_rho_phi)
      return normalize_vof_rho_phi(
          evaluateFaceScalarFunctor(_vof_rho_phi, &fi, Moose::currentState(), nullptr));
  }

  return RhieChowMassFlux::getMassFlux(fi);
}

Real
SharpInterfaceRhieChowMassFlux::rawRhieChowMassFlux(const FaceInfo & fi) const
{
  return RhieChowMassFlux::getMassFlux(fi);
}

Real
SharpInterfaceRhieChowMassFlux::predictorOperatorFaceMassFlux(const FaceInfo & fi,
                                                              const Moose::StateArg & time_arg) const
{
  return transportMassFluxDensityFromVolumetricPhi(
      &fi, referenceFaceVelocityState(&fi, time_arg) * fi.normal(), time_arg);
}

Real
SharpInterfaceRhieChowMassFlux::pressureCoupledWritebackMassFlux(const FaceInfo & fi) const
{
  return pressureVelocityWritebackFluxDensity(&fi);
}

Real
SharpInterfaceRhieChowMassFlux::storedPredictorOperatorPhi(const FaceInfo & fi) const
{
  return libmesh_map_find(_predictor_operator_phi, fi.id());
}

Real
SharpInterfaceRhieChowMassFlux::storedPressureCorrectionPhi(const FaceInfo & fi) const
{
  return libmesh_map_find(_pressure_correction_phi, fi.id());
}

Real
SharpInterfaceRhieChowMassFlux::vofRhoPhiMassFlux(const FaceInfo & fi) const
{
  if (!_vof_rho_phi)
    return 0.0;

  const Real face_measure = fi.faceArea() * fi.faceCoord();
  if (face_measure <= libMesh::TOLERANCE)
    return 0.0;

  return evaluateFaceScalarFunctor(_vof_rho_phi, &fi, Moose::currentState(), nullptr) /
         face_measure;
}

void
SharpInterfaceRhieChowMassFlux::dumpPressureCorrectorFaceDebugCSV(const std::string & path)
{
  if (!_pressure_equation_flux_valid)
    cachePressureEquationFlux();

  const auto time_arg = Moose::currentState();
  std::unique_ptr<FaceVectorField> predictor_body_force_face;
  std::unique_ptr<FaceVectorField> predictor_pressure_force_face;
  std::unique_ptr<FaceScalarField> predictor_body_force_scalar;
  std::unique_ptr<FaceScalarField> predictor_pressure_force_scalar;
  bool have_face_based_predictor_pressure = false;
  if (_use_face_based_predictor_body_force)
  {
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
    have_face_based_predictor_pressure =
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
  }

  std::ofstream out(path);
  if (!out)
    mooseError("Failed to open sharp-interface pressure debug CSV: ", path);

  out << std::setprecision(17);
  out << "face_id,x,y,z,is_boundary,has_hydrostatic_face_accel,"
         "suppress_startup_pressure_predictor_flux_sources,"
         "suppress_explicit_hydrostatic_pressure_flux,"
         "normal_x,normal_y,normal_z,"
         "vof_alpha_phi_limited,vof_rho_phi_integrated,"
         "outer_iteration_phi,outer_iteration_rho_phi_integrated,"
         "predictor_convective_phi,predictor_convective_mass_flux,"
         "update_hydrostatic_branch_taken,"
         "pressure_predictor_base_phi,predictor_operator_phi,transient_projection_flux,"
         "capillary_hydrostatic_flux,phig_flux,"
         "pressure_equation_flux,pressure_correction_phi,corrected_face_phi,"
         "raw_rc_mass_flux,predictor_operator_mass_flux,pressure_writeback_mass_flux,"
         "reconstructed_pressure_writeback_mass_flux,pressure_writeback_mass_flux_mismatch,"
         "elem_delta_u,elem_delta_v,elem_delta_w,"
         "neighbor_delta_u,neighbor_delta_v,neighbor_delta_w,"
         "face_delta_u,face_delta_v,face_delta_w,"
         "predictor_face_density,normal_ainv,normal_raw_ainv,"
         "mass_flux_density_to_volumetric_scale,"
         "update_hydrostatic_face_mass_flux_density_raw,"
         "update_physical_capillary_hydrostatic_flux,"
         "hydrostatic_face_mass_flux_density_raw,"
         "hydrostatic_face_flux_volumetric_raw,negative_sn_grad_p,"
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
    const auto & face_ainv = libmesh_map_find(_pressure_Ainv, fi->id());
    const RealVectorValue face_raw_ainv = interpolateFaceRawAinv(fi);

    Real normal_ainv = 0.0;
    for (const auto dim_i : make_range(_dim))
      normal_ainv += face_ainv(dim_i) * face_normal(dim_i) * face_normal(dim_i);
    const Real normal_raw_ainv = computeFaceNormalRawAinv(face_raw_ainv, face_normal);
    const Real mass_flux_density_to_volumetric_scale =
        massFluxDensityToVolumetricNormalFlux(fi, 1.0);

    Real negative_sn_grad_p = 0.0;
    if (is_boundary && _pressure_boundary_normal_gradient_valid)
      negative_sn_grad_p = -libmesh_map_find(_pressure_boundary_normal_gradient, fi->id());
    else if (std::abs(normal_ainv) > libMesh::TOLERANCE)
      negative_sn_grad_p = libmesh_map_find(_pressure_equation_flux, fi->id()) / normal_ainv;

    const RealVectorValue predictor_pressure_force = negative_sn_grad_p * face_normal;
    const Real predictor_pressure_force_scalar_value =
        have_face_based_predictor_pressure && predictor_pressure_force_scalar
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
    const Real hydrostatic_face_mass_flux_density_raw =
        computeHydrostaticFaceMassFlux(fi,
                                       predictorFaceDensity(fi, time_arg),
                                       face_raw_ainv,
                                       face_normal,
                                       time_arg);
    RealVectorValue elem_delta;
    RealVectorValue neighbor_delta;
    RealVectorValue face_delta;
    Real reconstructed_pressure_writeback_mass_flux = 0.0;
    Real pressure_writeback_mass_flux_mismatch = 0.0;
    if (_pressure_coupled_velocity_correction_valid && _vel[0]->isInternalFace(*fi))
    {
      elem_delta = pressureCoupledCellVelocityDelta(*fi->elemInfo(), time_arg);
      neighbor_delta = pressureCoupledCellVelocityDelta(*fi->neighborInfo(), time_arg);
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
        << (is_boundary ? 1 : 0) << ','
        << (_hydrostatic_density_gradient_face_acceleration ? 1 : 0) << ','
        << (_suppress_startup_pressure_predictor_flux_sources ? 1 : 0) << ','
        << (_suppress_explicit_hydrostatic_pressure_flux ? 1 : 0) << ',' << face_normal(0) << ','
        << face_normal(1) << ',' << face_normal(2) << ','
        << vof_alpha_phi_limited << ',' << vof_rho_phi_integrated << ','
        << libmesh_map_find(_outer_iteration_phi, fi->id()) << ','
        << libmesh_map_find(_outer_iteration_rho_phi, fi->id()) << ','
        << libmesh_map_find(_predictor_convective_phi, fi->id()) << ','
        << libmesh_map_find(_predictor_convective_mass_flux, fi->id()) << ','
        << libmesh_map_find(_debug_update_hydrostatic_branch_taken, fi->id()) << ','
        << libmesh_map_find(_pressure_predictor_base_phi, fi->id()) << ','
        << libmesh_map_find(_predictor_operator_phi, fi->id()) << ','
        << libmesh_map_find(_transient_projection_flux, fi->id()) << ','
        << libmesh_map_find(_capillary_hydrostatic_flux, fi->id()) << ','
        << libmesh_map_find(_phig_flux, fi->id()) << ',' << pressure_equation_flux << ','
        << (pressure_equation_flux - libmesh_map_find(_phig_flux, fi->id())) << ','
        << libmesh_map_find(_corrected_face_phi, fi->id()) << ','
        << rawRhieChowMassFlux(*fi) << ','
        << predictorOperatorFaceMassFlux(*fi, time_arg) << ','
        << pressureCoupledWritebackMassFlux(*fi) << ','
        << reconstructed_pressure_writeback_mass_flux << ','
        << pressure_writeback_mass_flux_mismatch << ','
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
SharpInterfaceRhieChowMassFlux::pressureCoupledCellVelocityDelta(
    const ElemInfo & elem_info, const Moose::StateArg & time_arg) const
{
  if (!_pressure_coupled_velocity_correction_valid)
    const_cast<SharpInterfaceRhieChowMassFlux *>(this)
        ->updatePressureCoupledVelocityCorrectionFaceField(time_arg);
  if (!_corrected_face_velocity_valid)
    const_cast<SharpInterfaceRhieChowMassFlux *>(this)->updateCorrectedFaceVelocityField(time_arg);

  return reconstructPressureCoupledCellVelocityDelta(&elem_info, time_arg);
}

SharpInterfaceRhieChowMassFlux::PressureCorrectionReconstructionDebug
SharpInterfaceRhieChowMassFlux::pressureCorrectionReconstructionDebug(
    const ElemInfo & elem_info, const Moose::StateArg & time_arg) const
{
  PressureCorrectionReconstructionDebug debug;

  if (!_pressure_coupled_velocity_correction_valid)
    return debug;
  if (!_corrected_face_velocity_valid)
    const_cast<SharpInterfaceRhieChowMassFlux *>(this)->updateCorrectedFaceVelocityField(time_arg);

  const Elem * const elem = elem_info.elem();
  if (!elem)
    return debug;
  for (const auto side : make_range(elem->n_sides()))
  {
    const Elem * const loc_neighbor = elem->neighbor_ptr(side);
    const bool elem_has_fi = Moose::FV::elemHasFaceInfo(*elem, loc_neighbor);
    const FaceInfo * const fi_loc =
        _moose_mesh.faceInfo(elem_has_fi ? elem : loc_neighbor,
                             elem_has_fi ? side : loc_neighbor->which_neighbor_am_i(elem));
    if (fi_loc)
      ++debug.contributing_faces;
  }

  const auto elem_arg = makeElemArg(elem);
  const RealVectorValue corrected_cell_velocity =
      MetaPhysicL::raw_value(_corrected_face_velocity(elem_arg, time_arg));
  for (const auto dim_i : make_range(_dim))
  {
    const Real predictor_velocity = predictorVelocityComponent(elem_info, dim_i);
    debug.solution[dim_i] = corrected_cell_velocity(dim_i);
    debug.delta_velocity[dim_i] = corrected_cell_velocity(dim_i) - predictor_velocity;
  }

  return debug;
}

SharpInterfaceRhieChowMassFlux::MomentumPredictorExplicitForceDebug
SharpInterfaceRhieChowMassFlux::momentumPredictorExplicitForceDebug(
    const ElemInfo & elem_info, const Moose::StateArg & time_arg)
{
  MomentumPredictorExplicitForceDebug debug;

  if (!splitMomentumPredictorOperator() || !hasBlocks(elem_info.subdomain_id()))
    return debug;

  std::unique_ptr<FaceVectorField> predictor_body_force_face;
  bool have_face_based_predictor_body = false;
  if (_use_face_based_predictor_body_force)
  {
    predictor_body_force_face =
        std::make_unique<FaceVectorField>(_moose_mesh,
                                          blockIDs(),
                                          "momentum_predictor_body_force_face_debug");
    have_face_based_predictor_body =
        populateMomentumPredictorBodyForceFaceField(*predictor_body_force_face, time_arg);
  }

  const Real cell_volume = elem_info.volume() * elem_info.coordFactor();

  const RealVectorValue pressure_force_density =
      evaluateCellMomentumPredictorPressureForceDensity(elem_info);
  RealVectorValue body_force_density;
  RealVectorValue cell_body_force_density;
  debug.face_based_pressure = false;
  if (_add_capillary_hydrostatic_flux)
    body_force_density =
        (_use_face_based_predictor_body_force && have_face_based_predictor_body)
            ? reconstructFaceVectorFieldToCellSourceDensity(
                  &elem_info, time_arg, *predictor_body_force_face)
            : evaluateLegacyMomentumPredictorBodyForceDensity(&elem_info, time_arg);

  if (_add_capillary_hydrostatic_flux)
    cell_body_force_density = evaluateLegacyMomentumPredictorBodyForceDensity(&elem_info, time_arg);

  const auto cv_pressure_force_density = RealVectorValue();
  const auto cv_body_force_density =
      (_use_face_based_predictor_body_force && have_face_based_predictor_body)
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

Real
SharpInterfaceRhieChowMassFlux::predictorVelocityComponent(const ElemInfo & elem_info,
                                                           const unsigned int component) const
{
  mooseAssert(component < _dim, "Momentum component index out of range.");

  const auto dof = elem_info.dofIndices()[_global_momentum_system_numbers[component]][0];
  return -(*_HbyA_raw[component])(dof);
}

void
SharpInterfaceRhieChowMassFlux::addMomentumPredictorExplicitForcing(
    const unsigned int system_i, NumericVector<Number> & rhs) const
{
  if (!splitMomentumPredictorOperator())
    return;

  mooseAssert(system_i < _dim, "Momentum component index out of range in explicit forcing hook.");
  // Option A / face-authoritative sharp reduced-pressure coupling:
  // pressure and capillary/hydrostatic predictor branches are carried only in
  // the face predictor flux, pressure boundary constraint, and corrected face
  // velocity state. Do not reapply them as independent cell-RHS forcing here.
  rhs.close();
}

void
SharpInterfaceRhieChowMassFlux::addMomentumPredictorBodyForceForcing(
    const unsigned int system_i, NumericVector<Number> & rhs) const
{
  if (!splitMomentumPredictorOperator())
    return;

  mooseAssert(system_i < _dim, "Momentum component index out of range in body-force forcing hook.");
  // Sharp reduced-pressure body forcing is face-authoritative in the same path
  // as pressure prediction/correction, so there is no extra cell-based body
  // source to add here.
  rhs.close();
}

bool
SharpInterfaceRhieChowMassFlux::seedHydrostaticPressure(LinearSystem & pressure_system,
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
SharpInterfaceRhieChowMassFlux::getVolumetricFaceFlux(const FaceInfo & fi) const
{
  if (_use_vof_rho_phi && _outer_iteration_convective_state_valid)
    return libmesh_map_find(_outer_iteration_phi, fi.id());

  return libmesh_map_find(_corrected_face_phi, fi.id());
}

Real
SharpInterfaceRhieChowMassFlux::getVolumetricFaceFlux(const Moose::FV::InterpMethod m,
                                                      const FaceInfo & fi,
                                                      const Moose::StateArg & time,
                                                      const THREAD_ID /*tid*/,
                                                      bool subtract_mesh_velocity) const
{
  mooseAssert(!subtract_mesh_velocity,
              "SharpInterfaceRhieChowMassFlux does not support moving meshes yet!");

  if (m != Moose::FV::InterpMethod::RhieChow)
    mooseError("Interpolation methods other than Rhie-Chow are not supported!");
  if (time.state != Moose::currentState().state)
    mooseError("Older interpolation times are not supported!");

  return getVolumetricFaceFlux(fi);
}

Real
SharpInterfaceRhieChowMassFlux::maxVolumeFractionCourant(const Real dt) const
{
  return RhieChowMassFlux::maxCourant(dt);
}

void
SharpInterfaceRhieChowMassFlux::initializeAdditionalPressureFluxStorage(
    const bool preserve_corrected_face_phi)
{
  for (const auto * fi : _fe_problem.mesh().faceInfo())
  {
    _transient_projection_flux[fi->id()] = 0.0;
    _capillary_hydrostatic_flux[fi->id()] = 0.0;
    _pressure_Ainv[fi->id()] = RealVectorValue();
    _predictor_convective_mass_flux[fi->id()] = 0.0;
    _predictor_convective_phi[fi->id()] = 0.0;
    _predictor_operator_phi[fi->id()] = 0.0;
    _pressure_predictor_base_phi[fi->id()] = 0.0;
    _pressure_equation_volumetric_flux[fi->id()] = 0.0;
    _pressure_correction_phi[fi->id()] = 0.0;
    _sharp_pressure_predictor_flux[fi->id()] = 0.0;
    _reference_face_mass_flux_for_writeback[fi->id()] = 0.0;
    if (!preserve_corrected_face_phi)
      _corrected_face_phi[fi->id()] = 0.0;
    _outer_iteration_rho_phi[fi->id()] = 0.0;
    _outer_iteration_phi[fi->id()] = 0.0;
    _debug_update_hydrostatic_face_mass_flux_density_raw[fi->id()] = 0.0;
    _debug_update_physical_capillary_hydrostatic_flux[fi->id()] = 0.0;
    _debug_update_hydrostatic_branch_taken[fi->id()] = 0.0;
    _pressure_coupled_velocity_correction_scalar[fi->id()] = 0.0;
    _pressure_coupled_velocity_correction_face[fi->id()] = RealVectorValue();
    _corrected_face_velocity[fi->id()] = RealVectorValue();
  }

  _outer_iteration_convective_state_valid = false;
  _corrected_face_phi_seeded = preserve_corrected_face_phi && _corrected_face_phi_seeded;
  _pressure_coupled_velocity_correction_valid = false;
  _corrected_face_velocity_valid = false;
}

Real
SharpInterfaceRhieChowMassFlux::transportMassFluxDensityFromVolumetricPhi(
    const FaceInfo * fi, const Real volumetric_phi, const Moose::StateArg & time_arg) const
{
  const Real face_rho = interpolateFaceDensity(fi, time_arg);
  return face_rho * volumetric_phi;
}

Real
SharpInterfaceRhieChowMassFlux::transportIntegratedRhoPhiFromVolumetricPhi(
    const FaceInfo * fi, const Real volumetric_phi, const Moose::StateArg & time_arg) const
{
  if (!fi)
    return 0.0;

  const Real face_measure = fi->faceArea() * fi->faceCoord();
  return transportMassFluxDensityFromVolumetricPhi(fi, volumetric_phi, time_arg) * face_measure;
}

void
SharpInterfaceRhieChowMassFlux::cacheCurrentCorrectedVolumetricFlux()
{
  // Keep volumetric phi as the primary corrected face quantity. In the sharp
  // reduced-pressure path the pressure equation itself is assembled in
  // volumetric-flux space, so the final transported face flux is just the
  // predictor-plus-correction chain in that same operator space.
  const auto time_arg = Moose::currentState();
  for (const auto * fi : _sharp_interface_face_info)
  {
    if (_pressure_predictor_face_state_valid && _pressure_equation_flux_valid)
    {
      _pressure_equation_volumetric_flux[fi->id()] = libmesh_map_find(_pressure_equation_flux, fi->id());
      _pressure_correction_phi[fi->id()] =
          libmesh_map_find(_pressure_equation_flux, fi->id()) - libmesh_map_find(_phig_flux, fi->id());
      _corrected_face_phi[fi->id()] = -libmesh_map_find(_predictor_operator_phi, fi->id()) +
                                      _pressure_correction_phi[fi->id()];
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
SharpInterfaceRhieChowMassFlux::syncPredictorConvectiveStateFromCurrentFluxes()
{
  const auto time_arg = Moose::currentState();
  for (const auto * fi : _sharp_interface_face_info)
  {
    const Real paired_phi = libmesh_map_find(_corrected_face_phi, fi->id());
    _predictor_convective_mass_flux[fi->id()] =
        transportMassFluxDensityFromVolumetricPhi(fi, paired_phi, time_arg);
    _predictor_convective_phi[fi->id()] = paired_phi;
    _predictor_operator_phi[fi->id()] = 0.0;
    _pressure_predictor_base_phi[fi->id()] = 0.0;
    _pressure_equation_volumetric_flux[fi->id()] = 0.0;
    _pressure_correction_phi[fi->id()] = 0.0;
    _sharp_pressure_predictor_flux[fi->id()] = 0.0;
    _reference_face_mass_flux_for_writeback[fi->id()] = 0.0;
  }
}

void
SharpInterfaceRhieChowMassFlux::refreshPredictorConvectiveStateFromCurrentCorrectedFluxes()
{
  const auto time_arg = Moose::currentState();
  for (const auto * fi : _sharp_interface_face_info)
  {
    const Real paired_phi = libmesh_map_find(_corrected_face_phi, fi->id());
    const Real face_measure = fi->faceArea() * fi->faceCoord();

    Real integrated_rho_phi =
        transportIntegratedRhoPhiFromVolumetricPhi(fi, paired_phi, time_arg);

    if (_outer_iteration_convective_state_valid && _vof_alpha_phi_limited && _liquid_density &&
        _gas_density && face_measure > libMesh::TOLERANCE)
    {
      const Real alpha_phi_limited =
          evaluateFaceScalarFunctor(_vof_alpha_phi_limited, fi, time_arg, nullptr);
      const Real liquid_density =
          evaluateCellBasedFaceScalarFunctor(_liquid_density, fi, time_arg);
      const Real gas_density = evaluateCellBasedFaceScalarFunctor(_gas_density, fi, time_arg);

      integrated_rho_phi =
          paired_phi * face_measure * gas_density +
          (liquid_density - gas_density) * alpha_phi_limited;
    }

    _predictor_convective_phi[fi->id()] = paired_phi;
    _predictor_convective_mass_flux[fi->id()] =
        face_measure > libMesh::TOLERANCE ? integrated_rho_phi / face_measure : 0.0;
  }
}

void
SharpInterfaceRhieChowMassFlux::freezeOuterIterationConvectiveState()
{
  const auto time_arg = Moose::currentState();
  for (const auto * fi : _sharp_interface_face_info)
  {
    Real paired_phi = libmesh_map_find(_corrected_face_phi, fi->id());
    Real integrated_rho_phi =
        transportIntegratedRhoPhiFromVolumetricPhi(fi, paired_phi, time_arg);
    Real vof_integrated_rho_phi = integrated_rho_phi;
    if (_vof_rho_phi)
    {
      vof_integrated_rho_phi = evaluateFaceScalarFunctor(_vof_rho_phi, fi, time_arg, nullptr);
      integrated_rho_phi = vof_integrated_rho_phi;
    }

    const Real face_measure = fi->faceArea() * fi->faceCoord();
    if (_vof_rho_phi && _vof_alpha_phi_limited && _liquid_density && _gas_density &&
        face_measure > libMesh::TOLERANCE)
    {
      const Real alpha_phi_limited =
          evaluateFaceScalarFunctor(_vof_alpha_phi_limited, fi, time_arg, nullptr);
      const Real liquid_density = evaluateCellBasedFaceScalarFunctor(_liquid_density, fi, time_arg);
      const Real gas_density = evaluateCellBasedFaceScalarFunctor(_gas_density, fi, time_arg);
      if (std::abs(gas_density) > libMesh::TOLERANCE)
        paired_phi =
            (vof_integrated_rho_phi - (liquid_density - gas_density) * alpha_phi_limited) /
            (gas_density * face_measure);
    }

    _outer_iteration_rho_phi[fi->id()] = integrated_rho_phi;
    _outer_iteration_phi[fi->id()] = paired_phi;
    _predictor_convective_mass_flux[fi->id()] =
        face_measure > libMesh::TOLERANCE ? integrated_rho_phi / face_measure : 0.0;
    _predictor_convective_phi[fi->id()] = paired_phi;
    _predictor_operator_phi[fi->id()] = 0.0;
    _pressure_predictor_base_phi[fi->id()] = 0.0;
    _pressure_equation_volumetric_flux[fi->id()] = 0.0;
    _pressure_correction_phi[fi->id()] = 0.0;
    _sharp_pressure_predictor_flux[fi->id()] = 0.0;
    _reference_face_mass_flux_for_writeback[fi->id()] = 0.0;
  }

  _outer_iteration_convective_state_valid = true;
}

void
SharpInterfaceRhieChowMassFlux::clearOuterIterationConvectiveState()
{
  for (auto & pair : _outer_iteration_rho_phi)
    pair.second = 0.0;
  for (auto & pair : _outer_iteration_phi)
    pair.second = 0.0;
  for (auto & pair : _predictor_operator_phi)
    pair.second = 0.0;
  for (auto & pair : _pressure_predictor_base_phi)
    pair.second = 0.0;
  for (auto & pair : _pressure_equation_volumetric_flux)
    pair.second = 0.0;
  for (auto & pair : _pressure_correction_phi)
    pair.second = 0.0;
  for (auto & pair : _sharp_pressure_predictor_flux)
    pair.second = 0.0;
  for (auto & pair : _reference_face_mass_flux_for_writeback)
    pair.second = 0.0;

  _outer_iteration_convective_state_valid = false;
  syncPredictorConvectiveStateFromCurrentFluxes();
}

void
SharpInterfaceRhieChowMassFlux::updatePredictorOperatorPhiField(const Moose::StateArg & time_arg)
{
  for (const auto * fi : _sharp_interface_face_info)
    _predictor_operator_phi[fi->id()] = -(referenceFaceVelocityState(fi, time_arg) * fi->normal());
}

void
SharpInterfaceRhieChowMassFlux::rebuildSharpInterfaceFaceInfo()
{
  _sharp_interface_face_info.clear();
  for (auto & fi : _fe_problem.mesh().faceInfo())
    if (hasBlocks(fi->elemPtr()->subdomain_id()) ||
        (fi->neighborPtr() && hasBlocks(fi->neighborPtr()->subdomain_id())))
      _sharp_interface_face_info.push_back(fi);

  initializeAdditionalPressureFluxStorage();
}

void
SharpInterfaceRhieChowMassFlux::meshChanged()
{
  RhieChowMassFlux::meshChanged();
  rebuildSharpInterfaceFaceInfo();
}

void
SharpInterfaceRhieChowMassFlux::initialSetup()
{
  const_cast<MooseLinearVariableFVReal *>(_p)->computeCellGradients();
  RhieChowMassFlux::initialSetup();
  rebuildSharpInterfaceFaceInfo();
  if (!_vof_rho_phi_name.empty() &&
      UserObject::_subproblem.hasFunctorWithType<Real>(_vof_rho_phi_name, _tid))
    _vof_rho_phi = &getFunctor<Real>(_vof_rho_phi_name);
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
SharpInterfaceRhieChowMassFlux::initialize()
{
  RhieChowMassFlux::initialize();

  initializeAdditionalPressureFluxStorage(/*preserve_corrected_face_phi=*/true);
  if (!_corrected_face_phi_seeded)
    cacheCurrentCorrectedVolumetricFlux();
}

void
SharpInterfaceRhieChowMassFlux::initFaceMassFlux()
{
  RhieChowMassFlux::initFaceMassFlux();
}

void
SharpInterfaceRhieChowMassFlux::computeFaceMassFlux()
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
SharpInterfaceRhieChowMassFlux::makeCenteredFaceArg(const FaceInfo * fi,
                                                    const Moose::StateArg * limiter_state) const
{
  return Moose::FaceArg{
      fi, Moose::FV::LimiterType::CentralDifference, true, false, nullptr, limiter_state};
}

Real
SharpInterfaceRhieChowMassFlux::interpolateFaceDensity(const FaceInfo * fi,
                                                       const Moose::StateArg & time_arg) const
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
SharpInterfaceRhieChowMassFlux::predictorFaceDensity(const FaceInfo * fi,
                                                     const Moose::StateArg & time_arg) const
{
  // In the sharp-interface VOF path, rhoPhi is assembled from the primary volumetric
  // flux plus the limited alpha transport flux. It is therefore not generally equal
  // to rho_f * phi, and recovering a predictor "face density" from rhoPhi / phi
  // injects a non-physical variable-density mismatch into the pressure predictor and
  // velocity writeback chain.
  return interpolateFaceDensity(fi, time_arg);
}

RealVectorValue
SharpInterfaceRhieChowMassFlux::interpolateFaceRawAinv(const FaceInfo * fi) const
{
  std::vector<std::unique_ptr<NumericVector<Number>>> owned_raw_ainv;
  std::vector<PetscVectorReader> raw_ainv_readers;
  buildSharpFaceRawAinvReaders(owned_raw_ainv, raw_ainv_readers);
  return interpolateFaceRawAinv(fi, raw_ainv_readers);
}

RealVectorValue
SharpInterfaceRhieChowMassFlux::interpolateFaceRawAinv(
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
SharpInterfaceRhieChowMassFlux::buildSharpFaceRawAinvReaders(
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

SharpInterfaceRhieChowMassFlux::SharpFaceOperatorState
SharpInterfaceRhieChowMassFlux::buildSharpFaceOperatorState(
    const FaceInfo * fi,
    const Moose::StateArg & time_arg,
    const std::vector<PetscVectorReader> & raw_ainv_readers) const
{
  SharpFaceOperatorState state;
  state.face_normal = fi->normal();
  state.face_rho = interpolateFaceDensity(fi, time_arg);
  state.face_raw_ainv = interpolateFaceRawAinv(fi, raw_ainv_readers);
  state.normal_raw_ainv = computeFaceNormalRawAinv(state.face_raw_ainv, state.face_normal);
  state.negative_sn_grad_p = -computeFaceNormalPressureGradient(fi, time_arg);

  if (_hydrostatic_density_gradient_face_acceleration &&
      !_suppress_startup_pressure_predictor_flux_sources &&
      !_suppress_explicit_hydrostatic_pressure_flux)
    state.hydrostatic_mass_flux_density_raw =
        computeHydrostaticFaceMassFlux(
            fi, state.face_rho, state.face_raw_ainv, state.face_normal, time_arg);

  return state;
}

RealVectorValue
SharpInterfaceRhieChowMassFlux::interpolateFaceRau(const FaceInfo * fi) const
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
SharpInterfaceRhieChowMassFlux::evaluateFaceVectorFunctor(
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
SharpInterfaceRhieChowMassFlux::evaluateBoundaryAwareVectorFunctor(
    const Moose::Functor<RealVectorValue> * functor,
    const FaceInfo * fi,
    const Moose::StateArg & time_arg) const
{
  if (!functor)
    return RealVectorValue();

  return MetaPhysicL::raw_value((*functor)(makeCenteredFaceArg(fi), time_arg));
}

RealVectorValue
SharpInterfaceRhieChowMassFlux::interpolateCellVectorFunctorToFace(
    const Moose::Functor<RealVectorValue> * functor,
    const FaceInfo * fi,
    const Moose::StateArg & time_arg) const
{
  if (!functor)
    return RealVectorValue();

  if (_vel[0]->isInternalFace(*fi))
  {
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
SharpInterfaceRhieChowMassFlux::interpolateCellBodyForceDensityToFace(
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
    return 0.5 * (evaluate_cell_body_force(fi->elemPtr()) +
                  evaluate_cell_body_force(fi->neighborPtr()));

  const bool elem_is_fluid = hasBlocks(fi->elemPtr()->subdomain_id());
  const Elem * const fluid_elem = elem_is_fluid ? fi->elemPtr() : fi->neighborPtr();
  return evaluate_cell_body_force(fluid_elem);
}

Real
SharpInterfaceRhieChowMassFlux::evaluateFaceScalarFunctor(const Moose::Functor<Real> * functor,
                                                          const FaceInfo * fi,
                                                          const Moose::StateArg & time_arg,
                                                          const Moose::StateArg * limiter_state) const
{
  if (!functor)
    return 0.0;

  return MetaPhysicL::raw_value((*functor)(makeCenteredFaceArg(fi, limiter_state), time_arg));
}

Real
SharpInterfaceRhieChowMassFlux::evaluateBoundaryAwareScalarFunctor(
    const Moose::Functor<Real> * functor,
    const FaceInfo * fi,
    const Moose::StateArg & time_arg) const
{
  if (!functor)
    return 0.0;

  return MetaPhysicL::raw_value((*functor)(makeCenteredFaceArg(fi), time_arg));
}

Real
SharpInterfaceRhieChowMassFlux::evaluateCellBasedFaceScalarFunctor(
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
SharpInterfaceRhieChowMassFlux::projectPhysicalMassFluxDensity(const Real face_rho,
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
SharpInterfaceRhieChowMassFlux::computeFaceNormalRawAinv(
    const RealVectorValue & face_ainv_raw, const RealVectorValue & face_normal) const
{
  Real normal_ainv = 0.0;
  for (const auto dim_i : make_range(_dim))
    normal_ainv += face_ainv_raw(dim_i) * face_normal(dim_i) * face_normal(dim_i);

  return normal_ainv;
}

Real
SharpInterfaceRhieChowMassFlux::massFluxDensityToVolumetricNormalFlux(
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
SharpInterfaceRhieChowMassFlux::computeDiscretePressureFaceVolumetricFlux(const FaceInfo * fi) const
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

      return p_elem_value * matrix_contribution - rhs_contribution;
    }
  }

  return 0.0;
}

Real
SharpInterfaceRhieChowMassFlux::computeFaceNormalDensityGradient(
    const FaceInfo * fi, const Moose::StateArg & time_arg) const
{
  if (_vel[0]->isInternalFace(*fi))
  {
    const Real elem_rho = _rho(makeElemArg(fi->elemPtr()), time_arg);
    const Real neighbor_rho = _rho(makeElemArg(fi->neighborPtr()), time_arg);
    const Real normal_spacing = std::abs(fi->dCN() * fi->normal());

    return normal_spacing > libMesh::TOLERANCE ? (neighbor_rho - elem_rho) / normal_spacing : 0.0;
  }

  const bool elem_is_fluid = hasBlocks(fi->elemPtr()->subdomain_id());
  const Elem * const fluid_elem = elem_is_fluid ? fi->elemPtr() : fi->neighborPtr();
  const Point fluid_centroid = elem_is_fluid ? fi->elemCentroid() : fi->neighborCentroid();
  const Point cell_to_face = fi->faceCentroid() - fluid_centroid;
  const Real normal_spacing = std::abs(cell_to_face * fi->normal());

  if (normal_spacing <= libMesh::TOLERANCE)
    return 0.0;

  const Real elem_rho = _rho(makeElemArg(fluid_elem), time_arg);
  const Moose::FaceArg boundary_face{
      fi, Moose::FV::LimiterType::CentralDifference, true, false, fluid_elem, nullptr};
  const Real boundary_rho = _rho(boundary_face, time_arg);

  return (boundary_rho - elem_rho) / normal_spacing;
}

Real
SharpInterfaceRhieChowMassFlux::computeFaceNormalPressureGradient(
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
  const Elem * const fluid_elem = elem_is_fluid ? fi->elemPtr() : fi->neighborPtr();
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
SharpInterfaceRhieChowMassFlux::computeHydrostaticFaceMassFlux(
    const FaceInfo * fi,
    const Real face_rho,
    const RealVectorValue & face_ainv_raw,
    const RealVectorValue & face_normal,
    const Moose::StateArg & time_arg) const
{
  const Real sn_grad_rho = computeFaceNormalDensityGradient(fi, time_arg);
  if (std::abs(sn_grad_rho) <= libMesh::TOLERANCE)
    return 0.0;

  const Point x = fi->faceCentroid();
  const Real gh = _gravity * (x - _reference_pressure_point);

  RealVectorValue hydrostatic_accel =
      -(gh / std::max(face_rho, libMesh::TOLERANCE)) * sn_grad_rho * face_normal;
  return projectPhysicalMassFluxDensity(face_rho, face_ainv_raw, hydrostatic_accel, face_normal);
}

bool
SharpInterfaceRhieChowMassFlux::populateMomentumPredictorBodyForceFaceField(
    FaceVectorField & face_field, const Moose::StateArg & time_arg) const
{
  const bool have_surface_face_force = _surface_tension_face_acceleration;

  if (!have_surface_face_force)
    return false;

  std::vector<std::unique_ptr<NumericVector<Number>>> owned_raw_ainv;
  std::vector<PetscVectorReader> raw_ainv_readers;
  buildSharpFaceRawAinvReaders(owned_raw_ainv, raw_ainv_readers);

  for (const auto * fi : _sharp_interface_face_info)
  {
    const auto state = buildSharpFaceOperatorState(fi, time_arg, raw_ainv_readers);
    RealVectorValue face_body_force_density;
    Real physical_mass_flux_density = 0.0;
    bool used_pressure_family_operator = false;

    if (have_surface_face_force)
    {
      const auto surface_accel =
          evaluateBoundaryAwareVectorFunctor(_surface_tension_face_acceleration, fi, time_arg);
      face_body_force_density += state.face_rho * surface_accel;
      physical_mass_flux_density +=
          projectPhysicalMassFluxDensity(
              state.face_rho, state.face_raw_ainv, surface_accel, state.face_normal);
      used_pressure_family_operator = true;
    }
    else if (_surface_tension_cell_acceleration)
    {
      const auto surface_force_density =
          interpolateCellBodyForceDensityToFace(_surface_tension_cell_acceleration, fi, time_arg);
      face_body_force_density += (surface_force_density * state.face_normal) * state.face_normal;
    }

    const Real normal_force_density =
        used_pressure_family_operator && std::abs(state.normal_raw_ainv) > libMesh::TOLERANCE
            ? physical_mass_flux_density / state.normal_raw_ainv
            : face_body_force_density * state.face_normal;
    face_field[fi->id()] = normal_force_density * state.face_normal;
  }

  return true;
}

RealVectorValue
SharpInterfaceRhieChowMassFlux::evaluateCellMomentumPredictorPressureForceDensity(
    const ElemInfo & elem_info) const
{
  RealVectorValue pressure_force_density;

  for (const auto dim_i : make_range(_dim))
    pressure_force_density(dim_i) = -_p->gradSlnComponent(elem_info, dim_i);

  return pressure_force_density;
}

RealVectorValue
SharpInterfaceRhieChowMassFlux::evaluateCellHydrostaticMomentumPredictorBodyForceDensity(
    const ElemInfo * elem_info, const Moose::StateArg & time_arg) const
{
  RealVectorValue body_force_density;

  if (!elem_info)
    return body_force_density;

  const auto * const elem = elem_info->elem();
  const auto elem_arg = makeElemArg(elem);

  if (_hydrostatic_density_gradient_cell_acceleration)
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
SharpInterfaceRhieChowMassFlux::evaluateLegacyMomentumPredictorBodyForceDensity(
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
SharpInterfaceRhieChowMassFlux::populateMomentumPredictorPressureForceFaceField(
    FaceVectorField & face_field, const Moose::StateArg & time_arg)
{
  (void)time_arg;
  bool found_nonzero_force = false;
  std::vector<std::unique_ptr<NumericVector<Number>>> owned_raw_ainv;
  std::vector<PetscVectorReader> raw_ainv_readers;
  buildSharpFaceRawAinvReaders(owned_raw_ainv, raw_ainv_readers);
  for (const auto * fi : _sharp_interface_face_info)
  {
    const auto state = buildSharpFaceOperatorState(fi, time_arg, raw_ainv_readers);
    const Real discrete_pressure_flux = computeDiscretePressureFaceVolumetricFlux(fi);
    const Real normal_force_density =
        std::abs(state.normal_raw_ainv) > libMesh::TOLERANCE
            ? discrete_pressure_flux / state.normal_raw_ainv
            : 0.0;

    if (std::abs(normal_force_density) > libMesh::TOLERANCE)
      found_nonzero_force = true;

    face_field[fi->id()] = normal_force_density * state.face_normal;
  }

  return found_nonzero_force;
}

RealVectorValue
SharpInterfaceRhieChowMassFlux::evaluateFaceBasedMomentumPredictorPressureForceDensity(
    const ElemInfo * elem_info,
    const Moose::StateArg & time_arg,
    const FaceVectorField * face_field) const
{
  if (!elem_info || !face_field)
    return RealVectorValue();

  return (*face_field)(makeElemArg(elem_info->elem()), time_arg);
}

RealVectorValue
SharpInterfaceRhieChowMassFlux::reconstructFaceVectorFieldToCellSourceDensity(
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

  if (!elem_info)
    return RealVectorValue();

  const Elem * const elem = elem_info->elem();
  if (!elem)
    return RealVectorValue();

  const Real cell_volume = elem_info->volume() * elem_info->coordFactor();
  if (cell_volume <= libMesh::TOLERANCE)
    return RealVectorValue();

  RealVectorValue accumulated_face_source;
  for (const auto side : make_range(elem->n_sides()))
  {
    const Elem * const loc_neighbor = elem->neighbor_ptr(side);
    const bool elem_has_fi = Moose::FV::elemHasFaceInfo(*elem, loc_neighbor);
    const FaceInfo * const fi_loc =
        _moose_mesh.faceInfo(elem_has_fi ? elem : loc_neighbor,
                             elem_has_fi ? side : loc_neighbor->which_neighbor_am_i(elem));
    if (!fi_loc)
      continue;

    const Real orientation = fi_loc->elemPtr() == elem ? 1.0 : -1.0;
    const Real face_weight = fi_loc->faceArea() * fi_loc->faceCoord();
    accumulated_face_source +=
        orientation * face_weight * face_field(makeCenteredFaceArg(fi_loc), time_arg);
  }

  return accumulated_face_source / cell_volume;
}

RealVectorValue
SharpInterfaceRhieChowMassFlux::evaluateFaceBasedMomentumPredictorBodyForceDensity(
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

  if (face_field)
    body_force_density += (*face_field)(elem_arg, time_arg);

  if (!_surface_tension_face_acceleration && _surface_tension_cell_acceleration)
    body_force_density +=
        rho * MetaPhysicL::raw_value((*_surface_tension_cell_acceleration)(elem_arg, time_arg));

  body_force_density += evaluateCellHydrostaticMomentumPredictorBodyForceDensity(elem_info, time_arg);

  return body_force_density;
}

RealVectorValue
SharpInterfaceRhieChowMassFlux::evaluateMomentumPredictorBodyForceDensity(
    const ElemInfo * elem_info,
    const Moose::StateArg & time_arg,
    const FaceVectorField * face_field) const
{
  if (_use_face_based_predictor_body_force && face_field)
    return evaluateFaceBasedMomentumPredictorBodyForceDensity(elem_info, time_arg, face_field);

  return evaluateLegacyMomentumPredictorBodyForceDensity(elem_info, time_arg);
}

RealVectorValue
SharpInterfaceRhieChowMassFlux::computeDefaultTransientProjectionFaceAcceleration(
    const FaceInfo * fi, const Moose::StateArg & time_arg) const
{
  if (!fi || !_is_transient || _dt <= libMesh::TOLERANCE ||
      time_arg.state != Moose::currentState().state)
    return RealVectorValue();

  const auto old_state = Moose::StateArg(1, Moose::SolutionIterationType::Time);
  const Real old_stored_phi = _corrected_face_phi(makeCenteredFaceArg(fi), old_state);

  Real old_interpolated_phi = 0.0;
  if (_vel[0]->isInternalFace(*fi))
  {
    const auto & elem_info = *fi->elemInfo();
    const auto & neighbor_info = *fi->neighborInfo();
    RealVectorValue old_face_velocity;

    for (const auto dim_i : index_range(_vel))
      interpolate(Moose::FV::InterpMethod::Average,
                  old_face_velocity(dim_i),
                  _vel[dim_i]->getElemValue(elem_info, old_state),
                  _vel[dim_i]->getElemValue(neighbor_info, old_state),
                  *fi,
                  true);

    old_interpolated_phi = old_face_velocity * fi->normal();
  }
  else
  {
    // Match OpenFOAM's ddtCouplingCoeff boundary behavior: no transient
    // pressure-predictor correction on fixed-value velocity patches.
    if (useConstrainedBoundaryPredictorState(fi))
      return RealVectorValue();

    const bool elem_is_fluid = fi->elemPtr() && hasBlocks(fi->elemPtr()->subdomain_id());
    const ElemInfo * const fluid_elem_info = elem_is_fluid ? fi->elemInfo() : fi->neighborInfo();
    if (!fluid_elem_info)
      return RealVectorValue();

    RealVectorValue old_face_velocity;
    for (const auto dim_i : index_range(_vel))
      old_face_velocity(dim_i) = _vel[dim_i]->getElemValue(*fluid_elem_info, old_state);

    const Real boundary_normal_multiplier = elem_is_fluid ? 1.0 : -1.0;
    old_interpolated_phi = boundary_normal_multiplier * (old_face_velocity * fi->normal());
  }

  const Real phi_corr = old_stored_phi - old_interpolated_phi;
  const Real phi_scale =
      std::abs(old_stored_phi) + std::numeric_limits<Real>::epsilon();
  const Real ddt_coupling_coeff =
      std::max(0.0, 1.0 - std::min(std::abs(phi_corr) / phi_scale, 1.0));

  return ddt_coupling_coeff * (phi_corr / _dt) * fi->normal();
}

Real
SharpInterfaceRhieChowMassFlux::pressureVelocityWritebackFluxDensity(const FaceInfo * fi) const
{
  const Real correction_phi =
      libmesh_map_find(_pressure_equation_flux, fi->id()) - libmesh_map_find(_phig_flux, fi->id());
  return transportMassFluxDensityFromVolumetricPhi(fi, correction_phi, Moose::currentState());
}

void
SharpInterfaceRhieChowMassFlux::updateAdditionalPressureFluxFunctors(const bool with_updated_pressure,
                                                                     const bool verbose)
{
  (void)with_updated_pressure;

  _pressure_coupled_velocity_correction_valid = false;
  _pressure_predictor_face_state_valid = false;
  _corrected_face_velocity_valid = false;

  const auto time_arg = Moose::currentState();
  updatePredictorOperatorPhiField(time_arg);

  std::vector<std::unique_ptr<NumericVector<Number>>> owned_raw_ainv;
  std::vector<PetscVectorReader> raw_ainv_readers;
  buildSharpFaceRawAinvReaders(owned_raw_ainv, raw_ainv_readers);

  for (const auto * fi : _sharp_interface_face_info)
  {
    const auto state = buildSharpFaceOperatorState(fi, time_arg, raw_ainv_readers);
    const Real predictor_operator_phi = _predictor_operator_phi[fi->id()];
    const Real predictor_operator_mass_flux =
        transportMassFluxDensityFromVolumetricPhi(fi, -predictor_operator_phi, time_arg);

    Real physical_transient_flux = 0.0;
    if (!_suppress_startup_pressure_predictor_flux_sources && _add_transient_projection_flux)
    {
      const auto transient_accel = _transient_projection_face_acceleration
                                       ? evaluateBoundaryAwareVectorFunctor(
                                             _transient_projection_face_acceleration, fi, time_arg)
                                       : computeDefaultTransientProjectionFaceAcceleration(fi,
                                                                                           time_arg);
      physical_transient_flux =
          projectPhysicalMassFluxDensity(
              state.face_rho, state.face_raw_ainv, transient_accel, state.face_normal);
    }

    RealVectorValue capillary_hydrostatic_accel;
    if (_add_capillary_hydrostatic_flux)
    {
      if (_surface_tension_face_acceleration)
        capillary_hydrostatic_accel +=
            evaluateBoundaryAwareVectorFunctor(_surface_tension_face_acceleration, fi, time_arg);
    }

    Real physical_capillary_hydrostatic_flux =
        projectPhysicalMassFluxDensity(state.face_rho,
                                       state.face_raw_ainv,
                                       capillary_hydrostatic_accel,
                                       state.face_normal);
    _pressure_Ainv[fi->id()] = state.face_raw_ainv;
    const bool add_hydrostatic_branch =
        !_suppress_startup_pressure_predictor_flux_sources && _add_capillary_hydrostatic_flux &&
        _hydrostatic_density_gradient_face_acceleration &&
        !_suppress_explicit_hydrostatic_pressure_flux;
    Real hydrostatic_face_mass_flux_density_raw = 0.0;
    if (add_hydrostatic_branch)
    {
      hydrostatic_face_mass_flux_density_raw = state.hydrostatic_mass_flux_density_raw;
      physical_capillary_hydrostatic_flux += hydrostatic_face_mass_flux_density_raw;
    }

    _debug_update_hydrostatic_branch_taken[fi->id()] = add_hydrostatic_branch ? 1.0 : 0.0;
    _debug_update_hydrostatic_face_mass_flux_density_raw[fi->id()] =
        hydrostatic_face_mass_flux_density_raw;
    _debug_update_physical_capillary_hydrostatic_flux[fi->id()] =
        physical_capillary_hydrostatic_flux;

    // These source branches are already projected with the raw pressure
    // operator, rho_f * Ainv_raw,f * accel_f . n_f, so they are already in the
    // same volumetric-flux space as the sharp reduced-pressure equation. Do not
    // run them back through the density-weighted Rhie-Chow conversion here.
    _transient_projection_flux[fi->id()] = -physical_transient_flux;
    _capillary_hydrostatic_flux[fi->id()] = -physical_capillary_hydrostatic_flux;
    _phig_flux[fi->id()] =
        _transient_projection_flux[fi->id()] + _capillary_hydrostatic_flux[fi->id()];
    _predictor_operator_phi[fi->id()] = predictor_operator_phi;

    // Keep the carried outer-iteration phi branch for alpha transport, but use
    // the operator-built face state for the pressure predictor. The final cell
    // writeback is now driven by a matched scalar correction solve plus the
    // pressure-coupled reconstruction, so the pressure-side predictor branch
    // must stay on the directly assembled operator state here.
    const Real sharp_predictor_phi = predictor_operator_phi;
    _sharp_pressure_predictor_flux[fi->id()] = sharp_predictor_phi + _phig_flux[fi->id()];
    _pressure_predictor_base_phi[fi->id()] = _sharp_pressure_predictor_flux[fi->id()];
    _pressure_predictor_base_flux[fi->id()] = _sharp_pressure_predictor_flux[fi->id()];
    _phiHbyA_flux[fi->id()] = _sharp_pressure_predictor_flux[fi->id()];
    _reference_face_mass_flux_for_writeback[fi->id()] = predictor_operator_mass_flux;

    if (verbose)
    {
      _console << "Sharp-interface predictor on face " << fi->id() << ": transient_source_flux="
               << _transient_projection_flux[fi->id()]
               << ", capillary_hydrostatic_source_flux="
               << _capillary_hydrostatic_flux[fi->id()]
               << ", phig_flux=" << _phig_flux[fi->id()]
               << ", predictor_operator_phi=" << _predictor_operator_phi[fi->id()]
               << ", sharp_predictor_phi=" << sharp_predictor_phi
               << ", pressure_predictor_base_phi=" << _pressure_predictor_base_phi[fi->id()]
               << ", phiHbyA_flux=" << _phiHbyA_flux[fi->id()]
               << std::endl;
    }
  }

  _pressure_predictor_face_state_valid = true;
}

void
SharpInterfaceRhieChowMassFlux::updatePressureCoupledVelocityCorrectionFaceField(
    const Moose::StateArg & time_arg)
{
  if (!_pressure_equation_flux_valid)
    cachePressureEquationFlux();

  for (const auto * fi : _sharp_interface_face_info)
  {
    // The live writeback must consume the same physical pressure-correction
    // branch that the reduced-pressure face update and consistency audits use:
    //   pressure_equation_flux - phig_flux.
    // Using the total final-minus-reference face increment here mixes the
    // pressure branch with the predictor/cell-face mismatch, which is a
    // different discrete error source.
    const Real pressure_coupled_phi =
        libmesh_map_find(_pressure_equation_flux, fi->id()) - libmesh_map_find(_phig_flux, fi->id());
    const RealVectorValue face_normal = fi->normal();

    // Keep the scalar correction for audits. This is the face-normal velocity
    // correction implied directly by the physical pressure-correction branch.
    const Real psi_f = pressure_coupled_phi;
    _pressure_coupled_velocity_correction_scalar[fi->id()] = psi_f;

    // Keep phi authoritative and publish only the physical pressure-correction
    // flux branch for the final cell writeback:
    //   pressureVelocityWritebackFluxDensity = pressure_equation_flux - phig_flux.
    // Do not mix any additional explicit source terms back into the writeback here.
    // They already entered the physical pressure/face-flux update through phiHbyA/phig.
    _pressure_coupled_velocity_correction_face[fi->id()] = psi_f * face_normal;
  }

  _pressure_coupled_velocity_correction_valid = true;
}

bool
SharpInterfaceRhieChowMassFlux::useConstrainedBoundaryPredictorState(const FaceInfo * fi) const
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
        if (auto * pressure_inlet_outlet_bc =
                dynamic_cast<LinearFVPressureInletOutletVelocityBC *>(bc_pointer))
        {
          pressure_inlet_outlet_bc->setupFaceData(
              fi,
              fi->faceType(std::make_pair(_vel[dim_i]->number(), _vel[dim_i]->sys().number())));
          if (pressure_inlet_outlet_bc->computeBoundaryGradientMatrixContribution() > 0.0)
          {
            use_constrained_boundary_state = true;
            break;
          }
        }
  }

  return use_constrained_boundary_state;
}

RealVectorValue
SharpInterfaceRhieChowMassFlux::referenceFaceVelocityState(const FaceInfo * fi,
                                                           const Moose::StateArg & time_arg) const
{
  using namespace Moose::FV;

  RealVectorValue face_velocity;

  if (_vel[0]->isInternalFace(*fi))
  {
    const auto & elem_info = *fi->elemInfo();
    const auto & neighbor_info = *fi->neighborInfo();

    for (const auto dim_i : make_range(_dim))
      interpolate(InterpMethod::Average,
                  face_velocity(dim_i),
                  predictorVelocityComponent(elem_info, dim_i),
                  predictorVelocityComponent(neighbor_info, dim_i),
                  *fi,
                  true);

    return face_velocity;
  }

  const bool elem_is_fluid = hasBlocks(fi->elemPtr()->subdomain_id());
  const ElemInfo & elem_info = elem_is_fluid ? *fi->elemInfo() : *fi->neighborInfo();
  const Moose::FaceArg boundary_face{
      fi, Moose::FV::LimiterType::CentralDifference, true, false, elem_info.elem(), nullptr};

  if (useConstrainedBoundaryPredictorState(fi))
  {
    for (const auto dim_i : make_range(_dim))
    {
      Real boundary_value = std::numeric_limits<Real>::quiet_NaN();

      if (!fi->boundaryIDs().empty())
      {
        mooseAssert(fi->boundaryIDs().size() == 1,
                    "Expected at most one physical boundary id on a FV boundary face.");
        if (auto * bc_pointer = _vel[dim_i]->getBoundaryCondition(*fi->boundaryIDs().begin()))
        {
          bc_pointer->setupFaceData(
              fi,
              fi->faceType(
                  std::make_pair(_vel[dim_i]->number(), _vel[dim_i]->sys().number())));
          boundary_value = bc_pointer->computeBoundaryValue();
        }
      }

      face_velocity(dim_i) =
          std::isfinite(boundary_value)
              ? boundary_value
              : MetaPhysicL::raw_value((*_vel[dim_i])(boundary_face, time_arg));
    }
  }
  else
    for (const auto dim_i : make_range(_dim))
      face_velocity(dim_i) = predictorVelocityComponent(elem_info, dim_i);

  return face_velocity;
}

Real
SharpInterfaceRhieChowMassFlux::referenceFaceMassFluxState(const FaceInfo * fi,
                                                           const Moose::StateArg & time_arg) const
{
  using namespace Moose::FV;

  if (_vel[0]->isInternalFace(*fi))
  {
    const auto & elem_info = *fi->elemInfo();
    const auto & neighbor_info = *fi->neighborInfo();
    const Real elem_rho = _rho(makeElemArg(fi->elemPtr()), time_arg);
    const Real neighbor_rho = _rho(makeElemArg(fi->neighborPtr()), time_arg);
    RealVectorValue density_times_velocity;

    for (const auto dim_i : make_range(_dim))
      interpolate(InterpMethod::Average,
                  density_times_velocity(dim_i),
                  elem_rho * predictorVelocityComponent(elem_info, dim_i),
                  neighbor_rho * predictorVelocityComponent(neighbor_info, dim_i),
                  *fi,
                  true);

    return density_times_velocity * fi->normal();
  }

  const bool elem_is_fluid = hasBlocks(fi->elemPtr()->subdomain_id());
  const ElemInfo & elem_info = elem_is_fluid ? *fi->elemInfo() : *fi->neighborInfo();
  const Real boundary_normal_multiplier = elem_is_fluid ? 1.0 : -1.0;

  if (useConstrainedBoundaryPredictorState(fi))
  {
    const Moose::FaceArg boundary_face{
        fi, Moose::FV::LimiterType::CentralDifference, true, false, elem_info.elem(), nullptr};
    const Real face_rho = _rho(boundary_face, time_arg);
    const RealVectorValue predictor_face_velocity = referenceFaceVelocityState(fi, time_arg);
    return boundary_normal_multiplier * face_rho * (predictor_face_velocity * fi->normal());
  }

  const Real cell_rho = _rho(makeElemArg(elem_info.elem()), time_arg);
  RealVectorValue density_times_velocity;
  for (const auto dim_i : make_range(_dim))
    density_times_velocity(dim_i) =
        boundary_normal_multiplier * cell_rho * predictorVelocityComponent(elem_info, dim_i);

  return density_times_velocity * fi->normal();
}

void
SharpInterfaceRhieChowMassFlux::updateCorrectedFaceVelocityField(const Moose::StateArg & time_arg)
{
  for (const auto * fi : _sharp_interface_face_info)
  {
    const RealVectorValue face_normal = fi->normal();
    const RealVectorValue reference_face_velocity = referenceFaceVelocityState(fi, time_arg);
    const Real reference_normal_velocity = reference_face_velocity * face_normal;
    const RealVectorValue reference_tangential_velocity =
        reference_face_velocity - reference_normal_velocity * face_normal;

    const Real corrected_normal_velocity = libmesh_map_find(_corrected_face_phi, fi->id());

    _corrected_face_velocity[fi->id()] =
        reference_tangential_velocity + corrected_normal_velocity * face_normal;
  }

  _corrected_face_velocity_valid = true;
}

bool
SharpInterfaceRhieChowMassFlux::shouldUseCorrectedBoundaryVelocityState(const FaceInfo * fi) const
{
  if (!_corrected_face_velocity_valid || !fi || _vel[0]->isInternalFace(*fi) || fi->boundaryIDs().empty())
    return false;

  mooseAssert(fi->boundaryIDs().size() == 1, "Expected a single boundary id on a FV boundary face.");
  const auto boundary_id = *fi->boundaryIDs().begin();

  for (const auto component : make_range(_dim))
    if (const auto * bc_pointer = _vel[component]->getBoundaryCondition(boundary_id);
        dynamic_cast<const LinearFVPressureInletOutletVelocityBC *>(bc_pointer))
      return true;

  return false;
}

Real
SharpInterfaceRhieChowMassFlux::pressureBoundaryTargetFlux(const FaceInfo * fi,
                                                           const Moose::StateArg & time_arg) const
{
  if (!fi || _vel[0]->isInternalFace(*fi))
    return 0.0;

  const bool elem_is_fluid = hasBlocks(fi->elemPtr()->subdomain_id());
  const Real boundary_normal_multiplier = elem_is_fluid ? 1.0 : -1.0;
  const RealVectorValue predictor_face_velocity = referenceFaceVelocityState(fi, time_arg);
  return boundary_normal_multiplier * (predictor_face_velocity * fi->normal());
}

Real
SharpInterfaceRhieChowMassFlux::pressureBoundaryNormalAinv(const FaceInfo * fi) const
{
  if (!fi)
    return 0.0;

  return computeFaceNormalRawAinv(interpolateFaceRawAinv(fi), fi->normal());
}

void
SharpInterfaceRhieChowMassFlux::updateVelocityBoundaryState()
{
  RhieChowMassFlux::updateVelocityBoundaryState();

  if (!_corrected_face_velocity_valid)
    return;

  const auto time_arg = Moose::currentState();

  for (const auto * fi : flowFaceInfo())
  {
    if (!shouldUseCorrectedBoundaryVelocityState(fi))
      continue;

    const bool elem_is_fluid = hasBlocks(fi->elemPtr()->subdomain_id());
    const Elem * const boundary_elem = elem_is_fluid ? fi->elemPtr() : fi->neighborPtr();
    const Real boundary_normal_multiplier = elem_is_fluid ? 1.0 : -1.0;
    const Moose::FaceArg boundary_face{
        fi, Moose::FV::LimiterType::CentralDifference, true, false, boundary_elem, nullptr};
    const Real face_rho = _rho(boundary_face, time_arg);
    const RealVectorValue corrected_face_velocity =
        libmesh_map_find(_corrected_face_velocity, fi->id());

    RealVectorValue density_times_velocity;
    for (const auto component : make_range(_dim))
    {
      _boundary_velocity_face_values[component][fi->id()] = corrected_face_velocity(component);
      density_times_velocity(component) =
          boundary_normal_multiplier * face_rho * corrected_face_velocity(component);
    }

    _face_mass_flux[fi->id()] = density_times_velocity * fi->normal();
  }

  cacheCurrentCorrectedVolumetricFlux();
  _velocity_boundary_state_valid = true;
}

RealVectorValue
SharpInterfaceRhieChowMassFlux::reconstructFaceNormalScalarToCellVector(
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

    const Real weight = std::max(fi_loc->faceArea() * fi_loc->faceCoord(), libMesh::TOLERANCE);
    const Real orientation = fi_loc->elemPtr() == elem ? 1.0 : -1.0;
    const RealVectorValue outward_normal = orientation * fi_loc->normal();
    const Real psi_f = orientation * scalar_field(makeCenteredFaceArg(fi_loc), time_arg);

    for (const auto i : make_range(_dim))
    {
      rhs(i) += weight * outward_normal(i) * psi_f;
      for (const auto j : make_range(_dim))
        normal_matrix(i, j) += weight * outward_normal(i) * outward_normal(j);
    }
  }

  if (_dim == 1)
  {
    if (std::abs(normal_matrix(0, 0)) <= libMesh::TOLERANCE)
      return RealVectorValue();
  }
  else if (_dim == 2)
  {
    if (std::abs(normal_matrix(0, 0) * normal_matrix(1, 1) -
                 normal_matrix(0, 1) * normal_matrix(1, 0)) <= libMesh::TOLERANCE)
      return RealVectorValue();
  }
  else if (_dim == 3)
  {
    const Real det =
        normal_matrix(0, 0) * (normal_matrix(1, 1) * normal_matrix(2, 2) -
                               normal_matrix(1, 2) * normal_matrix(2, 1)) -
        normal_matrix(0, 1) * (normal_matrix(1, 0) * normal_matrix(2, 2) -
                               normal_matrix(1, 2) * normal_matrix(2, 0)) +
        normal_matrix(0, 2) * (normal_matrix(1, 0) * normal_matrix(2, 1) -
                               normal_matrix(1, 1) * normal_matrix(2, 0));
    if (std::abs(det) <= libMesh::TOLERANCE)
      return RealVectorValue();
  }

  normal_matrix.lu_solve(rhs, solution);

  RealVectorValue reconstructed_source;
  for (const auto i : make_range(_dim))
    reconstructed_source(i) = solution(i);

  return reconstructed_source;
}

RealVectorValue
SharpInterfaceRhieChowMassFlux::reconstructFixedFaceNormalScalarToCellVector(
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

    const Real weight = std::max(fi_loc->faceArea() * fi_loc->faceCoord(), libMesh::TOLERANCE);
    const RealVectorValue face_normal = fi_loc->normal();
    const Real psi_f = scalar_field(makeCenteredFaceArg(fi_loc), time_arg);

    for (const auto i : make_range(_dim))
    {
      rhs(i) += weight * face_normal(i) * psi_f;
      for (const auto j : make_range(_dim))
        normal_matrix(i, j) += weight * face_normal(i) * face_normal(j);
    }
  }

  if (_dim == 1)
  {
    if (std::abs(normal_matrix(0, 0)) <= libMesh::TOLERANCE)
      return RealVectorValue();
  }
  else if (_dim == 2)
  {
    if (std::abs(normal_matrix(0, 0) * normal_matrix(1, 1) -
                 normal_matrix(0, 1) * normal_matrix(1, 0)) <= libMesh::TOLERANCE)
      return RealVectorValue();
  }
  else if (_dim == 3)
  {
    const Real det =
        normal_matrix(0, 0) * (normal_matrix(1, 1) * normal_matrix(2, 2) -
                               normal_matrix(1, 2) * normal_matrix(2, 1)) -
        normal_matrix(0, 1) * (normal_matrix(1, 0) * normal_matrix(2, 2) -
                               normal_matrix(1, 2) * normal_matrix(2, 0)) +
        normal_matrix(0, 2) * (normal_matrix(1, 0) * normal_matrix(2, 1) -
                               normal_matrix(1, 1) * normal_matrix(2, 0));
    if (std::abs(det) <= libMesh::TOLERANCE)
      return RealVectorValue();
  }

  normal_matrix.lu_solve(rhs, solution);

  RealVectorValue reconstructed_source;
  for (const auto i : make_range(_dim))
    reconstructed_source(i) = solution(i);

  return reconstructed_source;
}

RealVectorValue
SharpInterfaceRhieChowMassFlux::reconstructMatchedPressureCoupledCellCorrectionSource(
    const ElemInfo * elem_info, const Moose::StateArg & time_arg) const
{
  if (!elem_info || !_pressure_coupled_velocity_correction_valid)
    return RealVectorValue();
  if (!_corrected_face_velocity_valid)
    const_cast<SharpInterfaceRhieChowMassFlux *>(this)->updateCorrectedFaceVelocityField(time_arg);

  return MetaPhysicL::raw_value(_corrected_face_velocity(makeElemArg(elem_info->elem()), time_arg));
}

RealVectorValue
SharpInterfaceRhieChowMassFlux::reconstructPressureCoupledCellCorrectionSource(
    const ElemInfo * elem_info, const Moose::StateArg & time_arg) const
{
  return reconstructMatchedPressureCoupledCellCorrectionSource(elem_info, time_arg);
}

RealVectorValue
SharpInterfaceRhieChowMassFlux::matchedSourcePressureCoupledCellVelocityDelta(
    const ElemInfo * elem_info, const Moose::StateArg & time_arg) const
{
  if (!elem_info || !_pressure_coupled_velocity_correction_valid)
    return RealVectorValue();
  return reconstructPressureCoupledCellVelocityDelta(elem_info, time_arg);
}

RealVectorValue
SharpInterfaceRhieChowMassFlux::reconstructPressureCoupledCellVelocityDelta(
    const ElemInfo * elem_info, const Moose::StateArg & time_arg) const
{
  if (!elem_info || !_pressure_coupled_velocity_correction_valid)
    return RealVectorValue();
  if (!_corrected_face_velocity_valid)
    const_cast<SharpInterfaceRhieChowMassFlux *>(this)->updateCorrectedFaceVelocityField(time_arg);

  const RealVectorValue corrected_cell_velocity =
      MetaPhysicL::raw_value(_corrected_face_velocity(makeElemArg(elem_info->elem()), time_arg));

  RealVectorValue delta_velocity;
  for (const auto dim_i : make_range(_dim))
    delta_velocity(dim_i) =
        corrected_cell_velocity(dim_i) - predictorVelocityComponent(*elem_info, dim_i);

  return delta_velocity;
}

void
SharpInterfaceRhieChowMassFlux::applyAdditionalFaceMassFluxCorrection()
{
  // computeFaceMassFlux() now uses the explicit phiHbyA face state, which in the
  // sharp-interface path already includes the transient and capillary/hydrostatic
  // predictor-source fluxes.
  // There is therefore no additional post-solve face-flux correction to apply here.
}

void
SharpInterfaceRhieChowMassFlux::computeProvisionalCellVelocity()
{
  const auto time_arg = Moose::currentState();
  updatePressureCoupledVelocityCorrectionFaceField(time_arg);

  _corrected_face_velocity_valid = false;
  updateCorrectedFaceVelocityField(time_arg);

  for (const auto & elem_info : _fe_problem.mesh().elemInfoVector())
  {
    if (!hasBlocks(elem_info->subdomain_id()))
      continue;

    // Keep the corrected face state authoritative. The cell velocity is derived
    // afterward from the corrected face-velocity functor and is no longer the
    // primary carrier of the pressure-correction writeback.
    const RealVectorValue corrected_cell_velocity =
        MetaPhysicL::raw_value(_corrected_face_velocity(makeElemArg(elem_info->elem()), time_arg));

    for (const auto system_i : index_range(_momentum_implicit_systems))
    {
      auto & solution = *(_momentum_implicit_systems[system_i]->solution);
      const auto dof = elem_info->dofIndices()[_global_momentum_system_numbers[system_i]][0];
      solution.set(dof, corrected_cell_velocity(system_i));
    }
  }

  for (const auto system_i : index_range(_momentum_implicit_systems))
  {
    auto & solution = *(_momentum_implicit_systems[system_i]->solution);
    solution.close();
    _momentum_implicit_systems[system_i]->update();
    _momentum_systems[system_i]->setSolution(
        *_momentum_implicit_systems[system_i]->current_local_solution);
  }
}

void
SharpInterfaceRhieChowMassFlux::computeCellVelocity()
{
  computeProvisionalCellVelocity();
}
