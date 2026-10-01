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
#include "LinearSystem.h"

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
}

void
PorousRhieChowMassFlux::initialize()
{
  RhieChowMassFlux::initialize();

  for (const auto & pair : _baffle_jump)
    _baffle_jump[pair.first] = 0.0;
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
    _baffle_jump[fi->id()];

  RhieChowMassFlux::initFaceMassFlux();
}

void
PorousRhieChowMassFlux::initCouplingField()
{
  RhieChowMassFlux::initCouplingField();

  for (auto & fi : _fe_problem.mesh().faceInfo())
    _baffle_jump[fi->id()];
}

void
PorousRhieChowMassFlux::applyCellPorosityScaling(NumericVector<Number> & vec) const
{
  if (_cell_porosity)
    vec.pointwise_mult(vec, *_cell_porosity);
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

    if (elem_rho <= 0.0 || neighbor_rho <= 0.0)
      mooseError(name(), ": density must be positive on baffle face ", fi->id(), ".");
    if (eps_elem <= 0.0 || eps_neighbor <= 0.0)
      mooseError(name(), ": porosity must be positive on baffle face ", fi->id(), ".");

    const Real phi = _face_mass_flux.evaluate(fi);
    const Real U_n = phi / face_rho;

    // The momentum predictor keeps the ordinary conservative advection treatment while the
    // pressure-baffle operator carries the reversible Bernoulli jump. By default that jump uses
    // the side-local densities; optionally it can use the face-interpolated density on both sides.
    const Real rho_owner = elem_is_owner ? elem_rho : neighbor_rho;
    const Real rho_non_owner = elem_is_owner ? neighbor_rho : elem_rho;
    const Real eps_owner = elem_is_owner ? eps_elem : eps_neighbor;
    const Real eps_non_owner = elem_is_owner ? eps_neighbor : eps_elem;
    Real J_new = 0.0;

    if (_use_interpolated_density_in_bernoulli_jump)
    {
      const Real u_owner = U_n / eps_owner;
      const Real u_non_owner = U_n / eps_non_owner;

      J_new = 0.5 * face_rho * (u_owner * u_owner - u_non_owner * u_non_owner);
    }
    else
    {
      const Real u_owner = phi / (rho_owner * eps_owner);
      const Real u_non_owner = phi / (rho_non_owner * eps_non_owner);

      J_new = 0.5 * (rho_owner * u_owner * u_owner - rho_non_owner * u_non_owner * u_non_owner);
    }

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
      const Real u_ref =
          pick_elem ? phi / (elem_rho * eps_elem) : phi / (neighbor_rho * eps_neighbor);
      const Real flow_sign = U_n > 0.0 ? 1.0 : -1.0;
      J_new -= flow_sign * 0.5 * form_loss * face_rho * u_ref * u_ref;
    }

    _baffle_jump[fi->id()] = _pressure_baffle_relaxation * J_new +
                             (1.0 - _pressure_baffle_relaxation) * _baffle_jump[fi->id()];
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
