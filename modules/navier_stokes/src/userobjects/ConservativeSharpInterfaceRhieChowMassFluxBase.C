//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConservativeSharpInterfaceRhieChowMassFluxBase.h"

#include "LinearFVAdvectionDiffusionBC.h"
#include "LinearFVPressureInletOutletMomentumBC.h"
#include "LinearSystem.h"
#include "MooseFunctorArguments.h"
#include "MooseMesh.h"
#include "PIMPLE.h"
#include "SIMPLE.h"
#include "SubProblem.h"
#include "FVUtils.h"
#include "libmesh/dense_matrix.h"
#include "libmesh/dense_vector.h"
#include "libmesh/petsc_matrix.h"
#include "libmesh/petsc_vector.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <map>
#include <unordered_map>
#include <vector>

namespace
{
constexpr Real pressure_writeback_face_ainv_relative_tolerance = 1e-4;
constexpr Real surface_tension_normal_regularization = 1e-14;

const FaceInfo *
faceInfoForElementSide(const MooseMesh & moose_mesh,
                       const Elem * const elem,
                       const unsigned int side)
{
  if (!elem)
    return nullptr;

  const Elem * const loc_neighbor = elem->neighbor_ptr(side);
  if (loc_neighbor == libMesh::remote_elem)
    return nullptr;

  const bool elem_has_fi = Moose::FV::elemHasFaceInfo(*elem, loc_neighbor);
  if (!elem_has_fi && !loc_neighbor)
    return nullptr;

  return moose_mesh.faceInfo(elem_has_fi ? elem : loc_neighbor,
                             elem_has_fi ? side : loc_neighbor->which_neighbor_am_i(elem));
}

template <typename FaceNormalScalar>
RealVectorValue
reconstructCellVectorFromFaceNormalScalars(const MooseMesh & moose_mesh,
                                           const unsigned int dim,
                                           const ElemInfo * elem_info,
                                           FaceNormalScalar face_normal_scalar)
{
  if (!elem_info)
    return RealVectorValue();

  const Elem * const elem = elem_info->elem();
  if (!elem)
    return RealVectorValue();

  DenseMatrix<Real> normal_matrix(dim, dim);
  DenseVector<Real> rhs(dim);
  DenseVector<Real> solution(dim);

  for (const auto i : make_range(dim))
  {
    rhs(i) = 0.0;
    solution(i) = 0.0;
    for (const auto j : make_range(dim))
      normal_matrix(i, j) = 0.0;
  }

  for (const auto side : make_range(elem->n_sides()))
  {
    const Elem * const loc_neighbor = elem->neighbor_ptr(side);
    const bool elem_has_fi = Moose::FV::elemHasFaceInfo(*elem, loc_neighbor);
    if (!elem_has_fi && !loc_neighbor)
      continue;

    const FaceInfo * const fi_loc =
        moose_mesh.faceInfo(elem_has_fi ? elem : loc_neighbor,
                            elem_has_fi ? side : loc_neighbor->which_neighbor_am_i(elem));
    if (!fi_loc)
      continue;

    const Real face_measure = fi_loc->faceArea() * fi_loc->faceCoord();
    if (face_measure <= 0.0)
      continue;

    const Real orientation = fi_loc->elemPtr() == elem ? 1.0 : -1.0;
    Real normal_scalar = 0.0;
    if (!face_normal_scalar(*fi_loc, orientation, normal_scalar))
      continue;

    const RealVectorValue outward_normal = orientation * fi_loc->normal();
    const Real psi_f = orientation * normal_scalar;
    for (const auto i : make_range(dim))
    {
      rhs(i) += face_measure * outward_normal(i) * psi_f;
      for (const auto j : make_range(dim))
        normal_matrix(i, j) += face_measure * outward_normal(i) * outward_normal(j);
    }
  }

  bool singular = false;
  if (dim == 1)
    singular = std::abs(normal_matrix(0, 0)) <= libMesh::TOLERANCE;
  else if (dim == 2)
    singular = std::abs(normal_matrix(0, 0) * normal_matrix(1, 1) -
                        normal_matrix(0, 1) * normal_matrix(1, 0)) <= libMesh::TOLERANCE;
  else if (dim == 3)
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
  for (const auto i : make_range(dim))
    reconstructed_source(i) = solution(i);

  return reconstructed_source;
}
}

InputParameters
ConservativeSharpInterfaceRhieChowMassFluxBase::validParams()
{
  InputParameters params = RhieChowMassFlux::validParams();

  params.addClassDescription(
      "Rhie-Chow face-flux provider with additional reduced-pressure predictor functors for "
      "large-density-ratio sharp-interface coupling.");

  params.addParam<MooseFunctorName>(
      "vof_rho_phi_functor",
      "rho_phi",
      "Optional alpha-consistent density flux published by the sharp-interface VOF transport. "
      "When present, downstream advection queries use this face mass flux instead of the raw "
      "Rhie-Chow density interpolation.");
  params.addParam<RealVectorValue>(
      "gravity",
      RealVectorValue(),
      "Gravity vector used by the reduced-pressure hydrostatic pressure initializer.");
  params.addParam<Point>(
      "reference_pressure_point",
      Point(0, 0, 0),
      "Reference point used to compute the reduced-pressure head gh = g.(x-x_ref).");
  params.addParam<MooseFunctorName>(
      "surface_tension_coefficient",
      "",
      "Optional surface-tension coefficient functor. When supplied together with "
      "surface_tension_volume_fraction_functor, the sharp-interface pressure coupling includes "
      "the least-squares CSF capillary force sigma*kappa*grad(alpha).");
  params.addParam<MooseFunctorName>("surface_tension_volume_fraction_functor",
                                    "",
                                    "Volume-fraction functor used to compute the least-squares "
                                    "alpha gradient and curvature for surface tension.");
  params.set<MooseEnum>("pressure_projection_method") =
      MooseEnum("standard consistent", "consistent");

  return params;
}

ConservativeSharpInterfaceRhieChowMassFluxBase::ConservativeSharpInterfaceRhieChowMassFluxBase(
    const InputParameters & params)
  : RhieChowMassFlux(params),
    _corrected_face_phi(_moose_mesh, blockIDs(), "corrected_face_phi"),
    _previous_timestep_corrected_face_phi(
        _moose_mesh, blockIDs(), "previous_timestep_corrected_face_phi"),
    _vof_transport_phi(_moose_mesh, blockIDs(), "vof_transport_phi"),
    _pressure_coupled_cell_reconstruction_scalar(
        _moose_mesh, blockIDs(), "pressure_coupled_cell_reconstruction_scalar"),
    _gravity(getParam<RealVectorValue>("gravity")),
    _reference_pressure_point(getParam<Point>("reference_pressure_point")),
    _vof_rho_phi_name(getParam<MooseFunctorName>("vof_rho_phi_functor")),
    _surface_tension_coefficient_name(getParam<MooseFunctorName>("surface_tension_coefficient")),
    _surface_tension_volume_fraction_name(
        getParam<MooseFunctorName>("surface_tension_volume_fraction_functor")),
    _vof_rho_phi(nullptr)
{
  for (const auto tid : make_range(libMesh::n_threads()))
  {
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

  initializeAdditionalPressureFluxStorage();
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::getMassFlux(const FaceInfo & fi) const
{
  const Real face_measure = faceMeasure(fi);
  return face_measure > 0.0 ? vofRhoPhiIntegrated(fi) / face_measure : 0.0;
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
ConservativeSharpInterfaceRhieChowMassFluxBase::vofRhoPhiIntegrated(const FaceInfo & fi) const
{
  return evaluateFaceScalarFunctor(_vof_rho_phi, &fi, Moose::currentState(), nullptr);
}

RealVectorValue
ConservativeSharpInterfaceRhieChowMassFluxBase::reducedPressureMomentumPredictorForceDensity(
    const ElemInfo & elem_info, const Moose::StateArg & time_arg) const
{
  if (!hasBlocks(elem_info.subdomain_id()))
    return RealVectorValue();

  RealVectorValue force_density = reconstructCellVectorFromFaceNormalScalars(
      _moose_mesh,
      _dim,
      &elem_info,
      [this, &time_arg](const FaceInfo & fi_loc, const Real, Real & normal_force_density)
      {
        if (!_vel[0]->isInternalFace(fi_loc))
          return false;

        normal_force_density = -computeFaceNormalPressureGradient(&fi_loc, time_arg);
        if (useExplicitHydrostaticPredictorForce())
          normal_force_density +=
              computeDiscreteHydrostaticPredictorFaceNormalForceDensity(&fi_loc, time_arg);

        return true;
      });

  force_density += computeLeastSquaresCellSurfaceTensionForceDensity(elem_info, time_arg);

  return force_density;
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

void
ConservativeSharpInterfaceRhieChowMassFluxBase::initializeAdditionalPressureFluxStorage(
    const bool preserve_corrected_face_phi)
{
  for (const auto * fi : _fe_problem.mesh().faceInfo())
  {
    if (!preserve_corrected_face_phi)
      _corrected_face_phi[fi->id()] = 0.0;
    _previous_timestep_corrected_face_phi[fi->id()] = 0.0;
    _vof_transport_phi[fi->id()] = 0.0;
    _pressure_coupled_cell_reconstruction_scalar[fi->id()] = 0.0;
    _pressure_predictor_flux[fi->id()] = 0.0;
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
ConservativeSharpInterfaceRhieChowMassFluxBase::pressureBoundaryTargetFlux(
    const FaceInfo * fi, const Moose::StateArg & time_arg) const
{
  if (!fi)
    return 0.0;

  return boundaryVolumetricFluxTarget(fi, time_arg) * faceMeasure(*fi);
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

  return computeFaceNormalAinv(it->second, fi->normal());
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
  return computeFaceNormalAinv(pressure_face_rau, fi->normal());
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
    const Real face_measure = faceMeasure(*fi);
    auto integratedToVolumetricPhi = [face_measure](const Real integrated_phi)
    { return face_measure > libMesh::TOLERANCE ? integrated_phi / face_measure : 0.0; };

    // The sharp pressure equation stores area-integrated phi. Convert back to the
    // normal-flux-density transport convention only at the publication/writeback boundary.
    state.predictor_transport_phi =
        integratedToVolumetricPhi(-libmesh_map_find(_phiHbyA_flux, fi->id()));
    state.pressure_equation_phi =
        _pressure_equation_flux_valid
            ? integratedToVolumetricPhi(libmesh_map_find(_pressure_equation_flux, fi->id()))
            : 0.0;
    const Real phig_phi = integratedToVolumetricPhi(libmesh_map_find(_phig_flux, fi->id()));
    state.pressure_writeback_phi =
        _pressure_equation_flux_valid ? state.pressure_equation_phi + phig_phi : 0.0;
    state.corrected_transport_phi = state.predictor_transport_phi + state.pressure_equation_phi;

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
    // Sharp-interface pressure-correction contract: the corrected transport flux is the
    // predictor flux plus the pressure-equation flux. This face flux is authoritative for
    // continuity/transport. The reconstructed cell velocity below is only the cell-centered
    // velocity writeback branch and is not projected back to overwrite this face flux.
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
}

void
ConservativeSharpInterfaceRhieChowMassFluxBase::adoptPublishedVOFTransportState()
{
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
ConservativeSharpInterfaceRhieChowMassFluxBase::meshChanged()
{
  RhieChowMassFlux::meshChanged();
  initializeAdditionalPressureFluxStorage();
}

void
ConservativeSharpInterfaceRhieChowMassFluxBase::initialSetup()
{
  const_cast<MooseLinearVariableFVReal *>(_p)->computeCellGradients();
  RhieChowMassFlux::initialSetup();
  initializeAdditionalPressureFluxStorage();
  if (!_vof_rho_phi_name.empty() &&
      UserObject::_subproblem.hasFunctorWithType<Real>(_vof_rho_phi_name, _tid))
    _vof_rho_phi = &getFunctor<Real>(_vof_rho_phi_name);

  if (!_vof_rho_phi)
    paramError("vof_rho_phi_functor",
               "The conservative sharp-interface path requires the VOF-owned rhoPhi functor.");

  const bool any_surface_tension_param =
      !_surface_tension_coefficient_name.empty() || !_surface_tension_volume_fraction_name.empty();
  const bool all_surface_tension_params =
      !_surface_tension_coefficient_name.empty() && !_surface_tension_volume_fraction_name.empty();
  if (any_surface_tension_param && !all_surface_tension_params)
    paramError("surface_tension_coefficient",
               "Surface tension requires both surface_tension_coefficient and "
               "surface_tension_volume_fraction_functor.");

  if (all_surface_tension_params)
  {
    _surface_tension_coefficient = &getFunctor<Real>(_surface_tension_coefficient_name);
    _surface_tension_volume_fraction = &getFunctor<Real>(_surface_tension_volume_fraction_name);
  }
}

void
ConservativeSharpInterfaceRhieChowMassFluxBase::initialize()
{
  RhieChowMassFlux::initialize();

  initializeAdditionalPressureFluxStorage(/*preserve_corrected_face_phi=*/true);
  if (!_corrected_face_phi_seeded)
    cacheCurrentCorrectedVolumetricFlux();

  for (const auto * fi : flowFaceInfo())
    _previous_timestep_corrected_face_phi[fi->id()] =
        libmesh_map_find(_corrected_face_phi, fi->id());
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
               pressure_writeback_face_ainv_relative_tolerance * max_abs_normal_pressure_ainv);
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
  for (const auto * fi : flowFaceInfo())
  {
    // Keep the pressure-corrected face flux as the source of truth. Do not reconstruct cell U and
    // project it back to faces here; that round trip is not an exact inverse.
    _face_mass_flux[fi->id()] = transportMassFluxDensityFromVolumetricPhi(
        fi, libmesh_map_find(_corrected_face_phi, fi->id()), time_arg);
  }
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::interpolateFaceDensity(
    const FaceInfo * fi, const Moose::StateArg & time_arg) const
{
  if (_vel[0]->isInternalFace(*fi))
    return evaluateFaceScalarFunctor(&_rho, fi, time_arg, nullptr);

  const ElemInfo & elem_info = boundaryElemInfo(fi);

  if (useConstrainedBoundaryPredictorState(fi))
    return _rho(makeCenteredFaceArg(fi, elem_info.elem()), time_arg);

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
    return Moose::FV::linearInterpolation(_vel[component]->getElemValue(elem_info, time_arg),
                                          _vel[component]->getElemValue(neighbor_info, time_arg),
                                          *fi,
                                          true);
  }

  return boundaryVelocityValue(fi, component, time_arg);
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
  return interpolateMomentumVectorReadersToFace(fi, raw_ainv_readers);
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
  state.face_raw_ainv = interpolateMomentumVectorReadersToFace(fi, raw_ainv_readers);
  state.normal_raw_ainv = computeFaceNormalAinv(state.face_raw_ainv, state.face_normal);
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

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::evaluateFaceScalarFunctor(
    const Moose::Functor<Real> * functor,
    const FaceInfo * fi,
    const Moose::StateArg & time_arg,
    const Moose::StateArg * limiter_state) const
{
  if (!functor)
    return 0.0;

  return MetaPhysicL::raw_value(
      (*functor)(makeCenteredFaceArg(fi, nullptr, limiter_state), time_arg));
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

  // Euler ddtCorr contribution: fvcDdtPhiCoeff(U.oldTime(), phi.oldTime(), phiCorr) * phiCorr / dt.
  // The pressure corrector adds interpolate(rho*rAU()) times that value to phiHbyA.
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
  const Real normal_raw_ainv = computeFaceNormalAinv(face_raw_ainv, face_normal);
  const Real normal_density_weighted_ainv =
      computeFaceNormalAinv(face_density_weighted_ainv, face_normal);

  if (std::abs(normal_density_weighted_ainv) <= libMesh::TOLERANCE)
  {
    const Real face_rho = predictorFaceDensity(fi, Moose::currentState());
    return std::abs(face_rho) > libMesh::TOLERANCE ? mass_flux_density / face_rho : 0.0;
  }

  return mass_flux_density * normal_raw_ainv / normal_density_weighted_ainv;
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::pressureDiffusionFaceArea(const FaceInfo * fi) const
{
  return fi ? faceMeasure(*fi) : 0.0;
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::computeFaceNormalDensityGradient(
    const FaceInfo * fi, const Moose::StateArg & time_arg) const
{
  return computeFaceNormalScalarGradient(_rho, fi, time_arg);
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::computeFaceNormalScalarGradient(
    const Moose::Functor<Real> & functor,
    const FaceInfo * fi,
    const Moose::StateArg & time_arg) const
{
  if (!fi)
    return 0.0;

  if (_vel[0]->isInternalFace(*fi))
  {
    const Real elem_value = functor(makeElemArg(fi->elemPtr()), time_arg);
    const Real neighbor_value = functor(makeElemArg(fi->neighborPtr()), time_arg);
    const Point delta = fi->dCN();
    const Real delta_mag = delta.norm();

    if (delta_mag <= libMesh::TOLERANCE)
      return 0.0;

    const Real value_jump = neighbor_value - elem_value;
    const Real non_orth_delta_coeff = 1.0 / std::max(fi->normal() * delta, 0.05 * delta_mag);
    const Real base_part = non_orth_delta_coeff * value_jump;

    const Point non_orth_correction_vector = fi->normal() - delta * non_orth_delta_coeff;
    const RealVectorValue elem_grad =
        MetaPhysicL::raw_value(functor.gradient(makeElemArg(fi->elemPtr()), time_arg));
    const RealVectorValue neighbor_grad =
        MetaPhysicL::raw_value(functor.gradient(makeElemArg(fi->neighborPtr()), time_arg));

    RealVectorValue face_grad = 0;
    Moose::FV::interpolate(
        Moose::FV::InterpMethod::Average, face_grad, elem_grad, neighbor_grad, *fi, true);
    return base_part + non_orth_correction_vector * face_grad;
  }

  // On uncoupled boundary patches the explicit non-orthogonal correction is zero and the
  // operative snGrad comes from the boundary patch field.
  return MetaPhysicL::raw_value(
             functor.gradient(makeCenteredFaceArg(fi, boundaryElemInfo(fi).elem()), time_arg)) *
         fi->normal();
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

  const ElemInfo & elem_info = boundaryElemInfo(fi);
  const Point fluid_centroid = elem_info.centroid();
  const Point cell_to_face = fi->faceCentroid() - fluid_centroid;
  const Real normal_spacing = std::abs(cell_to_face * fi->normal());

  if (normal_spacing <= libMesh::TOLERANCE)
    return 0.0;

  const Real elem_p = _p->getElemValue(elem_info, time_arg);

  if (auto * bc_pointer = pressureBoundaryCondition(fi))
  {
    const Real bc_sn_grad = bc_pointer->computeBoundaryNormalGradient();
    if (std::isfinite(bc_sn_grad))
      return bc_sn_grad;

    const Real bc_value = bc_pointer->computeBoundaryValue();
    if (std::isfinite(bc_value))
      return (bc_value - elem_p) / normal_spacing;
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

  return cell_grad_dot_normal(boundaryElemInfo(fi));
}

bool
ConservativeSharpInterfaceRhieChowMassFluxBase::useExplicitHydrostaticPredictorForce() const
{
  return !_suppress_startup_pressure_predictor_flux_sources &&
         !_suppress_explicit_hydrostatic_pressure_flux;
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::
    computeDiscreteHydrostaticPredictorFaceNormalForceDensity(
        const FaceInfo * fi, const Moose::StateArg & time_arg) const
{
  if (!fi || !useExplicitHydrostaticPredictorForce())
    return 0.0;

  const Real ghf = _gravity * (fi->faceCentroid() - _reference_pressure_point);
  return -ghf * computeFaceNormalDensityGradient(fi, time_arg);
}

bool
ConservativeSharpInterfaceRhieChowMassFluxBase::surfaceTensionEnabled() const
{
  return _surface_tension_coefficient && _surface_tension_volume_fraction;
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::computeLeastSquaresFaceScalarValue(
    const Moose::Functor<Real> & functor,
    const FaceInfo * fi,
    const Elem * elem,
    const Moose::StateArg & time_arg) const
{
  if (!fi || !elem)
    return 0.0;

  if (_vel[0]->isInternalFace(*fi))
  {
    const Real elem_value = functor(makeElemArg(fi->elemPtr()), time_arg);
    const Real neighbor_value = functor(makeElemArg(fi->neighborPtr()), time_arg);
    Real face_value = 0.0;
    Moose::FV::interpolate(
        Moose::FV::InterpMethod::Average, face_value, elem_value, neighbor_value, *fi, true);
    return face_value;
  }

  return functor(makeCenteredFaceArg(fi, elem), time_arg);
}

RealVectorValue
ConservativeSharpInterfaceRhieChowMassFluxBase::computeLeastSquaresCellGradient(
    const Moose::Functor<Real> & functor,
    const ElemInfo & elem_info,
    const Moose::StateArg & time_arg) const
{
  const Elem * const elem = elem_info.elem();
  if (!elem)
    return RealVectorValue();

  const Real cell_value = functor(makeElemArg(elem), time_arg);
  std::vector<Point> offsets;
  std::vector<Real> increments;
  offsets.reserve(elem->n_sides());
  increments.reserve(elem->n_sides());

  for (const auto side : make_range(elem->n_sides()))
  {
    const FaceInfo * const fi_loc = faceInfoForElementSide(_moose_mesh, elem, side);
    if (!fi_loc)
      continue;

    const Point offset = fi_loc->faceCentroid() - elem_info.centroid();
    if (offset.norm() <= libMesh::TOLERANCE)
      continue;

    offsets.push_back(offset);
    increments.push_back(computeLeastSquaresFaceScalarValue(functor, fi_loc, elem, time_arg) -
                         cell_value);
  }

  if (offsets.size() < _dim)
    return RealVectorValue();

  DenseMatrix<Real> matrix(offsets.size(), _dim);
  DenseVector<Real> rhs(offsets.size());
  DenseVector<Real> solution(_dim);

  for (const auto row : index_range(offsets))
  {
    rhs(row) = increments[row];
    for (const auto dim_i : make_range(_dim))
      matrix(row, dim_i) = offsets[row](dim_i);
  }

  matrix.svd_solve(rhs, solution);

  RealVectorValue gradient;
  for (const auto dim_i : make_range(_dim))
    gradient(dim_i) = solution(dim_i);

  return gradient;
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::computeLeastSquaresCellInterfaceCurvature(
    const ElemInfo & elem_info, const Moose::StateArg & time_arg) const
{
  if (!surfaceTensionEnabled())
    return 0.0;

  const Elem * const elem = elem_info.elem();
  if (!elem)
    return 0.0;

  const Real cell_volume = elem_info.volume() * elem_info.coordFactor();
  if (cell_volume <= libMesh::TOLERANCE)
    return 0.0;

  const RealVectorValue elem_gradient =
      computeLeastSquaresCellGradient(*_surface_tension_volume_fraction, elem_info, time_arg);

  Real interface_normal_flux = 0.0;
  for (const auto side : make_range(elem->n_sides()))
  {
    const FaceInfo * const fi_loc = faceInfoForElementSide(_moose_mesh, elem, side);
    if (!fi_loc)
      continue;

    RealVectorValue face_gradient = elem_gradient;
    if (_vel[0]->isInternalFace(*fi_loc))
    {
      const ElemInfo * const neighbor_info =
          fi_loc->elemPtr() == elem ? fi_loc->neighborInfo() : fi_loc->elemInfo();
      const RealVectorValue neighbor_gradient = computeLeastSquaresCellGradient(
          *_surface_tension_volume_fraction, *neighbor_info, time_arg);
      Moose::FV::interpolate(Moose::FV::InterpMethod::Average,
                             face_gradient,
                             elem_gradient,
                             neighbor_gradient,
                             *fi_loc,
                             fi_loc->elemPtr() == elem);
    }

    const Real gradient_norm = face_gradient.norm();
    const RealVectorValue interface_normal =
        face_gradient / (gradient_norm + surface_tension_normal_regularization);
    const Real orientation = fi_loc->elemPtr() == elem ? 1.0 : -1.0;
    const RealVectorValue outward_normal = orientation * fi_loc->normal();
    interface_normal_flux += faceMeasure(*fi_loc) * (interface_normal * outward_normal);
  }

  return -interface_normal_flux / cell_volume;
}

RealVectorValue
ConservativeSharpInterfaceRhieChowMassFluxBase::computeLeastSquaresCellSurfaceTensionForceDensity(
    const ElemInfo & elem_info, const Moose::StateArg & time_arg) const
{
  if (!surfaceTensionEnabled() || _suppress_startup_pressure_predictor_flux_sources)
    return RealVectorValue();

  const Elem * const elem = elem_info.elem();
  if (!elem)
    return RealVectorValue();

  const Real sigma = (*_surface_tension_coefficient)(makeElemArg(elem), time_arg);
  if (sigma == 0.0)
    return RealVectorValue();

  const RealVectorValue alpha_gradient =
      computeLeastSquaresCellGradient(*_surface_tension_volume_fraction, elem_info, time_arg);
  const Real curvature = computeLeastSquaresCellInterfaceCurvature(elem_info, time_arg);

  return sigma * curvature * alpha_gradient;
}

Real
ConservativeSharpInterfaceRhieChowMassFluxBase::
    computeLeastSquaresSurfaceTensionFaceNormalForceDensity(const FaceInfo * fi,
                                                            const Moose::StateArg & time_arg) const
{
  if (!fi || !surfaceTensionEnabled() || _suppress_startup_pressure_predictor_flux_sources)
    return 0.0;

  if (_vel[0]->isInternalFace(*fi))
  {
    const RealVectorValue elem_force =
        computeLeastSquaresCellSurfaceTensionForceDensity(*fi->elemInfo(), time_arg);
    const RealVectorValue neighbor_force =
        computeLeastSquaresCellSurfaceTensionForceDensity(*fi->neighborInfo(), time_arg);
    RealVectorValue face_force;
    Moose::FV::interpolate(
        Moose::FV::InterpMethod::Average, face_force, elem_force, neighbor_force, *fi, true);
    return face_force * fi->normal();
  }

  const RealVectorValue cell_force =
      computeLeastSquaresCellSurfaceTensionForceDensity(boundaryElemInfo(fi), time_arg);
  return cell_force * fi->normal();
}

void
ConservativeSharpInterfaceRhieChowMassFluxBase::updateAdditionalPressureFluxFunctors(
    const bool with_updated_pressure, const bool /*verbose*/)
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

  for (const auto * fi : flowFaceInfo())
  {
    const auto predictor_state =
        buildMomentumPredictorFaceState(fi,
                                        hbya_readers,
                                        nullptr,
                                        time_arg,
                                        /* include_boundary_body_force_correction = */ false,
                                        /* use_boundary_velocity_values = */ true);

    const Real face_measure = faceMeasure(*fi);
    const Real predictor_operator_volumetric_flux = predictor_state.face_hbya * fi->normal();
    const auto state =
        buildSharpFaceOperatorState(fi, time_arg, raw_ainv_readers, &pressure_gradient_readers);
    const RealVectorValue pressure_face_raw_ainv = state.face_raw_ainv;
    const Real normal_pressure_ainv =
        computeFaceNormalAinv(pressure_face_raw_ainv, state.face_normal);
    Real transient_projection_flux = 0.0;
    Real hydrostatic_flux = 0.0;
    Real surface_tension_flux = 0.0;

    if (!_suppress_startup_pressure_predictor_flux_sources)
      transient_projection_flux =
          -computeDefaultTransientProjectionVolumetricFlux(fi, time_arg, state);

    if (!_suppress_startup_pressure_predictor_flux_sources)
    {
      if (!_suppress_explicit_hydrostatic_pressure_flux)
      {
        const Real ghf = _gravity * (fi->faceCentroid() - _reference_pressure_point);
        const Real sn_grad_rho = computeFaceNormalDensityGradient(fi, time_arg);
        hydrostatic_flux = -ghf * sn_grad_rho * normal_pressure_ainv;
      }

      surface_tension_flux = computeLeastSquaresSurfaceTensionFaceNormalForceDensity(fi, time_arg) *
                             normal_pressure_ainv;
    }

    const Real phig_flux = (hydrostatic_flux + surface_tension_flux) * face_measure;
    const Real pressure_predictor_base_flux =
        (predictor_operator_volumetric_flux + transient_projection_flux) * face_measure;

    _phig_flux[fi->id()] = phig_flux;
    _pressure_Ainv[fi->id()] = pressure_face_raw_ainv;
    _pressure_predictor_base_flux[fi->id()] = -pressure_predictor_base_flux;
    _phiHbyA_flux[fi->id()] = -(pressure_predictor_base_flux + phig_flux);
    _pressure_predictor_flux[fi->id()] = _phiHbyA_flux[fi->id()];
  }

  _pressure_predictor_face_state_valid = true;
}

void
ConservativeSharpInterfaceRhieChowMassFluxBase::updatePressureCoupledVelocityCorrectionFaceField(
    const Moose::StateArg & time_arg)
{
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
               pressure_writeback_face_ainv_relative_tolerance * max_abs_normal_pressure_ainv);

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
  if (!use_constrained_boundary_state)
  {
    for (const auto dim_i : make_range(_dim))
      if (auto * bc_pointer = velocityBoundaryCondition(fi, dim_i))
      {
        if (dynamic_cast<LinearFVPressureInletOutletMomentumBC *>(bc_pointer))
          continue;

        if (auto * adv_diff_bc = dynamic_cast<LinearFVAdvectionDiffusionBC *>(bc_pointer))
        {
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
ConservativeSharpInterfaceRhieChowMassFluxBase::postUpdateVelocityBoundaryState()
{
  cacheCurrentCorrectedVolumetricFlux();
}

RealVectorValue
ConservativeSharpInterfaceRhieChowMassFluxBase::reconstructFaceNormalScalarToCellVector(
    const ElemInfo * elem_info,
    const Moose::StateArg & time_arg,
    const FaceScalarField & scalar_field) const
{
  return reconstructCellVectorFromFaceNormalScalars(
      _moose_mesh,
      _dim,
      elem_info,
      [this, &scalar_field, &time_arg](const FaceInfo & fi_loc, const Real, Real & normal_scalar)
      {
        normal_scalar = scalar_field(makeCenteredFaceArg(&fi_loc), time_arg);
        return true;
      });
}

RealVectorValue
ConservativeSharpInterfaceRhieChowMassFluxBase::reconstructBasePressureCoupledCellVelocityDelta(
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
  if (!elem_info || !_pressure_coupled_velocity_correction_valid)
    return RealVectorValue();

  return reconstructBasePressureCoupledCellVelocityDelta(elem_info, time_arg);
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
