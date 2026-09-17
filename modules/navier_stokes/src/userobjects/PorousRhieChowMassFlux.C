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
#include "MooseEnum.h"
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
  params.addParam<bool>("use_interpolated_density_in_bernoulli_jump",
                        false,
                        "Use the face-interpolated density in the reversible Bernoulli jump term "
                        "instead of the owner/non-owner side densities.");
  params.addParam<bool>("use_interpolated_density_in_form_loss",
                        false,
                        "Use the face-interpolated density when forming the reference velocity "
                        "for the irreversible baffle form-loss term.");
  params.addParam<std::vector<Real>>(
      "baffle_form_loss",
      std::vector<Real>(),
      "Per-baffle form-loss coefficient K values aligned with pressure_baffle_sidesets.");
  MultiMooseEnum velocity_form_loss("higher_epsilon lower_epsilon");
  params.addParam<MultiMooseEnum>(
      "velocity_form_loss",
      velocity_form_loss,
      "Per-baffle velocity-side selection aligned with pressure_baffle_sidesets. "
      "Allowed values: higher_epsilon, lower_epsilon.");
  params.addParam<bool>("debug_baffle", false, "Enable debug output for baffle jumps.");
  MooseEnum reconstruction_quantity("velocity mass", "velocity");
  params.addParam<MooseEnum>(
      "flux_velocity_reconstruction_quantity",
      reconstruction_quantity,
      "Quantity reconstructed by FVReconstructedPressureGradient. "
      "'velocity' reconstructs face velocity m_dot_f/rho_f directly. 'mass' reconstructs rho*u "
      "from m_dot_f and divides by the cell density.");
  params.addParam<std::vector<BoundaryName>>(
      "flux_velocity_reconstruction_zero_flux_sidesets",
      {},
      "Boundary sidesets where the flux-based cell velocity reconstruction enforces zero normal "
      "velocity (symmetry/slip).");
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
    _use_interpolated_density_in_bernoulli_jump(
        getParam<bool>("use_interpolated_density_in_bernoulli_jump")),
    _use_interpolated_density_in_form_loss(getParam<bool>("use_interpolated_density_in_form_loss")),
    _debug_baffle(getParam<bool>("debug_baffle")),
    _use_mass_based_flux_velocity_reconstruction(
        getParam<MooseEnum>("flux_velocity_reconstruction_quantity") == "mass"),
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
  const auto & baffle_form_loss = getParam<std::vector<Real>>("baffle_form_loss");
  const auto & velocity_form_loss = getParam<MultiMooseEnum>("velocity_form_loss");

  if (!baffle_form_loss.empty() && baffle_form_loss.size() != baffle_names.size())
    paramError("baffle_form_loss", "Must have the same length as pressure_baffle_sidesets.");
  if (params.isParamSetByUser("velocity_form_loss") &&
      velocity_form_loss.size() != baffle_names.size())
    paramError("velocity_form_loss", "Must have the same length as pressure_baffle_sidesets.");
  const auto baffle_ids = _moose_mesh.getBoundaryIDs(baffle_names);
  _pressure_baffle_boundary_ids.insert(baffle_ids.begin(), baffle_ids.end());
  const auto limiter_ids = _moose_mesh.getBoundaryIDs(limiter_names);
  _pressure_gradient_limiter_ids.insert(limiter_ids.begin(), limiter_ids.end());
  const auto zero_flux_ids = _moose_mesh.getBoundaryIDs(zero_flux_names);
  _reconstruction_zero_flux_boundary_ids.insert(zero_flux_ids.begin(), zero_flux_ids.end());

  if (params.isParamSetByUser("velocity_form_loss") && baffle_form_loss.empty())
    paramError("velocity_form_loss", "Requires baffle_form_loss to be provided.");

  if (!baffle_form_loss.empty())
  {
    if (baffle_names.empty())
      paramError("pressure_baffle_sidesets", "Must be provided when baffle_form_loss is set.");

    for (const auto & val : baffle_form_loss)
      if (val < 0.0)
        paramError("baffle_form_loss", "All entries must be >= 0.");

    for (const auto i : index_range(baffle_names))
    {
      const auto bnd_id = _moose_mesh.getBoundaryID(baffle_names[i]);

      _pressure_baffle_form_loss_by_id[bnd_id] = baffle_form_loss[i];

      _pressure_baffle_form_loss_use_higher_eps_by_id[bnd_id] =
          params.isParamSetByUser("velocity_form_loss")
              ? (velocity_form_loss[i] == "higher_epsilon")
              : false;
    }
  }
}

void
PorousRhieChowMassFlux::meshChanged()
{
  RhieChowMassFlux::meshChanged();
  _baffle_jump.clear();
  _p_grad_flux.clear();
}

void
PorousRhieChowMassFlux::initialize()
{
  RhieChowMassFlux::initialize();

  for (const auto & pair : _baffle_jump)
    _baffle_jump[pair.first] = 0.0;

  for (const auto & pair : _p_grad_flux)
    _p_grad_flux[pair.first] = 0.0;
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
  for (auto & fi : _fe_problem.mesh().faceInfo())
  {
    _baffle_jump[fi->id()];
    _p_grad_flux[fi->id()];
  }

  RhieChowMassFlux::initFaceMassFlux();
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

  if (!_debug_baffle)
    return;

  const auto time_arg = Moose::currentState();
  PetscVectorReader p_reader(*_pressure_system->system().current_local_solution);

  // Collect porous cells that touch the limited interface
  std::unordered_set<dof_id_type> porous_interface_cells;
  for (auto & fi : _flow_face_info)
  {
    if (!fi->neighborPtr())
      continue;
    if (!isPressureGradientLimited(*fi))
      continue;

    if (fi->elem().subdomain_id() == 2)
      porous_interface_cells.insert(fi->elem().id());
    if (fi->neighbor().subdomain_id() == 2)
      porous_interface_cells.insert(fi->neighbor().id());
  }

  for (auto & fi : _flow_face_info)
  {
    if (!fi->neighborPtr())
      continue;
    if (!hasBlocks(fi->elemPtr()->subdomain_id()) || !hasBlocks(fi->neighborPtr()->subdomain_id()))
      continue;

    const bool interface_face = isPressureGradientLimited(*fi);

    const bool first_porous_interior_face = fi->elem().subdomain_id() == 2 &&
                                            fi->neighbor().subdomain_id() == 2 &&
                                            (porous_interface_cells.count(fi->elem().id()) ||
                                             porous_interface_cells.count(fi->neighbor().id()));

    if (!interface_face && !first_porous_interior_face)
      continue;

    const auto & elem_info = *fi->elemInfo();
    const auto & neigh_info = *fi->neighborInfo();

    const auto elem_pdof = elem_info.dofIndices()[_global_pressure_system_number][0];
    const auto neigh_pdof = neigh_info.dofIndices()[_global_pressure_system_number][0];

    const Real p_elem = p_reader(elem_pdof);
    const Real p_neigh = p_reader(neigh_pdof);

    const Real rho_elem = _rho(makeElemArg(fi->elemPtr()), time_arg);
    const Real rho_neigh = _rho(makeElemArg(fi->neighborPtr()), time_arg);

    const Real eps_elem = _eps(makeElemArg(fi->elemPtr()), time_arg);
    const Real eps_neigh = _eps(makeElemArg(fi->neighborPtr()), time_arg);

    const Real phi = _face_mass_flux.evaluate(fi);

    const Real U_n_elem = (rho_elem != 0.0) ? phi / rho_elem : 0.0;
    const Real U_n_neigh = (rho_neigh != 0.0) ? phi / rho_neigh : 0.0;

    const Real u_n_elem = (rho_elem != 0.0 && eps_elem != 0.0) ? phi / (rho_elem * eps_elem) : 0.0;
    const Real u_n_neigh =
        (rho_neigh != 0.0 && eps_neigh != 0.0) ? phi / (rho_neigh * eps_neigh) : 0.0;

    const Real bernoulli_elem_minus_neigh =
        0.5 * (rho_neigh * u_n_neigh * u_n_neigh - rho_elem * u_n_elem * u_n_elem);

    const auto upw = getAdvectedInterpolationCoeffs(
        *fi, Moose::FV::InterpMethod::Upwind, phi, /*apply_porosity_scaling=*/true);

    RealVectorValue U_elem_vec(0.0);
    RealVectorValue U_neigh_vec(0.0);
    for (const auto d : make_range(_dim))
    {
      U_elem_vec(d) = _vel[d]->getElemValue(elem_info, time_arg);
      U_neigh_vec(d) = _vel[d]->getElemValue(neigh_info, time_arg);
    }

    // Normal component of the actual advected interstitial state used by the embedded 1/eps form
    const Real u_adv_n = (upw.first * U_elem_vec + upw.second * U_neigh_vec) * fi->normal();

    _console << (interface_face ? "IFACE" : "POROUS_1ST") << " face=" << fi->id()
             << " elem=" << fi->elem().id() << " neigh=" << fi->neighbor().id()
             << " elem_blk=" << fi->elem().subdomain_id()
             << " neigh_blk=" << fi->neighbor().subdomain_id() << " phi=" << phi
             << " HbyA_flux=" << _HbyA_flux[fi->id()] << " p_grad_flux=" << _p_grad_flux[fi->id()]
             << " p_elem=" << p_elem << " p_neigh=" << p_neigh
             << " dp_elem_minus_neigh=" << (p_elem - p_neigh) << " rho_elem=" << rho_elem
             << " rho_neigh=" << rho_neigh << " eps_elem=" << eps_elem << " eps_neigh=" << eps_neigh
             << " U_n_elem=" << U_n_elem << " U_n_neigh=" << U_n_neigh << " u_n_elem=" << u_n_elem
             << " u_n_neigh=" << u_n_neigh
             << " bernoulli_elem_minus_neigh=" << bernoulli_elem_minus_neigh << " upw=("
             << upw.first << "," << upw.second << ")"
             << " u_adv_n=" << u_adv_n << " phi_u_adv_n=" << (phi * u_adv_n) << std::endl;
  }
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
  const bool elem_is_owner = elemIsBaffleOwner(fi);
  // J is stored as (p_non_owner - p_owner), so the owner side sees -J.
  return (elem_side == elem_is_owner) ? -J : J;
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

    const Real eps_elem = _eps(makeElemArg(elem), time_arg);
    const Real eps_neighbor = _eps(makeElemArg(neighbor), time_arg);
    const bool elem_is_owner = elemIsBaffleOwner(*fi);

    const Real phi = _face_mass_flux.evaluate(fi);
    const Real U_n = face_rho != 0.0 ? phi / face_rho : 0.0;

    // The momentum predictor keeps the ordinary conservative advection treatment while the
    // pressure-baffle operator carries the reversible Bernoulli jump. By default that jump uses
    // the side-local densities; optionally it can use the face-interpolated density on both sides.
    const Real u_elem = (elem_rho != 0.0 && eps_elem != 0.0) ? phi / (elem_rho * eps_elem) : 0.0;
    const Real u_neighbor =
        (neighbor_rho != 0.0 && eps_neighbor != 0.0) ? phi / (neighbor_rho * eps_neighbor) : 0.0;
    const Real rho_owner = elem_is_owner ? elem_rho : neighbor_rho;
    const Real rho_non_owner = elem_is_owner ? neighbor_rho : elem_rho;
    const Real eps_owner = elem_is_owner ? eps_elem : eps_neighbor;
    const Real eps_non_owner = elem_is_owner ? eps_neighbor : eps_elem;
    const Real bernoulli_rho_owner =
        _use_interpolated_density_in_bernoulli_jump ? face_rho : rho_owner;
    const Real bernoulli_rho_non_owner =
        _use_interpolated_density_in_bernoulli_jump ? face_rho : rho_non_owner;
    Real J_new = 0.0;

    if (_use_interpolated_density_in_bernoulli_jump)
    {
      const Real u_owner = (eps_owner != 0.0) ? U_n / eps_owner : 0.0;
      const Real u_non_owner = (eps_non_owner != 0.0) ? U_n / eps_non_owner : 0.0;

      J_new = 0.5 * face_rho * (u_owner * u_owner - u_non_owner * u_non_owner);
    }
    else
    {
      const Real u_owner =
          (rho_owner != 0.0 && eps_owner != 0.0) ? phi / (rho_owner * eps_owner) : 0.0;
      const Real u_non_owner = (rho_non_owner != 0.0 && eps_non_owner != 0.0)
                                   ? phi / (rho_non_owner * eps_non_owner)
                                   : 0.0;

      J_new = 0.5 * (rho_owner * u_owner * u_owner - rho_non_owner * u_non_owner * u_non_owner);
    }

    Real J_loss = 0.0;
    Real form_loss = 0.0;
    bool use_higher_eps = false;

    BoundaryID baffle_id = Moose::INVALID_BOUNDARY_ID;
    for (const auto & bnd_id : fi->boundaryIDs())
      if (_pressure_baffle_boundary_ids.count(bnd_id))
      {
        baffle_id = bnd_id;
        break;
      }

    if (baffle_id != Moose::INVALID_BOUNDARY_ID)
    {
      auto it_k = _pressure_baffle_form_loss_by_id.find(baffle_id);
      if (it_k != _pressure_baffle_form_loss_by_id.end())
        form_loss = it_k->second;

      auto it_side = _pressure_baffle_form_loss_use_higher_eps_by_id.find(baffle_id);
      if (it_side != _pressure_baffle_form_loss_use_higher_eps_by_id.end())
        use_higher_eps = it_side->second;
    }

    if (form_loss > 0.0 && U_n != 0.0)
    {
      const bool pick_elem =
          use_higher_eps ? (eps_elem >= eps_neighbor) : (eps_elem <= eps_neighbor);
      const Real eps_ref = pick_elem ? eps_elem : eps_neighbor;
      const Real u_ref = _use_interpolated_density_in_form_loss
                             ? ((eps_ref != 0.0) ? U_n / eps_ref : 0.0)
                             : (pick_elem ? u_elem : u_neighbor);
      const Real flow_sign = U_n > 0.0 ? 1.0 : -1.0;
      J_loss = -flow_sign * 0.5 * form_loss * face_rho * u_ref * u_ref;
      J_new += J_loss;
    }

    const Real J_old = _baffle_jump[fi->id()];

    _baffle_jump[fi->id()] =
        _pressure_baffle_relaxation * J_new + (1.0 - _pressure_baffle_relaxation) * J_old;

    if (_debug_baffle)
    {
      const auto elem_bnds = _moose_mesh.getBoundaryIDs(fi->elemPtr(), fi->elemSideID());
      const auto neigh_bnds = _moose_mesh.getBoundaryIDs(fi->neighborPtr(), fi->neighborSideID());
      _console << "Baffle jump face " << fi->id() << " elem_block=" << fi->elem().subdomain_id()
               << " neigh_block=" << fi->neighbor().subdomain_id()
               << " elem_is_owner=" << elem_is_owner << " elem_bnds=" << stringify_bnds(elem_bnds)
               << " neigh_bnds=" << stringify_bnds(neigh_bnds) << " phi=" << phi << " U_n=" << U_n
               << " rho_f=" << face_rho << " eps_elem=" << eps_elem << " eps_neigh=" << eps_neighbor
               << " eps_owner=" << eps_owner << " eps_non_owner=" << eps_non_owner
               << " rho_owner=" << rho_owner << " rho_non_owner=" << rho_non_owner
               << " bernoulli_rho_owner=" << bernoulli_rho_owner
               << " bernoulli_rho_non_owner=" << bernoulli_rho_non_owner << " K=" << form_loss
               << " J_new=" << J_new << " J_loss=" << J_loss << " J_old=" << J_old
               << " J=" << _baffle_jump[fi->id()] << std::endl;
    }
  }
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

Real
PorousRhieChowMassFlux::reconstructionFaceNormalVelocity(const FaceInfo & fi,
                                                         const ElemInfo & elem_info,
                                                         const Point & face_normal) const
{
  if (!fi.neighborPtr() && isReconstructionZeroFluxFace(fi))
    return 0.0;

  if (!_use_mass_based_flux_velocity_reconstruction)
    return RhieChowMassFlux::reconstructionFaceNormalVelocity(fi, elem_info, face_normal);

  const Point flux_normal =
      hasBlocks(fi.elemPtr()->subdomain_id()) ? fi.normal() : Point(-fi.normal());
  const Real cell_density = _rho(makeElemArg(elem_info.elem()), Moose::currentState());
  return cell_density != 0.0 ? getMassFlux(fi) * (flux_normal * face_normal) / cell_density : 0.0;
}

bool
PorousRhieChowMassFlux::faceUsesOneSidedReconstruction(const FaceInfo & fi) const
{
  if (isPressureGradientLimited(fi))
    return true;

  if (!fi.neighborPtr() || !isBaffleFace(fi))
    return false;

  const auto time_arg = Moose::currentState();
  const Real eps_elem = _eps(makeElemArg(fi.elemPtr()), time_arg);
  const Real eps_neighbor = _eps(makeElemArg(fi.neighborPtr()), time_arg);
  const bool porosity_jump = std::abs(eps_elem - eps_neighbor) > TOLERANCE;

  return porosity_jump || !_use_interpolated_density_in_bernoulli_jump;
}
