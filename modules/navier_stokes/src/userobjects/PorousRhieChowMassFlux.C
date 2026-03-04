//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousRhieChowMassFlux.h"

#include "MooseMesh.h"
#include "NS.h"
#include "MathFVUtils.h"
#include "FVUtils.h"
#include "PetscVectorReader.h"
#include "LinearSystem.h"

#include "libmesh/dense_matrix.h"
#include "libmesh/dense_vector.h"

#include <sstream>

using namespace libMesh;

registerMooseObject("NavierStokesApp", PorousRhieChowMassFlux);

InputParameters
PorousRhieChowMassFlux::validParams()
{
  InputParameters params = RhieChowMassFlux::validParams();
  params.addClassDescription(
      "Rhie-Chow mass flux object specialized for porous flow/baffle cases.");

  params.addParam<MooseFunctorName>(
      NS::porosity, "1", "Porosity functor (defaults to 1 for non-porous calculations).");
  params.addParam<std::vector<BoundaryName>>(
      "pressure_baffle_sidesets", {}, "Sidesets which define porous pressure baffles.");
  params.addRangeCheckedParam<Real>("pressure_baffle_relaxation",
                                    1.0,
                                    "0.0<pressure_baffle_relaxation<=1.0",
                                    "Under-relaxation factor for pressure baffle jump updates.");
  params.addParam<bool>("debug_baffle", false, "Enable debug output for baffle jumps.");
  params.addParam<bool>("use_flux_velocity_reconstruction",
                        false,
                        "Reconstruct cell velocity from corrected face fluxes "
                        "using an oscillation-free least-squares fit.");
  params.addRangeCheckedParam<Real>(
      "flux_velocity_reconstruction_relaxation",
      1.0,
      "0.0<flux_velocity_reconstruction_relaxation<=1.0",
      "Under-relaxation factor for flux-based velocity reconstruction.");
  params.addParam<std::vector<BoundaryName>>(
      "flux_velocity_reconstruction_zero_flux_sidesets",
      {},
      "Boundary sidesets where reconstruction enforces zero normal flux (symmetry/slip).");
  params.addParam<bool>("use_corrected_pressure_gradient",
                        true,
                        "Whether to use the baffle-corrected pressure gradient when forming HbyA.");
  params.addParam<std::vector<BoundaryName>>(
      "pressure_gradient_limiter",
      {},
      "Sidesets on which the pressure gradient uses a one-term expansion.");

  return params;
}

PorousRhieChowMassFlux::PorousRhieChowMassFlux(const InputParameters & params)
  : RhieChowMassFlux(params),
    _eps(getFunctor<Real>(NS::porosity)),
    _pressure_baffle_relaxation(getParam<Real>("pressure_baffle_relaxation")),
    _debug_baffle(getParam<bool>("debug_baffle")),
    _use_flux_velocity_reconstruction(getParam<bool>("use_flux_velocity_reconstruction")),
    _flux_velocity_reconstruction_relaxation(
        getParam<Real>("flux_velocity_reconstruction_relaxation")),
    _use_corrected_pressure_gradient(getParam<bool>("use_corrected_pressure_gradient")),
    _use_harmonic_Ainv_interp(isParamSetByUser(NS::porosity)),
    _p_grad_flux(_moose_mesh, blockIDs(), "p_grad_flux"),
    _baffle_jump(
        declareRestartableData<FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>>>(
            "baffle_jump", _moose_mesh, blockIDs(), "baffle_jump"))
{
  const auto & baffle_names = getParam<std::vector<BoundaryName>>("pressure_baffle_sidesets");
  const auto & limiter_names = getParam<std::vector<BoundaryName>>("pressure_gradient_limiter");
  const auto & zero_flux_names =
      getParam<std::vector<BoundaryName>>("flux_velocity_reconstruction_zero_flux_sidesets");
  const auto baffle_ids = _moose_mesh.getBoundaryIDs(baffle_names);
  _pressure_baffle_boundary_ids.insert(baffle_ids.begin(), baffle_ids.end());
  const auto limiter_ids = _moose_mesh.getBoundaryIDs(limiter_names);
  _pressure_gradient_limiter_ids.insert(limiter_ids.begin(), limiter_ids.end());
  const auto zero_flux_ids = _moose_mesh.getBoundaryIDs(zero_flux_names);
  _reconstruction_zero_flux_boundary_ids.insert(zero_flux_ids.begin(), zero_flux_ids.end());
}

void
PorousRhieChowMassFlux::meshChanged()
{
  RhieChowMassFlux::meshChanged();
  _baffle_jump.clear();
  _p_grad_flux.clear();
  _grad_p_corrected.clear();
  _grad_p_reconstructed.clear();
  _grad_w_prev.clear();
}

void
PorousRhieChowMassFlux::initialize()
{
  RhieChowMassFlux::initialize();

  for (const auto & pair : _baffle_jump)
    _baffle_jump[pair.first] = 0.0;

  for (const auto & pair : _p_grad_flux)
    _p_grad_flux[pair.first] = 0.0;

  for (auto & comp_vec : _grad_w_prev)
    for (auto & grad_vec : comp_vec)
      if (grad_vec)
        grad_vec->zero();

  for (auto & grad_vec : _grad_p_reconstructed)
    if (grad_vec)
      grad_vec->zero();
}

void
PorousRhieChowMassFlux::setupMeshInformation()
{
  RhieChowMassFlux::setupMeshInformation();

  _cell_porosity = _pressure_system->currentSolution()->zero_clone();
  const auto time_arg = Moose::currentState();
  for (const auto & elem_info : _fe_problem.mesh().elemInfoVector())
    if (hasBlocks(elem_info->subdomain_id()))
    {
      const auto elem_dof = elem_info->dofIndices()[_global_pressure_system_number][0];
      _cell_porosity->set(elem_dof, _eps(makeElemArg(elem_info->elem()), time_arg));
    }

  _cell_porosity->close();
}

void
PorousRhieChowMassFlux::initFaceMassFlux()
{
  using namespace Moose::FV;

  const auto time_arg = Moose::currentState();

  // Ensure coupling functors are initialized for all faces before any boundary pressure BC
  // queries (e.g., pressure flux BCs) are evaluated during gradient reconstruction.
  for (auto & fi : _fe_problem.mesh().faceInfo())
  {
    _HbyA_flux[fi->id()];
    _Ainv[fi->id()];
    _face_velocity[fi->id()];
    _baffle_jump[fi->id()];
    _p_grad_flux[fi->id()];
  }

  // We loop through the faces and compute the resulting face fluxes from the
  // initial conditions for velocity
  for (auto & fi : _flow_face_info)
  {
    RealVectorValue density_times_velocity;
    // On internal face we do a regular interpolation with geometric weights
    if (_vel[0]->isInternalFace(*fi))
    {
      const auto & elem_info = *fi->elemInfo();
      const auto & neighbor_info = *fi->neighborInfo();

      Real elem_rho = _rho(makeElemArg(fi->elemPtr()), time_arg);
      Real neighbor_rho = _rho(makeElemArg(fi->neighborPtr()), time_arg);

      for (const auto dim_i : index_range(_vel))
        interpolate(InterpMethod::Average,
                    density_times_velocity(dim_i),
                    _vel[dim_i]->getElemValue(elem_info, time_arg) * elem_rho,
                    _vel[dim_i]->getElemValue(neighbor_info, time_arg) * neighbor_rho,
                    *fi,
                    true);
    }
    // On the boundary, we just take the boundary values
    else
    {
      const bool elem_is_fluid = hasBlocks(fi->elemPtr()->subdomain_id());
      const Elem * const boundary_elem = elem_is_fluid ? fi->elemPtr() : fi->neighborPtr();

      // We need this multiplier in case the face is an internal face and
      const Real boundary_normal_multiplier = elem_is_fluid ? 1.0 : -1.0;
      const Moose::FaceArg boundary_face{
          fi, Moose::FV::LimiterType::CentralDifference, true, false, boundary_elem, nullptr};

      const Real face_rho = _rho(boundary_face, time_arg);
      for (const auto dim_i : index_range(_vel))
        density_times_velocity(dim_i) = boundary_normal_multiplier * face_rho *
                                        raw_value((*_vel[dim_i])(boundary_face, time_arg));
    }

    _face_mass_flux[fi->id()] = density_times_velocity * fi->normal();
  }

  updateFaceVelocityFromMassFlux();
  computePressureGradientFlux();
  computeCorrectedPressureGradient();
}

void
PorousRhieChowMassFlux::initCouplingField()
{
  RhieChowMassFlux::initCouplingField();

  for (auto & fi : _fe_problem.mesh().faceInfo())
  {
    _baffle_jump[fi->id()];
    _p_grad_flux[fi->id()];
  }
}

void
PorousRhieChowMassFlux::computeFaceMassFlux()
{
  RhieChowMassFlux::computeFaceMassFlux();
}

const std::vector<std::unique_ptr<NumericVector<Number>>> &
PorousRhieChowMassFlux::selectPressureGradient(const bool updated_pressure)
{
  const bool use_reconstructed =
      _use_flux_velocity_reconstruction && !_grad_p_reconstructed.empty();
  const bool use_corrected = _use_corrected_pressure_gradient && !_grad_p_corrected.empty();
  const auto & grad_container = _pressure_system->gradientContainer();
  const auto & source = use_reconstructed ? _grad_p_reconstructed
                                          : (use_corrected ? _grad_p_corrected : grad_container);

  if (updated_pressure)
  {
    _grad_p_current.clear();
    for (const auto & component : source)
      _grad_p_current.push_back(component->clone());
  }

  return _grad_p_current.empty() ? source : _grad_p_current;
}

void
PorousRhieChowMassFlux::storePressureGradientFlux(const FaceInfo & fi, Real p_grad_flux)
{
  _p_grad_flux[fi.id()] = p_grad_flux;
}

void
PorousRhieChowMassFlux::applyCellPorosityScaling(NumericVector<Number> & vec) const
{
  if (_cell_porosity)
    vec.pointwise_mult(vec, *_cell_porosity);
}

std::pair<Real, Real>
PorousRhieChowMassFlux::getAdvectedInterpolationCoeffs(const FaceInfo & fi,
                                                       Moose::FV::InterpMethod method,
                                                       Real face_mass_flux,
                                                       bool apply_porosity_scaling) const
{
  auto coeffs = Moose::FV::interpCoeffs(method, fi, /*one_is_elem=*/true, face_mass_flux);

  const auto time_arg = Moose::currentState();

  if (apply_porosity_scaling && fi.neighborPtr() && _eps.hasBlocks(fi.elem().subdomain_id()) &&
      _eps.hasBlocks(fi.neighborPtr()->subdomain_id()))
  {
    const Real eps_elem = _eps(makeElemArg(fi.elemPtr()), time_arg);
    const Real eps_neighbor = _eps(makeElemArg(fi.neighborPtr()), time_arg);
    coeffs.first /= eps_elem;
    coeffs.second /= eps_neighbor;
  }
  else if (apply_porosity_scaling)
  {
    const Real eps_elem = _eps(makeElemArg(fi.elemPtr()), time_arg);
    coeffs.first /= eps_elem;
    coeffs.second = 0.0;
  }

  return coeffs;
}

Real
PorousRhieChowMassFlux::getFaceSidePorosity(const FaceInfo & fi,
                                            bool elem_side,
                                            const Moose::StateArg & time) const
{
  const Elem * const elem = elem_side ? fi.elemPtr() : fi.neighborPtr();
  if (!elem)
    return 1.0;

  return _eps(makeElemArg(elem), time);
}

Real
PorousRhieChowMassFlux::getSignedBaffleJump(const FaceInfo & fi, bool elem_side) const
{
  if (!isBaffleFace(fi))
    return 0.0;

  const Real J = _baffle_jump.evaluate(&fi);
  // J is defined as (p_neighbor - p_elem), so the elem-side sees -J.
  return elem_side ? -J : J;
}

Real
PorousRhieChowMassFlux::pressureGradient(const ElemInfo & elem_info, unsigned int component) const
{
  if (component >= _dim)
    return 0.0;

  const auto dof = elem_info.dofIndices()[_global_pressure_system_number][0];
  if (_use_flux_velocity_reconstruction && !_grad_p_reconstructed.empty())
    return (*_grad_p_reconstructed[component])(dof);
  if (!_grad_p_corrected.empty())
    return (*_grad_p_corrected[component])(dof);

  const auto & grads = _pressure_system->gradientContainer();
  return (*grads[component])(dof);
}

void
PorousRhieChowMassFlux::updateBaffleJumps()
{
  if (_pressure_baffle_boundary_ids.empty())
    return;

  const auto time_arg = Moose::currentState();
  auto stringify_bnds = [](const std::vector<BoundaryID> & ids)
  {
    std::ostringstream os;
    os << "{";
    bool first = true;
    for (const auto id : ids)
    {
      if (!first)
        os << ",";
      os << id;
      first = false;
    }
    os << "}";
    return os.str();
  };

  for (auto & fi : _flow_face_info)
  {
    if (!isBaffleFace(*fi) || !fi->neighborPtr())
      continue;
    if (!hasBlocks(fi->elemPtr()->subdomain_id()) || !hasBlocks(fi->neighborPtr()->subdomain_id()))
      continue;

    const Elem * const elem = fi->elemPtr();
    const Elem * const neighbor = fi->neighborPtr();

    const auto elem_rho = _rho(makeElemArg(elem), time_arg);
    const auto neighbor_rho = _rho(makeElemArg(neighbor), time_arg);

    Real face_rho = 0.0;
    Moose::FV::interpolate(
        Moose::FV::InterpMethod::Average, face_rho, elem_rho, neighbor_rho, *fi, true);

    const Real phi = _face_mass_flux.evaluate(fi);
    const Real U_n = face_rho != 0.0 ? phi / face_rho : 0.0;

    const Real eps_elem = _eps(makeElemArg(elem), time_arg);
    const Real eps_neighbor = _eps(makeElemArg(neighbor), time_arg);

    const Real u_elem = U_n / eps_elem;
    const Real u_neighbor = U_n / eps_neighbor;

    const Real J_new =
        0.5 * (elem_rho * u_elem * u_elem - neighbor_rho * u_neighbor * u_neighbor);
    const Real J_old = _baffle_jump[fi->id()];

    _baffle_jump[fi->id()] =
        _pressure_baffle_relaxation * J_new + (1.0 - _pressure_baffle_relaxation) * J_old;

    if (_debug_baffle)
    {
      const auto elem_bnds = _moose_mesh.getBoundaryIDs(fi->elemPtr(), fi->elemSideID());
      const auto neigh_bnds = _moose_mesh.getBoundaryIDs(fi->neighborPtr(), fi->neighborSideID());
      _console << "Baffle jump face " << fi->id()
               << " elem_block=" << fi->elem().subdomain_id()
               << " neigh_block=" << fi->neighbor().subdomain_id()
               << " elem_bnds=" << stringify_bnds(elem_bnds)
               << " neigh_bnds=" << stringify_bnds(neigh_bnds) << " phi=" << phi << " U_n=" << U_n
               << " rho_f=" << face_rho << " eps_elem=" << eps_elem
               << " eps_neigh=" << eps_neighbor << " J_new=" << J_new << " J_old=" << J_old
               << " J=" << _baffle_jump[fi->id()] << std::endl;
    }
  }
}

void
PorousRhieChowMassFlux::computeCorrectedPressureGradient()
{
  if (!_pressure_system)
    return;

  if (_pressure_baffle_boundary_ids.empty() && _pressure_gradient_limiter_ids.empty())
  {
    _grad_p_corrected.clear();
    return;
  }

  const auto time_arg = Moose::currentState();

  if (_grad_p_corrected.empty())
  {
    _grad_p_corrected.resize(_dim);
    for (const auto i : make_range(_dim))
      _grad_p_corrected[i] = _pressure_system->currentSolution()->zero_clone();
  }

  for (const auto i : make_range(_dim))
    _grad_p_corrected[i]->zero();

  PetscVectorReader p_reader(*_pressure_system->system().current_local_solution);

  const auto & mesh = _fe_problem.mesh();
  const auto rz_radial_coord = mesh.getAxisymmetricRadialCoord();

  for (const auto & elem_info : mesh.elemInfoVector())
  {
    if (!hasBlocks(elem_info->subdomain_id()))
      continue;

    const Elem & elem = *elem_info->elem();
    const auto coord_type = mesh.getCoordSystem(elem.subdomain_id());
    const auto elem_dof = elem_info->dofIndices()[_global_pressure_system_number][0];

    RealVectorValue sum(0.0);

    auto act = [&](const Elem &,
                   const Elem * const neighbor,
                   const FaceInfo * const fi,
                   const Point & surface_vector,
                   const Real /*coord*/,
                   const bool elem_has_info)
    {
      const Elem * const sided_elem = elem_has_info ? fi->elemPtr() : fi->neighborPtr();
      const ElemInfo * const elem_info_face = elem_has_info ? fi->elemInfo() : fi->neighborInfo();
      const ElemInfo * const neighbor_info_face =
          elem_has_info ? fi->neighborInfo() : fi->elemInfo();

      const auto elem_dof_face = elem_info_face->dofIndices()[_global_pressure_system_number][0];
      const Real p_elem = p_reader(elem_dof_face);

      Real p_face = p_elem;

      const bool neighbor_active =
          neighbor && neighbor_info_face && hasBlocks(neighbor_info_face->subdomain_id());

      if (neighbor_active && !isPressureGradientLimited(*fi))
      {
        const auto neighbor_dof =
            neighbor_info_face->dofIndices()[_global_pressure_system_number][0];
        const Real p_neighbor = p_reader(neighbor_dof);
        const auto coeffs =
            Moose::FV::interpCoeffs(Moose::FV::InterpMethod::Average, *fi, elem_has_info);
        const Real jump_side = getSignedBaffleJump(*fi, elem_has_info);
        p_face = coeffs.first * p_elem + coeffs.second * (p_neighbor + jump_side);
        if (_debug_baffle && isBaffleFace(*fi))
          _console << "Baffle grad face " << fi->id() << " elem_has_info=" << elem_has_info
                   << " p_elem=" << p_elem << " p_neigh=" << p_neighbor
                   << " jump_side=" << jump_side << " p_face=" << p_face << std::endl;
      }
      else if (!neighbor_active)
      {
        const Moose::FaceArg face_arg{
            fi, Moose::FV::LimiterType::CentralDifference, true, false, sided_elem, nullptr};
        p_face = MetaPhysicL::raw_value((*_p)(face_arg, time_arg));
      }

      sum += surface_vector * p_face;
    };

    Moose::FV::loopOverElemFaceInfo(elem, _moose_mesh, act, coord_type, rz_radial_coord);

    const Real volume = (*_cell_volumes)(elem_dof);
    const Real inv_volume = volume != 0.0 ? 1.0 / volume : 0.0;

    for (const auto i : make_range(_dim))
      _grad_p_corrected[i]->set(elem_dof, sum(i) * inv_volume);
  }

  for (const auto i : make_range(_dim))
    _grad_p_corrected[i]->close();
}

bool
PorousRhieChowMassFlux::isBaffleFace(const FaceInfo & fi) const
{
  if (_pressure_baffle_boundary_ids.empty())
    return false;

  for (const auto & bnd_id : fi.boundaryIDs())
    if (_pressure_baffle_boundary_ids.count(bnd_id))
      return true;

  return false;
}

bool
PorousRhieChowMassFlux::elemIsBaffleOwner(const FaceInfo & fi) const
{
  if (!fi.neighborPtr())
    return true;

  bool elem_has_baffle = false;
  bool neighbor_has_baffle = false;

  const auto elem_bnd_ids = _moose_mesh.getBoundaryIDs(fi.elemPtr(), fi.elemSideID());
  for (const auto & bnd_id : elem_bnd_ids)
    if (_pressure_baffle_boundary_ids.count(bnd_id))
    {
      elem_has_baffle = true;
      break;
    }

  if (fi.neighborPtr())
  {
    const auto neighbor_bnd_ids = _moose_mesh.getBoundaryIDs(fi.neighborPtr(), fi.neighborSideID());
    for (const auto & bnd_id : neighbor_bnd_ids)
      if (_pressure_baffle_boundary_ids.count(bnd_id))
      {
        neighbor_has_baffle = true;
        break;
      }
  }

  if (elem_has_baffle != neighbor_has_baffle)
    return elem_has_baffle;

  return fi.elem().subdomain_id() <= fi.neighbor().subdomain_id();
}

bool
PorousRhieChowMassFlux::isPressureGradientLimited(const FaceInfo & fi) const
{
  if (_pressure_gradient_limiter_ids.empty())
    return false;

  for (const auto & bnd_id : fi.boundaryIDs())
    if (_pressure_gradient_limiter_ids.count(bnd_id))
      return true;

  return false;
}

bool
PorousRhieChowMassFlux::isReconstructionZeroFluxFace(const FaceInfo & fi) const
{
  if (_reconstruction_zero_flux_boundary_ids.empty())
    return false;

  for (const auto & bnd_id : fi.boundaryIDs())
    if (_reconstruction_zero_flux_boundary_ids.count(bnd_id))
      return true;

  return false;
}

void
PorousRhieChowMassFlux::updateGradPrevFromFaceVelocity()
{
  if (_grad_w_prev.empty())
    return;

  for (auto & comp_vec : _grad_w_prev)
    for (auto & grad_vec : comp_vec)
      grad_vec->zero();

  const auto time_arg = Moose::currentState();
  const auto & mesh = _fe_problem.mesh();
  const auto rz_radial_coord = mesh.getAxisymmetricRadialCoord();
  std::vector<unsigned int> var_nums;
  for (const auto comp_index : make_range(_dim))
    var_nums.push_back(
        _momentum_implicit_systems[comp_index]->variable_number(_vel[comp_index]->name()));

  for (const auto & elem_info : mesh.elemInfoVector())
  {
    if (!hasBlocks(elem_info->subdomain_id()))
      continue;

    const Elem & elem = *elem_info->elem();
    std::vector<RealVectorValue> sum(_dim, RealVectorValue());

    auto act = [&](const Elem &,
                   const Elem * const,
                   const FaceInfo * const fi,
                   const Point & surface_vector,
                   const Real /*coord*/,
                   const bool elem_has_info)
    {
      const ElemInfo * const elem_info_face = elem_has_info ? fi->elemInfo() : fi->neighborInfo();
      const ElemInfo * const neighbor_info_face =
          elem_has_info ? fi->neighborInfo() : fi->elemInfo();

      const bool neighbor_active =
          neighbor_info_face && hasBlocks(neighbor_info_face->subdomain_id());

      for (const auto comp_index : make_range(_dim))
      {
        const Real u_elem = _vel[comp_index]->getElemValue(*elem_info_face, time_arg);
        const Real u_neighbor = neighbor_active
                                    ? _vel[comp_index]->getElemValue(*neighbor_info_face, time_arg)
                                    : u_elem;
        const Real u_face = neighbor_active ? 0.5 * (u_elem + u_neighbor) : u_elem;
        sum[comp_index] += surface_vector * u_face;
      }
    };

    Moose::FV::loopOverElemFaceInfo(
        elem, _moose_mesh, act, mesh.getCoordSystem(elem.subdomain_id()), rz_radial_coord);

    const Real volume = elem_info->volume() * elem_info->coordFactor();
    const Real inv_volume = volume != 0.0 ? 1.0 / volume : 0.0;

    for (const auto comp_index : make_range(_dim))
    {
      const unsigned int system_number = _momentum_implicit_systems[comp_index]->number();
      const auto dof = elem.dof_number(system_number, var_nums[comp_index], 0);
      for (const auto dir_index : make_range(_dim))
        _grad_w_prev[comp_index][dir_index]->set(dof, sum[comp_index](dir_index) * inv_volume);
    }
  }

  for (auto & comp_vec : _grad_w_prev)
    for (auto & grad_vec : comp_vec)
      grad_vec->close();
}

void
PorousRhieChowMassFlux::computeCellVelocity()
{
  if (!_use_flux_velocity_reconstruction)
  {
    auto & pressure_gradient = selectPressureGradient(/*updated_pressure=*/true);

    for (const auto system_i : index_range(_momentum_implicit_systems))
    {
      auto working_vector = _Ainv_raw[system_i]->clone();
      working_vector->pointwise_mult(*working_vector, *pressure_gradient[system_i]);
      // _Ainv_raw already contains the porosity scaling
      working_vector->add(*_HbyA_raw[system_i]);
      working_vector->scale(-1.0);
      (*_momentum_implicit_systems[system_i]->solution) = *working_vector;
      _momentum_implicit_systems[system_i]->update();
      _momentum_systems[system_i]->setSolution(
          *_momentum_implicit_systems[system_i]->current_local_solution);
    }
    return;
  }

  updateFaceVelocityFromMassFlux();
  if (!_reconstruction_zero_flux_boundary_ids.empty())
    for (auto & fi : _flow_face_info)
      if (!_vel[0]->isInternalFace(*fi) && isReconstructionZeroFluxFace(*fi))
        _face_velocity[fi->id()] = RealVectorValue();

  if (!_pressure_baffle_boundary_ids.empty() || !_pressure_gradient_limiter_ids.empty())
    computeCorrectedPressureGradient();

  if (_grad_w_prev.empty())
  {
    _grad_w_prev.resize(_dim);
    for (const auto comp_index : make_range(_dim))
    {
      _grad_w_prev[comp_index].resize(_dim);
      for (const auto dir_index : make_range(_dim))
        _grad_w_prev[comp_index][dir_index] =
            _momentum_implicit_systems[comp_index]->current_local_solution->zero_clone();
    }
  }

  if (_grad_p_reconstructed.empty())
  {
    _grad_p_reconstructed.resize(_dim);
    for (const auto comp_index : make_range(_dim))
      _grad_p_reconstructed[comp_index] =
          _momentum_implicit_systems[comp_index]->current_local_solution->zero_clone();
  }
  for (auto & grad_vec : _grad_p_reconstructed)
    grad_vec->zero();

  std::vector<unsigned int> var_nums;
  for (const auto comp_index : make_range(_dim))
    var_nums.push_back(
        _momentum_implicit_systems[comp_index]->variable_number(_vel[comp_index]->name()));

  const auto time_arg = Moose::currentState();
  const auto & mesh = _fe_problem.mesh();
  const auto rz_radial_coord = mesh.getAxisymmetricRadialCoord();

  for (const auto & elem_info : mesh.elemInfoVector())
  {
    if (!hasBlocks(elem_info->subdomain_id()))
      continue;

    DenseMatrix<Real> M(_dim, _dim);
    DenseVector<Real> b_h(_dim);
    DenseVector<Real> b_p(_dim);
    M.zero();
    b_h.zero();
    b_p.zero();
    bool force_corrected_grad = false;

    const Elem & elem = *elem_info->elem();
    const auto p_dof = elem_info->dofIndices()[_global_pressure_system_number][0];

    auto act = [&](const Elem &,
                   const Elem * const neighbor,
                   const FaceInfo * const fi,
                   const Point & surface_vector,
                   const Real /*coord*/,
                   const bool elem_has_info)
    {
      if (!force_corrected_grad && (isBaffleFace(*fi) || isPressureGradientLimited(*fi)))
        force_corrected_grad = true;

      const Real magS = surface_vector.norm();
      if (magS == 0.0)
        return;

      const Elem * const elem_face = elem_has_info ? fi->elemPtr() : fi->neighborPtr();
      const Elem * const neighbor_face = elem_has_info ? fi->neighborPtr() : fi->elemPtr();

      const bool neighbor_active =
          neighbor && neighbor_face && hasBlocks(neighbor_face->subdomain_id());

      Real face_rho = 0.0;
      if (neighbor_active)
      {
        const Real elem_rho = _rho(makeElemArg(elem_face), time_arg);
        const Real neighbor_rho = _rho(makeElemArg(neighbor_face), time_arg);
        Moose::FV::interpolate(
            Moose::FV::InterpMethod::Average, face_rho, elem_rho, neighbor_rho, *fi, true);
      }
      else
      {
        const Moose::FaceArg boundary_face{
            fi, Moose::FV::LimiterType::CentralDifference, true, false, elem_face, nullptr};
        face_rho = _rho(boundary_face, time_arg);
      }

      const bool enforce_zero_flux = !neighbor_active && isReconstructionZeroFluxFace(*fi);

      const Real h_flux_raw = _HbyA_flux.evaluate(fi);
      const Real h_flux = elem_has_info ? h_flux_raw : -h_flux_raw;
      const Real p_grad_flux_raw = _p_grad_flux.evaluate(fi);
      const Real p_grad_flux = elem_has_info ? p_grad_flux_raw : -p_grad_flux_raw;

      const Real H = (enforce_zero_flux || face_rho == 0.0) ? 0.0 : h_flux / face_rho;
      const Real P = (enforce_zero_flux || face_rho == 0.0) ? 0.0 : p_grad_flux / face_rho;

      const Point dPf = fi->faceCentroid() - elem_info->centroid();

      std::vector<Real> grad_times_dpf(_dim, 0.0);
      if (!_grad_w_prev.empty())
      {
        for (const auto comp_index : make_range(_dim))
        {
          const unsigned int system_number = _momentum_implicit_systems[comp_index]->number();
          const unsigned int var_num = var_nums[comp_index];
          const auto elem_dof = elem_face->dof_number(system_number, var_num, 0);
          const auto neighbor_dof =
              neighbor_active ? neighbor_face->dof_number(system_number, var_num, 0) : elem_dof;

          Real comp_val = 0.0;
          for (const auto dir_index : make_range(_dim))
          {
            const Real grad_elem = (*_grad_w_prev[comp_index][dir_index])(elem_dof);
            const Real grad_neighbor =
                neighbor_active ? (*_grad_w_prev[comp_index][dir_index])(neighbor_dof) : grad_elem;
            const Real grad_face = neighbor_active ? 0.5 * (grad_elem + grad_neighbor) : grad_elem;
            comp_val += grad_face * dPf(dir_index);
          }
          grad_times_dpf[comp_index] = comp_val;
        }
      }

      Real corr = 0.0;
      for (const auto comp_index : make_range(_dim))
        corr += grad_times_dpf[comp_index] * surface_vector(comp_index);

      for (const auto i : make_range(_dim))
      {
        b_h(i) += ((H) / magS) * surface_vector(i);
        b_p(i) += ((P - corr) / magS) * surface_vector(i);
        for (const auto j : make_range(_dim))
          M(i, j) += surface_vector(i) * surface_vector(j) / magS;
      }
    };

    Moose::FV::loopOverElemFaceInfo(
        elem, _moose_mesh, act, mesh.getCoordSystem(elem.subdomain_id()), rz_radial_coord);

    DenseVector<Real> result_h(_dim);
    DenseVector<Real> result_p(_dim);
    if (_dim == 1)
    {
      const Real denom = M(0, 0);
      result_h(0) = denom != 0.0 ? b_h(0) / denom : 0.0;
      result_p(0) = denom != 0.0 ? b_p(0) / denom : 0.0;
    }
    else
    {
      M.cholesky_solve(b_h, result_h);
      M.cholesky_solve(b_p, result_p);
    }

    for (const auto comp_index : make_range(_dim))
    {
      const unsigned int system_number = _momentum_implicit_systems[comp_index]->number();
      const auto index = elem.dof_number(system_number, var_nums[comp_index], 0);
      const Real old_val = (*_momentum_implicit_systems[comp_index]->current_local_solution)(index);
      const Real alpha = _flux_velocity_reconstruction_relaxation;
      const Real recon_val = -result_h(comp_index) + result_p(comp_index);
      const Real new_val = (1.0 - alpha) * old_val + alpha * recon_val;
      _momentum_implicit_systems[comp_index]->solution->set(index, new_val);

      const Real HbyA = (*_HbyA_raw[comp_index])(index);
      const Real Ainv = (*_Ainv_raw[comp_index])(index);
      Real grad_tilde =
          Ainv != 0.0 ? (result_h(comp_index) - result_p(comp_index) - HbyA) / Ainv : 0.0;
      if (force_corrected_grad && _use_corrected_pressure_gradient && !_grad_p_corrected.empty())
        grad_tilde = (*_grad_p_corrected[comp_index])(p_dof);
      _grad_p_reconstructed[comp_index]->set(index, grad_tilde);
    }
  }

  for (const auto system_i : index_range(_momentum_implicit_systems))
  {
    _momentum_implicit_systems[system_i]->solution->close();
    _momentum_implicit_systems[system_i]->update();
    _momentum_systems[system_i]->setSolution(
        *_momentum_implicit_systems[system_i]->current_local_solution);
  }

  for (auto & grad_vec : _grad_p_reconstructed)
    grad_vec->close();

  updateGradPrevFromFaceVelocity();
}
