//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConservativeDiffuseInterfaceRhieChowMassFlux.h"
#include "TimeIntegrator.h"

registerMooseObject("NavierStokesApp", ConservativeDiffuseInterfaceRhieChowMassFlux);
#include "NSFVLeastSquares.h"

#include "LinearFVAdvectionDiffusionBC.h"
#include "LinearFVPressureInletOutletVelocityBC.h"
#include "LinearSystem.h"
#include "MooseFunctorArguments.h"
#include "MooseMesh.h"
#include "SubProblem.h"
#include "FVUtils.h"
#include "libmesh/dense_matrix.h"
#include "libmesh/dense_vector.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <unordered_map>
#include <vector>

namespace
{
constexpr Real pressure_writeback_face_ainv_relative_tolerance = 1e-4;

/// The value stored for a face, or zero for a face the field has not reached yet. The flux
/// fields are read during initialisation before they are filled, where the stored fields they
/// replaced read as zero.
Real
faceValueOrZero(const std::unordered_map<dof_id_type, Real> & field, const FaceInfo & fi)
{
  const auto it = field.find(fi.id());
  return it == field.end() ? 0.0 : it->second;
}

/// Least-squares reconstruction of a cell vector from its face-normal components, given by
/// \p face_normal_scalar(fi) in the orientation of the face normal: the vector v minimizing
/// sum_f S_f (v.n_f - psi_f)^2 over the cell's faces, which is the solution of the normal
/// equations (sum_f S_f n_f n_f^T) v = sum_f S_f n_f psi_f. On a Cartesian cell this reduces to
/// averaging the two opposite faces of each direction. It turns the face-normal pressure
/// gradient, face forces and face flux corrections into the cell vectors the momentum equation
/// needs, and is the only place a face quantity becomes a cell quantity in this object.
template <typename FaceNormalScalar>
RealVectorValue
reconstructCellVectorFromFaceNormalScalars(const MooseMesh & moose_mesh,
                                           const unsigned int dim,
                                           const ElemInfo & elem_info,
                                           FaceNormalScalar face_normal_scalar)
{
  const Elem * const elem = elem_info.elem();
  if (!elem)
    return RealVectorValue();

  DenseMatrix<Real> normal_matrix(dim, dim);
  DenseVector<Real> rhs(dim);
  DenseVector<Real> solution(dim);

  for (const auto side : make_range(elem->n_sides()))
  {
    const FaceInfo * const fi_loc = NS::faceInfoForElementSide(moose_mesh, elem, side);
    if (!fi_loc)
      continue;

    const Real face_measure = NS::faceMeasure(*fi_loc);
    if (face_measure <= 0.0)
      continue;

    const Real orientation = fi_loc->elemPtr() == elem ? 1.0 : -1.0;
    const RealVectorValue outward_normal = orientation * fi_loc->normal();
    const Real psi_f = orientation * face_normal_scalar(*fi_loc);
    for (const auto i : make_range(dim))
    {
      rhs(i) += face_measure * outward_normal(i) * psi_f;
      for (const auto j : make_range(dim))
        normal_matrix(i, j) += face_measure * outward_normal(i) * outward_normal(j);
    }
  }

  Real det = 0.0;
  if (dim == 1)
    det = normal_matrix(0, 0);
  else if (dim == 2)
    det = normal_matrix(0, 0) * normal_matrix(1, 1) - normal_matrix(0, 1) * normal_matrix(1, 0);
  else if (dim == 3)
    det = normal_matrix(0, 0) * (normal_matrix(1, 1) * normal_matrix(2, 2) -
                                 normal_matrix(1, 2) * normal_matrix(2, 1)) -
          normal_matrix(0, 1) * (normal_matrix(1, 0) * normal_matrix(2, 2) -
                                 normal_matrix(1, 2) * normal_matrix(2, 0)) +
          normal_matrix(0, 2) * (normal_matrix(1, 0) * normal_matrix(2, 1) -
                                 normal_matrix(1, 1) * normal_matrix(2, 0));

  // The determinant scales as (face measure)^dim, which depends on the cell size and, in RZ, on
  // the radius, so it is compared with the same power of the mean diagonal entry. The ratio is one
  // for a Cartesian cell and vanishes only when the face normals fail to span the space.
  Real trace = 0.0;
  for (const auto i : make_range(dim))
    trace += normal_matrix(i, i);
  if (!(trace > 0.0) || std::abs(det) <= libMesh::TOLERANCE * std::pow(trace / dim, dim))
    return RealVectorValue();

  normal_matrix.lu_solve(rhs, solution);

  RealVectorValue reconstructed_source;
  for (const auto i : make_range(dim))
    reconstructed_source(i) = solution(i);

  return reconstructed_source;
}
}

/**
 * The parts of one velocity component's assembled momentum equation A u = H(u) + b that the
 * pressure coupling needs separately. A is the diagonal, and H(u) the
 * right-hand side minus the off-diagonal products; both are rebuilt here from the matrix as
 * assembled, before the solve relaxes it.
 */
struct ConservativeDiffuseInterfaceRhieChowMassFlux::MomentumPredictorOperator
{
  /// The assembled right-hand side b, without the explicit reduced-pressure forces, which enter
  /// the pressure equation as face forces
  std::unique_ptr<NumericVector<Number>> rhs_raw;
  /// The diagonal A the pressure equation's 1/A is built from: the assembled one made
  /// diagonally dominant, see momentumPredictorAssembled
  std::unique_ptr<NumericVector<Number>> pressure_coupling_diagonal_raw;
  /// Assembled diagonal minus the dominant one, added into H so that A u - H stays the
  /// assembled equation
  std::unique_ptr<NumericVector<Number>> h_diagonal_correction_raw;
};

void
ConservativeDiffuseInterfaceRhieChowMassFlux::beginMomentumPredictorAssembly(
    const unsigned int system_i)
{
  // The first component starts a new vector operator.
  if (system_i == 0)
    updateFaceNormalDensityGradients();

  mooseAssert(system_i < _momentum_implicit_systems.size() && _momentum_implicit_systems[system_i],
              "The requested momentum component is not linked to RhieChowMassFlux.");

  _momentum_predictor_operators.resize(_momentum_systems.size());

  // The operator's vectors are allocated once and reused by every later predictor:
  // momentumPredictorAssembled overwrites each of them before use. meshChanged drops them, so
  // they are reallocated with the new layout.
  auto & predictor = _momentum_predictor_operators[system_i];
  if (!predictor)
  {
    const auto & current_local_solution =
        *(_momentum_implicit_systems[system_i]->current_local_solution);
    predictor = std::make_unique<MomentumPredictorOperator>();
    for (auto * vector : {&predictor->rhs_raw,
                          &predictor->pressure_coupling_diagonal_raw,
                          &predictor->h_diagonal_correction_raw})
      *vector = current_local_solution.zero_clone();
  }

  _predictor_ready = false;
}

void
ConservativeDiffuseInterfaceRhieChowMassFlux::momentumPredictorAssembled(
    const unsigned int system_i,
    SparseMatrix<Number> & matrix,
    const NumericVector<Number> & rhs,
    const Real /*relaxation_parameter*/)
{
  mooseAssert(system_i < _momentum_predictor_operators.size() &&
                  _momentum_predictor_operators[system_i],
              "A split momentum predictor operator assembly was not started.");

  auto & predictor = *_momentum_predictor_operators[system_i];

  *predictor.rhs_raw = rhs;
  predictor.rhs_raw->close();

  // Read the operator now: relaxation divides the matrix diagonal by the relaxation factor
  // afterwards, and the pressure coupling uses it unrelaxed. With the relaxed one, 1/A and with it
  // the Rhie-Chow dissipation scale with the relaxation factor, and the converged step depends on
  // that factor (Majumdar 1988).
  //
  // Each component keeps its own boundary diagonal. fvMatrix averages the boundary coefficient
  // over the components because its 1/A is one scalar field; 1/A is per component here, and an
  // averaged coefficient left the outlet cells, where the normal and tangential velocity
  // boundary conditions differ, unable to satisfy the momentum equation at the fixed point.
  const auto num_local_rows = matrix.row_stop() - matrix.row_start();
  std::vector<dof_id_type> rows;
  std::vector<Real> pressure_diagonal;
  std::vector<Real> h_diagonal_correction;
  rows.reserve(num_local_rows);
  pressure_diagonal.reserve(num_local_rows);
  h_diagonal_correction.reserve(num_local_rows);

  std::vector<numeric_index_type> column_dofs;
  std::vector<Real> coefficients;
  for (const auto row_dof : make_range(matrix.row_start(), matrix.row_stop()))
  {
    matrix.get_row(row_dof, column_dofs, coefficients);

    Real diagonal = 0.0;
    Real offdiag_abs_sum = 0.0;
    for (const auto entry_i : index_range(column_dofs))
      if (column_dofs[entry_i] == row_dof)
        diagonal += coefficients[entry_i];
      else
        offdiag_abs_sum += std::abs(coefficients[entry_i]);

    // The diagonal is raised to the off-diagonal row sum where it is not dominant. H gets back the
    // difference so that the corrected velocity satisfies the assembled equation once the outer
    // loop has converged.
    const Real dominant_diagonal = std::max(std::abs(diagonal), offdiag_abs_sum);
    rows.push_back(row_dof);
    pressure_diagonal.push_back(dominant_diagonal);
    h_diagonal_correction.push_back(diagonal - dominant_diagonal);
  }

  predictor.pressure_coupling_diagonal_raw->insert(pressure_diagonal, rows);
  predictor.pressure_coupling_diagonal_raw->close();
  predictor.h_diagonal_correction_raw->insert(h_diagonal_correction, rows);
  predictor.h_diagonal_correction_raw->close();

  // The components are assembled in order, so the last one completes the vector operator.
  if (system_i + 1 == _momentum_systems.size())
  {
    mooseAssert(std::all_of(_momentum_predictor_operators.begin(),
                            _momentum_predictor_operators.end(),
                            [](const auto & op) { return static_cast<bool>(op); }),
                "Every momentum component must be assembled before the predictor is complete.");
    _predictor_ready = true;
  }
}

void
ConservativeDiffuseInterfaceRhieChowMassFlux::computePredictorOperator(const unsigned int system_i,
                                                                       NumericVector<Number> & HbyA,
                                                                       NumericVector<Number> & Ainv)
{
  mooseAssert(_predictor_ready,
              "The momentum predictor operator must be assembled and finalized before H/A.");
  mooseAssert(system_i < _momentum_predictor_operators.size() &&
                  _momentum_predictor_operators[system_i],
              "The momentum predictor operator is required before computing HbyA.");
  const auto & predictor = *_momentum_predictor_operators[system_i];

  computePredictorOperatorBase(system_i, HbyA, Ainv, predictor.rhs_raw.get());

  // H gets back the assembled diagonal minus the dominant one, times u, so that A u - H stays the
  // assembled equation.
  NumericVector<Number> & current_local_solution =
      *(_momentum_implicit_systems[system_i]->current_local_solution);
  auto h_correction = current_local_solution.zero_clone();
  h_correction->pointwise_mult(*predictor.h_diagonal_correction_raw, current_local_solution);
  h_correction->close();
  HbyA.add(1.0, *h_correction);

  // A: the unrelaxed, diagonally dominant diagonal of this component.
  Ainv = *predictor.pressure_coupling_diagonal_raw;
  Ainv.close();
}

Real
ConservativeDiffuseInterfaceRhieChowMassFlux::maxCourant(const Real dt) const
{
  if (dt <= 0.0)
    return 0.0;

  std::unordered_map<dof_id_type, Real> cell_flux_sum;
  cell_flux_sum.reserve(_flow_face_info.size());

  for (const auto * fi : _flow_face_info)
  {
    const Real face_measure = NS::faceMeasure(*fi);
    const Real face_volumetric_flux = std::abs(getVolumetricFaceFlux(*fi));
    const Real volumetric_flux = face_volumetric_flux * face_measure;

    if (fi->elemPtr() && hasBlocks(fi->elemPtr()->subdomain_id()))
      cell_flux_sum[fi->elemPtr()->id()] += volumetric_flux;

    if (fi->neighborPtr() && hasBlocks(fi->neighborPtr()->subdomain_id()))
      cell_flux_sum[fi->neighborPtr()->id()] += volumetric_flux;
  }

  // The face-flux Courant number: half the sum of the absolute face
  // fluxes of a cell, which is the flux through the cell for a divergence-free field, times dt
  // over the cell volume. The executioner sizes the volume-fraction subcycles with it.
  Real max_courant = 0.0;
  for (const auto & elem_info : _fe_problem.mesh().elemInfoVector())
  {
    if (!hasBlocks(elem_info->subdomain_id()))
      continue;

    const Real volume = NS::cellMeasure(*elem_info);
    if (volume <= 0.0)
      continue;

    const auto it = cell_flux_sum.find(elem_info->elem()->id());
    if (it == cell_flux_sum.end())
      continue;

    const Real courant = 0.5 * dt * it->second / volume;
    max_courant = std::max(max_courant, courant);
  }

  // The subcycle count it sizes must agree on every rank
  comm().max(max_courant);
  return max_courant;
}

void
ConservativeDiffuseInterfaceRhieChowMassFlux::updatePressureBoundaryNormalGradients()
{
  mooseAssert(_pressure_predictor_face_state_valid,
              "The predictor face state must be published before the boundary gradients.");

  const auto time_arg = Moose::currentState();
  Real max_boundary_normal_ainv = 0.0;

  for (const auto * fi : _flow_face_info)
    if (!_vel[0]->isInternalFace(*fi))
      max_boundary_normal_ainv =
          std::max(max_boundary_normal_ainv, std::abs(faceNormalPressureAinv(*fi)));

  // Treat a boundary pressure coefficient as degenerate only relative to the scale of the
  // current boundary operator. An absolute libMesh::TOLERANCE cutoff is too aggressive for
  // high-density-ratio outlet faces, where valid normal Ainv values can be smaller than 1e-6.
  const Real degenerate_normal_ainv_tol =
      std::max(std::numeric_limits<Real>::min(),
               std::numeric_limits<Real>::epsilon() * std::max(1.0, max_boundary_normal_ainv));

  for (const auto * fi : _flow_face_info)
  {
    if (_vel[0]->isInternalFace(*fi))
      continue;

    const Real normal_ainv = faceNormalPressureAinv(*fi);
    // The diffuse pressure equation stores area-integrated phi; the gradient wants a density.
    const Real face_measure = NS::faceMeasure(*fi);
    if (!std::isfinite(normal_ainv) || std::abs(normal_ainv) <= degenerate_normal_ainv_tol ||
        face_measure <= libMesh::TOLERANCE)
    {
      _pressure_boundary_normal_gradient[fi->id()] = 0.0;
      continue;
    }

    // The corrected flux on this face is (-phiHbyA + pressure flux) / S_f with the pressure flux
    // -(1/A)_n dn(p_rgh) S_f in the source convention, and it has to equal the prescribed
    // boundary velocity flux u_b.n. Solving for the gradient gives
    //   dn(p_rgh) = -(phiHbyA + u_b.n S_f) / (S_f (1/A)_n),
    // which is the Neumann condition the pressure equation gets on a fixed-velocity boundary
    // (zero at a wall at rest) and the gradient the predictor force reads back there.
    RealVectorValue boundary_velocity;
    for (const auto component : index_range(_vel))
      boundary_velocity(component) =
          boundaryNormalMultiplier(*fi) * boundaryVelocityValue(*fi, component, time_arg);
    const Real required_pressure_flux =
        phiHbyAFlux(*fi) + boundary_velocity * fi->normal() * face_measure;

    _pressure_boundary_normal_gradient[fi->id()] =
        -(required_pressure_flux / face_measure) / normal_ainv;
  }

  _pressure_boundary_normal_gradient_valid = true;
  ++_face_mass_flux_generation;
}

void
ConservativeDiffuseInterfaceRhieChowMassFlux::updateVelocityBoundaryState()
{
  const auto time_arg = Moose::currentState();

  _velocity_boundary_state_valid = false;
  for (auto & component_face_values : _boundary_velocity_face_values)
    component_face_values.clear();

  for (const auto * fi : _flow_face_info)
  {
    if (_vel[0]->isInternalFace(*fi))
      continue;

    RealVectorValue density_times_velocity;
    const Real boundary_normal_multiplier = boundaryNormalMultiplier(*fi);
    const Real face_rho = _rho(makeCDFace(*fi), time_arg);

    for (const auto component : index_range(_vel))
    {
      // The base class value, read from the boundary condition or the cell; the override of
      // boundaryVelocityValue serves it from this cache once the cache is marked valid
      const Real boundary_velocity =
          RhieChowMassFlux::boundaryVelocityValue(*fi, component, time_arg);
      _boundary_velocity_face_values[component][fi->id()] = boundary_velocity;
      density_times_velocity(component) = boundary_normal_multiplier * face_rho * boundary_velocity;
    }

    // Before a pressure correction has been solved, publish the predictor-side
    // boundary phi from the refreshed patch state. But once the pressure
    // equation has produced a corrected face flux, preserve that phi so the
    // boundary face correction stays matched to the cell writeback instead of
    // being overwritten by a later cache refresh.
    if (!_pressure_equation_flux_valid)
      _face_mass_flux[fi->id()] = density_times_velocity * fi->normal();
  }

  _velocity_boundary_state_valid = true;
}

Real
ConservativeDiffuseInterfaceRhieChowMassFlux::boundaryVelocityValue(
    const FaceInfo & fi, const unsigned int component, const Moose::StateArg & time_arg) const
{
  if (_velocity_boundary_state_valid)
    if (const auto it = _boundary_velocity_face_values[component].find(fi.id());
        it != _boundary_velocity_face_values[component].end())
      return it->second;

  return RhieChowMassFlux::boundaryVelocityValue(fi, component, time_arg);
}

RealVectorValue
ConservativeDiffuseInterfaceRhieChowMassFlux::interpolateMomentumVectorReadersToFace(
    const FaceInfo & fi,
    const std::vector<PetscVectorReader> & readers,
    const Moose::FV::InterpMethod interp_method) const
{
  using namespace Moose::FV;

  mooseAssert(readers.size() >= _dim, "One reader per velocity component is required.");
  RealVectorValue face_value;

  if (_vel[0]->isInternalFace(fi))
  {
    const auto & elem_info = *fi.elemInfo();
    const auto & neighbor_info = *fi.neighborInfo();

    for (const auto dim_i : make_range(_dim))
    {
      const auto elem_dof = elem_info.dofIndices()[_global_momentum_system_numbers[dim_i]][0];
      const auto neighbor_dof =
          neighbor_info.dofIndices()[_global_momentum_system_numbers[dim_i]][0];
      interpolate(interp_method,
                  face_value(dim_i),
                  readers[dim_i](elem_dof),
                  readers[dim_i](neighbor_dof),
                  fi,
                  true);
    }

    return face_value;
  }

  const ElemInfo & elem_info = boundaryElemInfo(fi);
  for (const auto dim_i : make_range(_dim))
  {
    const auto elem_dof = elem_info.dofIndices()[_global_momentum_system_numbers[dim_i]][0];
    face_value(dim_i) = readers[dim_i](elem_dof);
  }

  return face_value;
}

Real
ConservativeDiffuseInterfaceRhieChowMassFlux::computeFaceNormalAinv(
    const RealVectorValue & face_ainv, const RealVectorValue & face_normal) const
{
  Real normal_ainv = 0.0;
  for (const auto dim_i : make_range(_dim))
    normal_ainv += face_ainv(dim_i) * face_normal(dim_i) * face_normal(dim_i);

  return normal_ainv;
}

InputParameters
ConservativeDiffuseInterfaceRhieChowMassFlux::validParams()
{
  InputParameters params = RhieChowMassFlux::validParams();

  params.addClassDescription(
      "Rhie-Chow face-flux provider for the reduced-pressure diffuse-interface two-phase solve: "
      "the momentum predictor is assembled without the pressure, gravity and capillary forces, "
      "which enter the pressure equation as face fluxes, and the face mass flux is the one the "
      "limited volume-fraction transport advanced with.");

  params.addParam<MooseFunctorName>(
      "vof_rho_phi_functor",
      "rho_phi",
      "Alpha-consistent density flux published by the diffuse-interface VOF transport. "
      "Downstream advection queries use this face mass flux instead of the raw Rhie-Chow density "
      "interpolation.");
  params.addParam<RealVectorValue>(
      "gravity",
      RealVectorValue(),
      "Gravity vector used by the reduced-pressure hydrostatic pressure initializer.");
  params.addParam<Point>(
      "reference_pressure_point",
      Point(0, 0, 0),
      "Reference point used to compute the reduced-pressure head gh = g.(x-x_ref).");
  params.addParam<MooseFunctorName>(
      "surface_tension_force_density_functor",
      "",
      "Optional surface tension force density functor sigma*kappa*grad(alpha), as published by "
      "ConservativeDiffuseInterfaceSurfaceTensionFunctorMaterial. When supplied, the "
      "diffuse-interface pressure coupling carries the capillary force.");
  // The consistent (SIMPLEC) projection would replace the unrelaxed, diagonally dominant 1/A of
  // computePredictorOperator with a row sum of the relaxed matrix
  params.set<MooseEnum>("pressure_projection_method") =
      MooseEnum("standard consistent", "standard");
  params.suppressParameter<MooseEnum>("pressure_projection_method");

  return params;
}

ConservativeDiffuseInterfaceRhieChowMassFlux::ConservativeDiffuseInterfaceRhieChowMassFlux(
    const InputParameters & params)
  : RhieChowMassFlux(params),
    _phiHbyA_flux(_moose_mesh,
                  blockIDs(),
                  "phiHbyA_flux",
                  [this](const FaceInfo & fi) { return phiHbyAFlux(fi); }),
    _phig_flux(_moose_mesh, blockIDs(), "phig_flux"),
    _pressure_Ainv(_moose_mesh, blockIDs(), "pressure_Ainv"),
    _pressure_boundary_normal_gradient(
        _moose_mesh, blockIDs(), "pressure_boundary_normal_gradient"),
    _boundary_velocity_face_values(_dim),
    _corrected_face_phi(_moose_mesh,
                        blockIDs(),
                        "corrected_face_phi",
                        [this](const FaceInfo & fi) { return getVolumetricFaceFlux(fi); }),
    _previous_timestep_corrected_face_phi(
        declareRestartableData<FaceScalarField>("previous_timestep_corrected_face_phi",
                                                _moose_mesh,
                                                blockIDs(),
                                                "previous_timestep_corrected_face_phi")),
    _older_timestep_corrected_face_phi(
        declareRestartableData<FaceScalarField>("older_timestep_corrected_face_phi",
                                                _moose_mesh,
                                                blockIDs(),
                                                "older_timestep_corrected_face_phi")),
    _transport_history_seeded(declareRestartableData<bool>("transport_history_seeded", false)),
    _accepted_pressure_boundary_normal_gradient(
        declareRestartableData<std::unordered_map<dof_id_type, Real>>(
            "accepted_pressure_boundary_normal_gradient")),
    _accepted_pressure_Ainv(
        declareRestartableData<std::unordered_map<dof_id_type, RealVectorValue>>(
            "accepted_pressure_Ainv")),
    _lagged_corrected_face_phi(_moose_mesh, blockIDs(), "lagged_corrected_face_phi"),
    _gravity(getParam<RealVectorValue>("gravity")),
    _reference_pressure_point(getParam<Point>("reference_pressure_point")),
    _vof_rho_phi_name(getParam<MooseFunctorName>("vof_rho_phi_functor")),
    _surface_tension_force_name(
        getParam<MooseFunctorName>("surface_tension_force_density_functor")),
    _vof_rho_phi(nullptr)
{
  for (const auto tid : make_range(libMesh::n_threads()))
  {
    UserObject::_subproblem.addFunctor("pressure_Ainv", _pressure_Ainv, tid);
    UserObject::_subproblem.addFunctor("phiHbyA", _phiHbyA_flux, tid);
    UserObject::_subproblem.addFunctor(
        "pressure_boundary_normal_gradient", _pressure_boundary_normal_gradient, tid);
    UserObject::_subproblem.addFunctor("corrected_face_phi", _corrected_face_phi, tid);
    UserObject::_subproblem.addFunctor(
        "previous_timestep_corrected_face_phi", _previous_timestep_corrected_face_phi, tid);
    UserObject::_subproblem.addFunctor(
        "lagged_corrected_face_phi", _lagged_corrected_face_phi, tid);
  }

  initializeAdditionalPressureFluxStorage();
}

Real
ConservativeDiffuseInterfaceRhieChowMassFlux::getMassFlux(const FaceInfo & fi) const
{
  // Deliberately not the inherited _face_mass_flux, which this class also maintains as the
  // restartable rho_f * phi_f. Momentum advection needs the density flux that the limited volume
  // fraction update actually transported, and across a large density jump the two differ: the
  // transported product does not factor into one face density times one face volumetric flux.
  // _face_mass_flux remains what the base class and restart expect; this is what advection needs.
  const Real face_measure = NS::faceMeasure(fi);
  return face_measure > 0.0 ? (*_vof_rho_phi)(makeCDFace(fi), Moose::currentState()) / face_measure
                            : 0.0;
}

void
ConservativeDiffuseInterfaceRhieChowMassFlux::commitAcceptedTimestepTransportHistory()
{
  // The history of the two last accepted steps; the first commit seeds both with the same flux
  for (const auto * fi : _flow_face_info)
  {
    _older_timestep_corrected_face_phi[fi->id()] =
        _transport_history_seeded
            ? libmesh_map_find(_previous_timestep_corrected_face_phi, fi->id())
            : getVolumetricFaceFlux(*fi);
    _previous_timestep_corrected_face_phi[fi->id()] = getVolumetricFaceFlux(*fi);
  }
  _transport_history_seeded = true;

  _accepted_pressure_boundary_normal_gradient.clear();
  _accepted_pressure_Ainv.clear();
  if (_pressure_boundary_normal_gradient_valid)
  {
    _accepted_pressure_boundary_normal_gradient.insert(_pressure_boundary_normal_gradient.begin(),
                                                       _pressure_boundary_normal_gradient.end());
    _accepted_pressure_Ainv.insert(_pressure_Ainv.begin(), _pressure_Ainv.end());
  }
}

void
ConservativeDiffuseInterfaceRhieChowMassFlux::seedTransportHistoryOnce()
{
  if (!_transport_history_seeded)
    commitAcceptedTimestepTransportHistory();
}

void
ConservativeDiffuseInterfaceRhieChowMassFlux::timestepSetup()
{
  RhieChowMassFlux::timestepSetup();
  _face_normal_density_gradient_valid = false;
  initializeAdditionalPressureFluxStorage();

  // Every attempt at a step starts from the boundary pressure force of the last accepted step
  if (!_accepted_pressure_boundary_normal_gradient.empty())
  {
    for (const auto & [face_id, gradient] : _accepted_pressure_boundary_normal_gradient)
      _pressure_boundary_normal_gradient[face_id] = gradient;
    for (const auto & [face_id, ainv] : _accepted_pressure_Ainv)
      _pressure_Ainv[face_id] = ainv;
    _pressure_boundary_normal_gradient_valid = true;
  }
}

RealVectorValue
ConservativeDiffuseInterfaceRhieChowMassFlux::reducedPressureMomentumPredictorForceDensity(
    const ElemInfo & elem_info, const Moose::StateArg & time_arg) const
{
  if (!hasBlocks(elem_info.subdomain_id()))
    return RealVectorValue();

  return reconstructCellVectorFromFaceNormalScalars(
      _moose_mesh,
      _dim,
      elem_info,
      [this, &time_arg](const FaceInfo & fi_loc)
      {
        // Every face, boundary faces included: the pressure-corrected velocity is written back
        // from the same all-face reconstruction, and a wall cell whose predictor force skipped
        // the wall face never satisfies the momentum equation the corrector hands it.
        Real normal_force_density = -computeFaceNormalPressureGradient(fi_loc, time_arg);
        normal_force_density +=
            computeDiscreteHydrostaticPredictorFaceNormalForceDensity(fi_loc, time_arg);
        normal_force_density += surfaceTensionFaceNormalForceDensity(fi_loc, time_arg);
        return normal_force_density;
      });
}

void
ConservativeDiffuseInterfaceRhieChowMassFlux::initializeAdditionalPressureFluxStorage()
{
  // Keyed over every face of the local mesh, not only the owned ones. These maps are published as
  // functors, and a cell-gradient stencil that reaches a ghosted cell asks for a value on that
  // cell's own boundary faces. Keeping the keys present makes the functors total; the owned faces
  // are still the only ones computed here, and synchronizeHaloFaceFlux() brings the rest over.
  for (const auto & fi : _fe_problem.mesh().allFaceInfo())
  {
    // The accepted-step history is written once per accepted step; it is only zeroed while it
    // still holds nothing, and otherwise merely kept keyed.
    if (!_transport_history_seeded)
    {
      _previous_timestep_corrected_face_phi[fi.id()] = 0.0;
      _older_timestep_corrected_face_phi[fi.id()] = 0.0;
    }
    else
    {
      _previous_timestep_corrected_face_phi.try_emplace(fi.id(), 0.0);
      _older_timestep_corrected_face_phi.try_emplace(fi.id(), 0.0);
    }
    _lagged_corrected_face_phi[fi.id()] = 0.0;
    _phig_flux[fi.id()] = 0.0;
    _phiHbyA_face_values[fi.id()] = 0.0;
    _pressure_Ainv[fi.id()] = RealVectorValue();
    _pressure_boundary_normal_gradient[fi.id()] = 0.0;
  }

  for (auto & component_face_values : _boundary_velocity_face_values)
    component_face_values.clear();
  _velocity_boundary_state_valid = false;
  _pressure_boundary_normal_gradient_valid = false;
  _pressure_predictor_face_state_valid = false;
}

void
ConservativeDiffuseInterfaceRhieChowMassFlux::writePressureCorrectedVelocityToMomentumSolution(
    const Real degenerate_normal_pressure_ainv_tol)
{
  std::vector<std::unique_ptr<NumericVector<Number>>> pressure_corrected_solution;
  pressure_corrected_solution.reserve(_momentum_implicit_systems.size());

  for (const auto system_i : index_range(_momentum_implicit_systems))
  {
    auto * momentum_system = _momentum_implicit_systems[system_i];
    mooseAssert(momentum_system && momentum_system->current_local_solution,
                "The requested momentum component is not linked to "
                "ConservativeDiffuseInterfaceRhieChowMassFlux.");
    pressure_corrected_solution.push_back(momentum_system->current_local_solution->zero_clone());
  }

  for (const auto & elem_info : _fe_problem.mesh().elemInfoVector())
  {
    if (!hasBlocks(elem_info->subdomain_id()))
      continue;

    const RealVectorValue pressure_delta = reconstructPressureCoupledCellVelocityDelta(
        *elem_info, degenerate_normal_pressure_ainv_tol);

    for (const auto system_i : index_range(_momentum_implicit_systems))
    {
      const auto & dof_indices = elem_info->dofIndices()[_global_momentum_system_numbers[system_i]];
      if (dof_indices.empty())
        continue;

      // The predictor velocity -(H/A) plus the pressure correction
      pressure_corrected_solution[system_i]->set(
          dof_indices[0], -(*_HbyA_raw[system_i])(dof_indices[0]) + pressure_delta(system_i));
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
ConservativeDiffuseInterfaceRhieChowMassFlux::faceNormalPressureAinv(const FaceInfo & fi) const
{
  // _pressure_Ainv is the face coefficient the pressure diffusion kernel consumes, on internal
  // and boundary faces alike, so the writeback divides by the same value the kernel used
  return computeFaceNormalAinv(libmesh_map_find(_pressure_Ainv, fi.id()), fi.normal());
}

Real
ConservativeDiffuseInterfaceRhieChowMassFlux::phiHbyAFlux(const FaceInfo & fi) const
{
  return faceValueOrZero(_phiHbyA_face_values, fi);
}

Real
ConservativeDiffuseInterfaceRhieChowMassFlux::getVolumetricFaceFlux(const FaceInfo & fi) const
{
  // The corrected transport flux is the predictor flux plus whatever the pressure equation added,
  // both of which the provider already holds. Expressing it rather than storing it means a
  // consumer cannot read a value that was correct for an earlier predictor state: that staleness
  // is what let a transient outlet backflow decision persist.
  if (!_pressure_predictor_face_state_valid)
  {
    // Before a predictor exists the only mass flux is the one initFaceMassFlux built as
    // face_rho * boundary velocity, formed with a single face density, so dividing it back out
    // recovers that velocity rather than inverting a transported product. This is the one place
    // the division is exact, which is why it is confined to this branch: the transported rhoPhi
    // is not generally rho_f * phi, so a face density must never be recovered from it.
    const Real face_rho = interpolateFaceDensity(fi, Moose::currentState());
    return std::abs(face_rho) > libMesh::TOLERANCE ? faceValueOrZero(_face_mass_flux, fi) / face_rho
                                                   : 0.0;
  }

  const Real face_measure = NS::faceMeasure(fi);
  if (face_measure <= libMesh::TOLERANCE)
    return 0.0;

  // The diffuse pressure equation stores area-integrated phi; transport wants a flux density.
  const Real predictor_phi = -phiHbyAFlux(fi) / face_measure;
  const Real pressure_equation_phi =
      _pressure_equation_flux_valid ? faceValueOrZero(_pressure_equation_flux, fi) / face_measure
                                    : 0.0;
  return predictor_phi + pressure_equation_phi;
}

Real
ConservativeDiffuseInterfaceRhieChowMassFlux::writebackReconstructionScalar(
    const FaceInfo & fi, const Real degenerate_normal_pressure_ainv_tol) const
{
  if (!_pressure_predictor_face_state_valid)
    return 0.0;

  // The diffuse pressure equation stores area-integrated phi. Convert back to the
  // normal-flux-density transport convention only at the writeback boundary.
  const Real face_measure = NS::faceMeasure(fi);
  if (face_measure <= libMesh::TOLERANCE)
    return 0.0;

  const Real pressure_writeback_phi =
      libmesh_map_find(_pressure_equation_flux, fi.id()) / face_measure +
      libmesh_map_find(_phig_flux, fi.id()) / face_measure;
  const Real normal_pressure_ainv = faceNormalPressureAinv(fi);

  return std::isfinite(normal_pressure_ainv) &&
                 std::abs(normal_pressure_ainv) > degenerate_normal_pressure_ainv_tol
             ? pressure_writeback_phi / normal_pressure_ainv
             : 0.0;
}

bool
ConservativeDiffuseInterfaceRhieChowMassFlux::isHaloFace(const FaceInfo & fi) const
{
  if (!hasFaceSide(fi, true) && !hasFaceSide(fi, false))
    return false;
  if (fi.elem().processor_id() == processor_id())
    return false;
  return !(fi.neighborPtr() && fi.neighborPtr()->processor_id() == processor_id());
}

void
ConservativeDiffuseInterfaceRhieChowMassFlux::synchronizeHaloFaceFlux(
    FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>> & field) const
{
  if (n_processors() == 1)
    return;

  std::unordered_map<processor_id_type, std::vector<dof_id_type>> queries;
  for (const auto & fi : _fe_problem.mesh().allFaceInfo())
    if (isHaloFace(fi))
      queries[fi.elem().processor_id()].push_back(fi.id());

  auto gather = [&field](const processor_id_type,
                         const std::vector<dof_id_type> & face_ids,
                         std::vector<Real> & values)
  {
    for (const auto face_id : face_ids)
    {
      const auto it = field.find(face_id);
      values.push_back(it == field.end() ? 0.0 : it->second);
    }
  };

  auto act = [&field](const processor_id_type,
                      const std::vector<dof_id_type> & face_ids,
                      const std::vector<Real> & values)
  {
    for (const auto i : index_range(face_ids))
      field[face_ids[i]] = values[i];
  };

  libMesh::Parallel::pull_parallel_vector_data<Real>(
      _communicator, queries, gather, act, static_cast<const Real *>(nullptr));
}

void
ConservativeDiffuseInterfaceRhieChowMassFlux::freezeLaggedCorrectedFaceFlux(
    const bool use_previous_timestep_flux)
{
  for (const auto * fi : _flow_face_info)
    _lagged_corrected_face_phi[fi->id()] =
        use_previous_timestep_flux
            ? libmesh_map_find(_previous_timestep_corrected_face_phi, fi->id())
            : getVolumetricFaceFlux(*fi);

  synchronizeHaloFaceFlux(_lagged_corrected_face_phi);
}

void
ConservativeDiffuseInterfaceRhieChowMassFlux::meshChanged()
{
  RhieChowMassFlux::meshChanged();
  _face_normal_density_gradient.clear();
  _face_normal_density_gradient_valid = false;
  // Keyed by face ids of the old mesh
  _accepted_pressure_boundary_normal_gradient.clear();
  _accepted_pressure_Ainv.clear();
  _momentum_predictor_operators.clear();
  _predictor_ready = false;
  initializeAdditionalPressureFluxStorage();
}

void
ConservativeDiffuseInterfaceRhieChowMassFlux::initialSetup()
{
  // The first predictor force reads the pressure before any pressure solve has run
  const_cast<MooseLinearVariableFVReal *>(_p)->computeCellGradients();
  RhieChowMassFlux::initialSetup();
  initializeAdditionalPressureFluxStorage();
  if (!_vof_rho_phi_name.empty() &&
      UserObject::_subproblem.hasFunctorWithType<Real>(_vof_rho_phi_name, _tid))
    _vof_rho_phi = &getFunctor<Real>(_vof_rho_phi_name);

  if (!_vof_rho_phi)
    paramError("vof_rho_phi_functor",
               "The conservative diffuse-interface path requires the VOF-owned rhoPhi functor.");

  if (!_surface_tension_force_name.empty())
    _surface_tension_force = &getFunctor<RealVectorValue>(_surface_tension_force_name);
}

void
ConservativeDiffuseInterfaceRhieChowMassFlux::computeHbyA(const bool verbose)
{
  // A new predictor invalidates everything derived from the previous one.
  _pressure_predictor_face_state_valid = false;
  _pressure_boundary_normal_gradient_valid = false;
  RhieChowMassFlux::computeHbyA(verbose);
}

void
ConservativeDiffuseInterfaceRhieChowMassFlux::computeFaceMassFlux()
{
  mooseAssert(_pressure_predictor_face_state_valid,
              "The predictor face state must be published before the face mass flux.");

  if (!_pressure_equation_flux_valid)
    cachePressureEquationFlux();

  const auto time_arg = Moose::currentState();
  for (const auto * fi : _flow_face_info)
  {
    // Keep the pressure-corrected face flux as the source of truth. Do not reconstruct cell U and
    // project it back to faces here; that round trip is not an exact inverse.
    _face_mass_flux[fi->id()] = interpolateFaceDensity(*fi, time_arg) * getVolumetricFaceFlux(*fi);
  }
}

Real
ConservativeDiffuseInterfaceRhieChowMassFlux::interpolateFaceDensity(
    const FaceInfo & fi, const Moose::StateArg & time_arg) const
{
  if (_vel[0]->isInternalFace(fi) || useConstrainedBoundaryPredictorState(fi))
    return _rho(makeCDFace(fi), time_arg);

  return _rho(makeElemArg(boundaryElemInfo(fi).elem()), time_arg);
}

Real
ConservativeDiffuseInterfaceRhieChowMassFlux::interpolatedPhysicalFaceFlux(
    const FaceInfo & fi, const Moose::StateArg & time_arg) const
{
  RealVectorValue face_velocity;
  if (_vel[0]->isInternalFace(fi))
  {
    const auto & elem_info = *fi.elemInfo();
    const auto & neighbor_info = *fi.neighborInfo();
    for (const auto dim_i : make_range(_dim))
      face_velocity(dim_i) =
          Moose::FV::linearInterpolation(_vel[dim_i]->getElemValue(elem_info, time_arg),
                                         _vel[dim_i]->getElemValue(neighbor_info, time_arg),
                                         fi,
                                         true);
  }
  else
    for (const auto dim_i : make_range(_dim))
      face_velocity(dim_i) = boundaryVelocityValue(fi, dim_i, time_arg);

  return face_velocity * fi.normal();
}

Real
ConservativeDiffuseInterfaceRhieChowMassFlux::computeDefaultTransientProjectionVolumetricFlux(
    const FaceInfo & fi, const Moose::StateArg & time_arg, const Real normal_density_weighted_ainv)
{
  if (time_arg.state != Moose::currentState().state)
    return 0.0;

  const Real dt = _fe_problem.dt();
  if (dt <= libMesh::TOLERANCE)
    return 0.0;

  if (!_vel[0]->isInternalFace(fi))
    return 0.0;

  // phi_corr of an old state is the part of its face flux that its cell velocity does not
  // reproduce when interpolated to the face: the Rhie-Chow dissipation of that step. The old
  // states enter the predictor through the time derivative, each with the weight the time
  // integrator gives it (1/dt for implicit Euler; 2/dt and -1/(2 dt) for BDF2 at constant step)
  // times rho per unit 1/A, so adding their phi_corr back with the same weights keeps the face
  // flux consistent with the time derivative the predictor used, for every integrator. A
  // mismatched weight would leave a flux error proportional to the dissipation, which itself
  // scales with dt, and so a first-order error in time. The weighted sum is scaled by dt so that
  // it has the size of a flux, which the fading coefficient below compares with the old flux.
  const auto & weights =
      _momentum_systems[0]->getTimeIntegrator(_vel[0]->number()).timeDerivativeRHSWeights();
  const std::array<const FaceScalarField *, 2> histories = {&_previous_timestep_corrected_face_phi,
                                                            &_older_timestep_corrected_face_phi};
  if (weights.size() > histories.size())
    mooseError("The transient projection keeps the face flux of the two last accepted steps, "
               "but the time integrator of the momentum systems reads ",
               weights.size(),
               " old states.");
  Real weighted_phi_corr = 0.0;
  for (const auto i : index_range(weights))
    if (weights[i] != 0.0)
    {
      const Real old_corrected_phi = libmesh_map_find(*histories[i], fi.id());
      const Real old_interpolated_face_phi = interpolatedPhysicalFaceFlux(
          fi, Moose::StateArg(i + 1, Moose::SolutionIterationType::Time));
      weighted_phi_corr += dt * weights[i] * (old_corrected_phi - old_interpolated_face_phi);
    }

  // The coefficient fades the correction out where it is as large as the old flux itself, which
  // happens at a fresh start or where the flow is reversing, and would otherwise feed a stale
  // flux back in
  const Real old_corrected_phi = libmesh_map_find(_previous_timestep_corrected_face_phi, fi.id());
  const Real phi_scale = std::abs(old_corrected_phi) + std::numeric_limits<Real>::epsilon();
  const Real ddt_coupling_coeff =
      std::max(0.0, 1.0 - std::min(std::abs(weighted_phi_corr) / phi_scale, 1.0));

  // ddtCorr contribution, times interpolate(rho*rAU), which the caller adds to phiHbyA
  return ddt_coupling_coeff * normal_density_weighted_ainv * weighted_phi_corr / dt;
}

Real
ConservativeDiffuseInterfaceRhieChowMassFlux::evaluateFaceNormalDensityGradient(
    const FaceInfo & fi, const Moose::StateArg & time_arg) const
{
  const bool internal = _vel[0]->isInternalFace(fi);
  return NS::faceNormalScalarGradient(
      _rho, fi, internal ? nullptr : boundaryElemInfo(fi).elem(), internal, time_arg);
}

void
ConservativeDiffuseInterfaceRhieChowMassFlux::updateFaceNormalDensityGradients()
{
  const auto time_arg = Moose::currentState();
  // One cell gradient per cell for the sweep, instead of two per face.
  std::unordered_map<const Elem *, RealVectorValue> cell_gradients;
  const auto gradient_of = [this, &time_arg, &cell_gradients](const Elem * const elem)
  {
    auto it = cell_gradients.find(elem);
    if (it == cell_gradients.end())
      it =
          cell_gradients
              .emplace(elem,
                       MetaPhysicL::raw_value(_rho.gradient(Moose::ElemArg{elem, false}, time_arg)))
              .first;
    return it->second;
  };
  static const RealVectorValue zero_gradient;
  for (const auto * fi : _flow_face_info)
    _face_normal_density_gradient[fi->id()] =
        !_vel[0]->isInternalFace(*fi) ? evaluateFaceNormalDensityGradient(*fi, time_arg)
        // On an orthogonal face the cell gradients multiply a zero correction: skip them.
        : NS::faceNonOrthogonalCorrection(*fi).norm_sq() == 0.0
            ? NS::faceNormalScalarGradient(_rho, *fi, time_arg, zero_gradient, zero_gradient)
            : NS::faceNormalScalarGradient(
                  _rho, *fi, time_arg, gradient_of(fi->elemPtr()), gradient_of(fi->neighborPtr()));
  _face_normal_density_gradient_valid = true;
}

Real
ConservativeDiffuseInterfaceRhieChowMassFlux::computeFaceNormalPressureGradient(
    const FaceInfo & fi, const Moose::StateArg & time_arg) const
{
  if (_vel[0]->isInternalFace(fi))
  {
    const Real elem_p = _p->getElemValue(*fi.elemInfo(), time_arg);
    const Real neighbor_p = _p->getElemValue(*fi.neighborInfo(), time_arg);
    const Real normal_spacing = std::abs(fi.dCN() * fi.normal());

    return normal_spacing > libMesh::TOLERANCE ? (neighbor_p - elem_p) / normal_spacing : 0.0;
  }

  // On a fixed-value boundary the pressure equation sets the flux itself, so the predictor must
  // see the gradient that flux implies, the same one the velocity writeback divides out. The
  // cached gradient below is the one a fixed-flux boundary needs, and handing it to an outlet
  // left every outlet cell short of the momentum equation by a constant.
  if (auto * bc_pointer = NS::linearFVBoundaryCondition(*_p, fi);
      bc_pointer && std::isfinite(bc_pointer->computeBoundaryValue()) &&
      _pressure_equation_flux_valid && _pressure_Ainv.find(fi.id()) != _pressure_Ainv.end())
  {
    const Real normal_ainv = faceNormalPressureAinv(fi);
    const Real face_measure = NS::faceMeasure(fi);
    if (std::isfinite(normal_ainv) && std::abs(normal_ainv) > std::numeric_limits<Real>::min() &&
        face_measure > libMesh::TOLERANCE)
    {
      PetscVectorReader p_reader(*_pressure_system->system().current_local_solution);
      return -computeFacePressureGradientFlux(fi, p_reader) / (face_measure * normal_ainv);
    }
  }

  if (_pressure_boundary_normal_gradient_valid)
    return libmesh_map_find(_pressure_boundary_normal_gradient, fi.id());

  const ElemInfo & elem_info = boundaryElemInfo(fi);
  const Point fluid_centroid = elem_info.centroid();
  const Point cell_to_face = fi.faceCentroid() - fluid_centroid;
  const Real normal_spacing = std::abs(cell_to_face * fi.normal());

  if (normal_spacing <= libMesh::TOLERANCE)
    return 0.0;

  const Real elem_p = _p->getElemValue(elem_info, time_arg);

  if (auto * bc_pointer = NS::linearFVBoundaryCondition(*_p, fi))
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
ConservativeDiffuseInterfaceRhieChowMassFlux::
    computeDiscreteHydrostaticPredictorFaceNormalForceDensity(
        const FaceInfo & fi, const Moose::StateArg & time_arg) const
{
  if (_startup_projection_active)
    return 0.0;

  // With p = p_rgh + rho gh, the body force and pressure gradient of the momentum equation are
  //   -grad(p) + rho g = -grad(p_rgh) - gh grad(rho),
  // so the gravity term becomes a force that is nonzero only where the density varies, that is
  // at the interface. Evaluating gh at the face centroid and grad(rho) as a face-normal
  // difference makes it cancel the face-normal p_rgh gradient exactly in a fluid at rest, which a
  // cell-centered rho g against a face pressure gradient does not.
  const Real ghf = _gravity * (fi.faceCentroid() - _reference_pressure_point);
  // The face-normal density gradient, from the per-predictor cache when it is current
  if (_face_normal_density_gradient_valid && time_arg.state == 0)
    if (const auto it = _face_normal_density_gradient.find(fi.id());
        it != _face_normal_density_gradient.end())
      return -ghf * it->second;
  return -ghf * evaluateFaceNormalDensityGradient(fi, time_arg);
}

Real
ConservativeDiffuseInterfaceRhieChowMassFlux::surfaceTensionFaceNormalForceDensity(
    const FaceInfo & fi, const Moose::StateArg & time_arg) const
{
  if (!_surface_tension_force || _startup_projection_active)
    return 0.0;

  // An interior face averages the two cell forces; a boundary face names the side that owns the
  // cell quantity, since there is no second cell to average with. makeCDFace does both, from
  // which sides of the face our blocks cover.
  return (*_surface_tension_force)(makeCDFace(fi), time_arg) * fi.normal();
}

void
ConservativeDiffuseInterfaceRhieChowMassFlux::updateAdditionalPressureFluxFunctors()
{
  // Builds the three face fields of the pressure equation from the predictor of this outer
  // iteration, all area-integrated and in the source convention:
  //   base    = -((-H/A).n + ddtCorr) S_f           the predictor flux,
  //   phig    = (-gh dn(rho) + f_sigma.n) (1/A)_n S_f the explicit face forces,
  //   phiHbyA = base - phig                           the right-hand side flux,
  // and pressure_Ainv = (1/A)_f, the coefficient of div((1/A)_f grad(p_rgh)). The face forces
  // carry the same (1/A)_n the pressure gradient carries in the pressure flux, so that at
  // equilibrium the pressure flux cancels phig face by face.
  _pressure_predictor_face_state_valid = false;

  const auto time_arg = Moose::currentState();

  mooseAssert(_Ainv_raw.size() >= _dim, "computeHbyA must run before the face operators.");
  std::vector<PetscVectorReader> raw_ainv_readers;
  raw_ainv_readers.reserve(_dim);
  for (const auto dim_i : make_range(_dim))
    raw_ainv_readers.emplace_back(*_Ainv_raw[dim_i]);

  std::vector<PetscVectorReader> hbya_readers;
  hbya_readers.reserve(_HbyA_raw.size());
  for (const auto & hbya_raw : _HbyA_raw)
    hbya_readers.emplace_back(*hbya_raw);

  for (const auto * fi : _flow_face_info)
  {
    const auto predictor_state =
        buildMomentumPredictorFaceState(*fi, hbya_readers, raw_ainv_readers, time_arg);

    const Real face_measure = NS::faceMeasure(*fi);
    // FaceMomentumPredictorState carries H/A with the sign it is stored in, so that the velocity
    // u = -(H/A) - Ainv grad(p). The reduced-pressure predictor flux is a velocity, and the
    // bookkeeping below negates it once more when forming the base flux, so convert here rather
    // than asking the shared producer to hold a second sign convention for this consumer alone.
    const Real predictor_operator_volumetric_flux = -(predictor_state.face_hbya * fi->normal());
    // The pressure equation's diffusion coefficient and the explicit face forces scaled by it must
    // use the same face value, or their equilibrium cancellation breaks, so one interpolation
    // method serves both. 1/A scales as dt/rho, so it jumps with the density across the interface.
    // Averaging is the default even though harmonic weighting is the usual choice for a diffusion
    // coefficient: this face value also multiplies the explicit face forces and divides the
    // velocity writeback, and the gas-side bias of a harmonic mean unbalances those three uses
    // against each other. A harmonic hydrostatic interface diverges on the first step.
    const RealVectorValue face_raw_ainv = interpolateMomentumVectorReadersToFace(
        *fi, raw_ainv_readers, _pressure_diffusion_interp_method);
    const Real normal_pressure_ainv = computeFaceNormalAinv(face_raw_ainv, fi->normal());
    Real transient_projection_flux = 0.0;
    Real hydrostatic_flux = 0.0;
    Real surface_tension_flux = 0.0;

    if (!_startup_projection_active)
    {
      transient_projection_flux = computeDefaultTransientProjectionVolumetricFlux(
          *fi,
          time_arg,
          computeFaceNormalAinv(predictor_state.face_density_weighted_ainv, fi->normal()));

      hydrostatic_flux = computeDiscreteHydrostaticPredictorFaceNormalForceDensity(*fi, time_arg) *
                         normal_pressure_ainv;

      surface_tension_flux =
          surfaceTensionFaceNormalForceDensity(*fi, time_arg) * normal_pressure_ainv;
    }

    const Real phig_flux = (hydrostatic_flux + surface_tension_flux) * face_measure;
    const Real pressure_predictor_base_flux =
        (predictor_operator_volumetric_flux + transient_projection_flux) * face_measure;

    _phig_flux[fi->id()] = phig_flux;
    _pressure_Ainv[fi->id()] = face_raw_ainv;
    _phiHbyA_face_values[fi->id()] = -pressure_predictor_base_flux - phig_flux;
  }

  _pressure_predictor_face_state_valid = true;
}

bool
ConservativeDiffuseInterfaceRhieChowMassFlux::useConstrainedBoundaryPredictorState(
    const FaceInfo & fi) const
{
  if (_vel[0]->isInternalFace(fi))
    return false;

  bool use_constrained_boundary_state = _vel[0]->isDirichletBoundaryFace(fi);
  if (!use_constrained_boundary_state)
  {
    for (const auto dim_i : make_range(_dim))
      if (auto * bc_pointer = NS::linearFVBoundaryCondition(*_vel[dim_i], fi))
      {
        if (dynamic_cast<LinearFVPressureInletOutletVelocityBC *>(bc_pointer))
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

RealVectorValue
ConservativeDiffuseInterfaceRhieChowMassFlux::reconstructPressureCoupledCellVelocityDelta(
    const ElemInfo & elem_info, const Real degenerate_normal_pressure_ainv_tol) const
{
  // The face scalar is (pressure flux + phig) / (S_f (1/A)_n), that is -dn(p_rgh) plus the face
  // forces, so its reconstruction is the cell force density the corrected velocity responds to,
  // and the cell 1/A per component turns it into the velocity change. The face flux itself is
  // never reconstructed: the cell velocity is a momentum-consistent reading of the corrected
  // state, continuity lives on the faces. The corrected face flux stays authoritative for
  // continuity and transport; this scalar only feeds the cell-centered velocity writeback and is
  // never projected back onto the faces.
  const RealVectorValue reconstructed_operator = reconstructCellVectorFromFaceNormalScalars(
      _moose_mesh,
      _dim,
      elem_info,
      [this, degenerate_normal_pressure_ainv_tol](const FaceInfo & fi_loc)
      { return writebackReconstructionScalar(fi_loc, degenerate_normal_pressure_ainv_tol); });

  RealVectorValue pressure_delta;
  for (const auto system_i : index_range(_momentum_implicit_systems))
  {
    const auto & dof_indices = elem_info.dofIndices()[_global_momentum_system_numbers[system_i]];
    if (dof_indices.empty())
      continue;

    pressure_delta(system_i) =
        (*_Ainv_raw[system_i])(dof_indices[0]) * reconstructed_operator(system_i);
  }

  return pressure_delta;
}

void
ConservativeDiffuseInterfaceRhieChowMassFlux::computeCellVelocity()
{
  if (!_pressure_equation_flux_valid)
    cachePressureEquationFlux();

  // The degenerate-coefficient cutoff is a fraction of the active-face maximum. An absolute
  // cutoff would suppress the writeback on interface faces, where the live face-normal 1/A is
  // far below one.
  Real max_abs_normal_pressure_ainv = 0.0;
  for (const auto * fi : _flow_face_info)
  {
    const Real normal_pressure_ainv = faceNormalPressureAinv(*fi);
    if (std::isfinite(normal_pressure_ainv))
      max_abs_normal_pressure_ainv =
          std::max(max_abs_normal_pressure_ainv, std::abs(normal_pressure_ainv));
  }
  // Over all processes, so the cutoff does not depend on the partitioning
  _communicator.max(max_abs_normal_pressure_ainv);
  const Real degenerate_normal_pressure_ainv_tol =
      std::max(std::numeric_limits<Real>::min(),
               pressure_writeback_face_ainv_relative_tolerance * max_abs_normal_pressure_ainv);

  writePressureCorrectedVelocityToMomentumSolution(degenerate_normal_pressure_ainv_tol);
  _velocity_boundary_state_valid = false;
}

void
ConservativeDiffuseInterfaceRhieChowMassFlux::addMomentumPredictorExplicitForcing(
    const unsigned int system_i, NumericVector<Number> & rhs) const
{
  mooseAssert(system_i < _momentum_implicit_systems.size(),
              "Momentum component index out of range.");

  // Adds the volume integral of the reduced-pressure force density, see
  // reducedPressureMomentumPredictorForceDensity, to the predictor's right-hand side. The
  // physics adds no pressure gradient kernel, so this is the only place the pressure, gravity
  // and capillary forces enter the momentum equation.

  // The reconstruction is the same vector for every component: build it with the first one.
  if (system_i == 0)
    _predictor_force_density.clear();
  for (const auto & elem_info : _fe_problem.mesh().elemInfoVector())
  {
    if (!hasBlocks(elem_info->subdomain_id()))
      continue;

    const auto & dof_indices = elem_info->dofIndices()[_global_momentum_system_numbers[system_i]];
    if (dof_indices.empty())
      continue;

    auto it = _predictor_force_density.find(elem_info->elem()->id());
    if (it == _predictor_force_density.end())
      it = _predictor_force_density
               .emplace(
                   elem_info->elem()->id(),
                   reducedPressureMomentumPredictorForceDensity(*elem_info, Moose::currentState()))
               .first;
    rhs.add(dof_indices[0], it->second(system_i) * NS::cellMeasure(*elem_info));
  }
  rhs.close();
}
