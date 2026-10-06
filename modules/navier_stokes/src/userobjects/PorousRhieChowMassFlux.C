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
#include "LinearSystem.h"
#include "PressureJumpModel.h"

using namespace libMesh;

registerMooseObject("NavierStokesApp", PorousRhieChowMassFlux);

InputParameters
PorousRhieChowMassFlux::validParams()
{
  InputParameters params = RhieChowMassFlux::validParams();
  params.addClassDescription(
      "Rhie-Chow mass flux object specialized for porous flow/baffle cases.");
  params.set<MooseEnum>("pressure_diffusion_interpolation") = "harmonic";

  params.addParam<MooseFunctorName>(
      NS::porosity, "1", "Porosity functor (defaults to 1 for non-porous calculations).");
  params.addParam<std::vector<UserObjectName>>(
      "pressure_jump_models", {}, "Pressure jump models applied by this object.");
  params.addRangeCheckedParam<Real>("pressure_jump_relaxation",
                                    1.0,
                                    "0.0<pressure_jump_relaxation<=1.0",
                                    "Under-relaxation factor for pressure jump updates.");
  params.addParam<std::vector<BoundaryName>>(
      "pressure_gradient_limiter",
      {},
      "Sidesets on which the pressure gradient uses a one-term expansion.");

  return params;
}

PorousRhieChowMassFlux::PorousRhieChowMassFlux(const InputParameters & params)
  : RhieChowMassFlux(params),
    _eps(getFunctor<Real>(NS::porosity)),
    _pressure_jump_relaxation(getParam<Real>("pressure_jump_relaxation")),
    _baffle_jump(
        declareRestartableData<FaceCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>>>(
            "baffle_jump", _moose_mesh, blockIDs(), "baffle_jump"))
{
  const auto & limiter_names = getParam<std::vector<BoundaryName>>("pressure_gradient_limiter");
  const auto limiter_ids = _moose_mesh.getBoundaryIDs(limiter_names);
  _pressure_gradient_limiter_ids.insert(limiter_ids.begin(), limiter_ids.end());

  std::unordered_set<BoundaryID> pressure_jump_boundary_ids;
  for (const auto & model_name : getParam<std::vector<UserObjectName>>("pressure_jump_models"))
  {
    const auto & model = getUserObjectByName<PressureJumpModel>(model_name);
    for (const auto boundary_id : model.boundaryIDs())
      if (!pressure_jump_boundary_ids.insert(boundary_id).second)
        paramError("pressure_jump_models",
                   "Pressure jump models must not act on overlapping boundaries.");
    _pressure_jump_models.push_back(&model);
  }
}

void
PorousRhieChowMassFlux::meshChanged()
{
  RhieChowMassFlux::meshChanged();
  _baffle_jump.clear();
  ++_baffle_jump_generation;
}

void
PorousRhieChowMassFlux::initialize()
{
  RhieChowMassFlux::initialize();

  for (const auto & pair : _baffle_jump)
    _baffle_jump[pair.first] = 0.0;
  ++_baffle_jump_generation;
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
  const auto * const pressure_jump_model = getPressureJumpModel(fi);
  if (!pressure_jump_model)
    return 0.0;

  const Real J = _baffle_jump.evaluate(&fi);
  const bool elem_is_owner = pressure_jump_model->elemIsOwner(fi);
  // J is stored as (p_non_owner - p_owner), so the owner side sees -J.
  return (elem_side == elem_is_owner) ? -J : J;
}

void
PorousRhieChowMassFlux::updateBaffleJumps()
{
  if (_pressure_jump_models.empty())
    return;

  bool updated = false;
  for (auto & fi : _flow_face_info)
  {
    const auto * const pressure_jump_model = getPressureJumpModel(*fi);
    if (!pressure_jump_model || !fi->neighborPtr())
      continue;
    if (!hasBlocks(fi->elemPtr()->subdomain_id()) || !hasBlocks(fi->neighborPtr()->subdomain_id()))
      continue;

    const Real new_jump =
        pressure_jump_model->computePressureJump(*fi, _face_mass_flux.evaluate(fi));
    _baffle_jump[fi->id()] = _pressure_jump_relaxation * new_jump +
                             (1.0 - _pressure_jump_relaxation) * _baffle_jump[fi->id()];
    updated = true;
  }

  if (updated)
    ++_baffle_jump_generation;
}

bool
PorousRhieChowMassFlux::isBaffleFace(const FaceInfo & fi) const
{
  return getPressureJumpModel(fi) != nullptr;
}

const PressureJumpModel *
PorousRhieChowMassFlux::getPressureJumpModel(const FaceInfo & fi) const
{
  for (const auto * const model : _pressure_jump_models)
    if (model->appliesTo(fi))
      return model;

  return nullptr;
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
PorousRhieChowMassFlux::faceUsesOneSidedReconstruction(const FaceInfo & fi) const
{
  if (isPressureGradientLimited(fi))
    return true;

  if (!fi.neighborPtr() || !isBaffleFace(fi))
    return false;

  return getPressureJumpModel(fi)->useOneSidedReconstruction(fi);
}
