//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConservativeSharpInterfaceVOFMULESCorrector.h"

#include "LinearFVBoundaryCondition.h"
#include "LinearFVAdvectionDiffusionExtrapolatedBC.h"
#include "LinearFVAdvectionDiffusionFunctorDirichletBC.h"
#include "LinearFVInletOutletScalarBC.h"
#include "LinearSystem.h"
#include "MooseLinearVariableFV.h"
#include "MooseFunctorArguments.h"
#include "FEProblemBase.h"
#include "SubProblem.h"
#include "MooseMesh.h"
#include "FaceInfo.h"
#include "ElemInfo.h"
#include "MathFVUtils.h"
#include "MathUtils.h"
#include "Limiter.h"

#include "timpi/parallel_sync.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <unordered_map>
#include <vector>

registerMooseObject("NavierStokesApp", ConservativeSharpInterfaceVOFMULESCorrector);

namespace
{
constexpr Real alpha_min = 0.0;
constexpr Real alpha_max = 1.0;
constexpr Real correction_relaxation = 1.0;
constexpr Real first_correction_relaxation = 1.0;
constexpr Real later_correction_relaxation = 0.5;
}

InputParameters
ConservativeSharpInterfaceVOFMULESCorrector::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params += NonADFunctorInterface::validParams();
  params += BlockRestrictable::validParams();
  params.addClassDescription(
      "Applies an explicit bounded correction to a donor/upwind volume-fraction transport solve.");
  params.addRequiredParam<SolverSystemName>("system_name",
                                            "The linear system transporting the volume fraction.");
  params.addRequiredParam<VariableName>("variable", "The transported volume-fraction variable.");
  params.addRequiredParam<MooseFunctorName>(
      "face_flux", "Volumetric face-flux functor used by alpha/rhoPhi correction.");
  params.addRequiredParam<MooseFunctorName>("liquid_density",
                                            "Liquid density functor for rhoPhi accumulation.");
  params.addRequiredParam<MooseFunctorName>("gas_density",
                                            "Gas density functor for rhoPhi accumulation.");
  params.addParam<MooseFunctorName>(
      "compression_factor",
      "0",
      "Compression coefficient cAlpha used in the explicit compression correction.");
  params.addRequiredParam<MooseFunctorName>(
      "interface_normal",
      "Face-oriented interface unit normal used in the explicit compression correction.");
  params.addRangeCheckedParam<unsigned int>(
      "n_alpha_corrections",
      2,
      "n_alpha_corrections>=0",
      "Number of explicit correction sweeps. Zero publishes the donor alpha flux without an "
      "explicit correction sweep.");
  params.addRangeCheckedParam<unsigned int>(
      "n_limiter_iterations",
      3,
      "n_limiter_iterations>0",
      "Number of limiter tightening passes for each correction sweep.");
  ExecFlagEnum & exec_enum = params.set<ExecFlagEnum>("execute_on", true);
  exec_enum.addAvailableFlags(EXEC_NONE);
  exec_enum = {EXEC_NONE};
  params.suppressParameter<ExecFlagEnum>("execute_on");
  return params;
}

ConservativeSharpInterfaceVOFMULESCorrector::ConservativeSharpInterfaceVOFMULESCorrector(
    const InputParameters & params)
  : GeneralUserObject(params),
    NonADFunctorInterface(this),
    BlockRestrictable(this),
    _system_name(getParam<SolverSystemName>("system_name")),
    _variable_name(getParam<VariableName>("variable")),
    _face_flux(getFunctor<Real>("face_flux")),
    _compression_factor(getFunctor<Real>("compression_factor")),
    _interface_normal(getFunctor<RealVectorValue>("interface_normal")),
    _liquid_density(getFunctor<Real>("liquid_density")),
    _gas_density(getFunctor<Real>("gas_density")),
    _num_alpha_corrections(getParam<unsigned int>("n_alpha_corrections")),
    _num_limiter_iterations(getParam<unsigned int>("n_limiter_iterations")),
    _alpha_phi_limited(_fe_problem.mesh(), blockIDs(), "alpha_phi_limited"),
    _rho_phi(_fe_problem.mesh(), blockIDs(), "rho_phi")
{
  for (const auto tid : make_range(libMesh::n_threads()))
  {
    UserObject::_subproblem.addFunctor("alpha_phi_limited", _alpha_phi_limited, tid);
    UserObject::_subproblem.addFunctor("rho_phi", _rho_phi, tid);
  }
}

void
ConservativeSharpInterfaceVOFMULESCorrector::initialSetup()
{
  cacheSystemData();
  initializeFluxStorage();
}

void
ConservativeSharpInterfaceVOFMULESCorrector::meshChanged()
{
  cacheSystemData();
  initializeFluxStorage();
}

void
ConservativeSharpInterfaceVOFMULESCorrector::cacheSystemData()
{
  _system = &_fe_problem.getLinearSystem(_fe_problem.linearSysNum(_system_name));
  _alpha_var = dynamic_cast<MooseLinearVariableFVReal *>(
      &UserObject::_subproblem.getVariable(0, _variable_name));

  if (!_alpha_var)
    paramError("variable", "The volume-fraction variable must be a MooseLinearVariableFVReal.");

  _sys_num = _alpha_var->sys().number();
  _var_num = _alpha_var->number();
}

void
ConservativeSharpInterfaceVOFMULESCorrector::initializeFluxStorage()
{
  for (const auto * fi : _fe_problem.mesh().faceInfo())
  {
    _alpha_phi_limited[fi->id()] = 0.0;
    _rho_phi[fi->id()] = 0.0;
  }
}

Real
ConservativeSharpInterfaceVOFMULESCorrector::cellVolume(const ElemInfo & elem_info) const
{
  return elem_info.volume() * elem_info.coordFactor();
}

Real
ConservativeSharpInterfaceVOFMULESCorrector::faceMeasure(const FaceInfo & fi) const
{
  return fi.faceArea() * fi.faceCoord();
}

Real
ConservativeSharpInterfaceVOFMULESCorrector::cellAlpha(const ElemInfo & elem_info) const
{
  return _alpha_var->getElemValue(elem_info, Moose::currentState());
}

Real
ConservativeSharpInterfaceVOFMULESCorrector::boundedAlpha(const Real value) const
{
  return std::min(alpha_max, std::max(alpha_min, value));
}

void
ConservativeSharpInterfaceVOFMULESCorrector::resetSubcycleFluxes()
{
  for (auto & pair : _alpha_phi_limited)
    pair.second = 0.0;
  for (auto & pair : _rho_phi)
    pair.second = 0.0;
}

void
ConservativeSharpInterfaceVOFMULESCorrector::refreshPublishedRhoPhi()
{
  if (!_system || !_alpha_var)
    return;

  for (const auto * fi : _fe_problem.mesh().faceInfo())
  {
    if (!fi)
      continue;

    const auto face_type = fi->faceType(std::make_pair(_var_num, _sys_num));
    if (face_type == FaceInfo::VarFaceNeighbors::NEITHER)
      continue;

    if (!hasBlocks(fi->elemSubdomainID()) &&
        !(fi->neighborPtr() && hasBlocks(fi->neighborSubdomainID())))
      continue;

    const auto face_id = fi->id();
    const Real limited_alpha_flux =
        _alpha_phi_limited.count(face_id) ? libmesh_map_find(_alpha_phi_limited, face_id) : 0.0;
    _rho_phi[face_id] = rhoPhi(*fi, limited_alpha_flux);
  }
}

LinearFVBoundaryCondition *
ConservativeSharpInterfaceVOFMULESCorrector::boundaryCondition(const FaceInfo & fi) const
{
  for (const auto bnd_id : fi.boundaryIDs())
    if (const auto & bc_map = _alpha_var->getBoundaryConditionMap();
        bc_map.find(bnd_id) != bc_map.end())
      return bc_map.at(bnd_id);

  return nullptr;
}

Real
ConservativeSharpInterfaceVOFMULESCorrector::boundaryValue(
    const FaceInfo & fi, FaceInfo::VarFaceNeighbors face_type) const
{
  if (auto * bc = boundaryCondition(fi))
  {
    bc->setupFaceData(&fi, face_type);
    return boundedAlpha(bc->computeBoundaryValue());
  }

  const auto * fluid_info =
      face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR ? fi.neighborInfo() : fi.elemInfo();
  return fluid_info ? cellAlpha(*fluid_info) : 0.0;
}

bool
ConservativeSharpInterfaceVOFMULESCorrector::hasFaceSide(const FaceInfo & fi,
                                                         const bool fi_elem_side) const
{
  const auto face_type = fi.faceType(std::make_pair(_var_num, _sys_num));
  if (fi_elem_side)
    return face_type == FaceInfo::VarFaceNeighbors::ELEM ||
           face_type == FaceInfo::VarFaceNeighbors::BOTH;
  else
    return face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR ||
           face_type == FaceInfo::VarFaceNeighbors::BOTH;
}

Moose::FaceArg
ConservativeSharpInterfaceVOFMULESCorrector::functorFaceArg(const Moose::Functor<Real> & functor,
                                                            const FaceInfo & fi) const
{
  auto face_arg = makeCDFace(fi);
  const auto on_elem = functor.hasFaceSide(fi, true);
  const auto on_neighbor = functor.hasFaceSide(fi, false);

  if (on_elem && on_neighbor)
    face_arg.face_side = nullptr;
  else if (on_elem)
    face_arg.face_side = fi.elemPtr();
  else if (on_neighbor)
    face_arg.face_side = fi.neighborPtr();
  else
    mooseError(
        "The functor '", functor.functorName(), "' is not defined on either side of the face.");

  return face_arg;
}

Real
ConservativeSharpInterfaceVOFMULESCorrector::donorFlux(const FaceInfo & fi) const
{
  const auto face_type = fi.faceType(std::make_pair(_var_num, _sys_num));
  if (face_type == FaceInfo::VarFaceNeighbors::NEITHER)
    return 0.0;

  const Real volumetric_flux = vofTransportVolumetricFaceFlux(fi);
  const bool upwind_is_elem = volumetric_flux >= 0.0;
  const Real elem_alpha = cellAlpha(*fi.elemInfo());

  Real donor_alpha = elem_alpha;
  if (fi.neighborPtr() && face_type == FaceInfo::VarFaceNeighbors::BOTH)
    donor_alpha = upwind_is_elem ? elem_alpha : cellAlpha(*fi.neighborInfo());
  else if (auto * inlet_outlet_bc =
               dynamic_cast<LinearFVInletOutletScalarBC *>(boundaryCondition(fi)))
  {
    inlet_outlet_bc->setupFaceData(&fi, face_type);
    donor_alpha = upwind_is_elem
                      ? elem_alpha
                      : boundedAlpha(inlet_outlet_bc->computeBoundaryValue(/* backflow = */ true));
  }
  else if (_alpha_var->isDirichletBoundaryFace(fi))
    donor_alpha = upwind_is_elem ? elem_alpha : boundaryValue(fi, face_type);

  return volumetric_flux * donor_alpha * faceMeasure(fi);
}

Real
ConservativeSharpInterfaceVOFMULESCorrector::highOrderFaceValue(const FaceInfo & fi) const
{
  const auto face_type = fi.faceType(std::make_pair(_var_num, _sys_num));
  if (face_type == FaceInfo::VarFaceNeighbors::NEITHER)
    return 0.0;

  const Real volumetric_flux = vofTransportVolumetricFaceFlux(fi);
  const bool upwind_is_elem = volumetric_flux >= 0.0;
  auto * bc = boundaryCondition(fi);

  Real high_order_alpha = 0.0;
  if (fi.neighborPtr() && face_type == FaceInfo::VarFaceNeighbors::BOTH)
    high_order_alpha = sharedVanLeerFaceValue(fi, upwind_is_elem);
  else if (auto * dirichlet_bc = dynamic_cast<LinearFVAdvectionDiffusionFunctorDirichletBC *>(bc))
  {
    (void)dirichlet_bc;
    high_order_alpha = boundaryValue(fi, face_type);
  }
  else if (auto * inlet_outlet_bc = dynamic_cast<LinearFVInletOutletScalarBC *>(bc))
  {
    inlet_outlet_bc->setupFaceData(&fi, face_type);
    high_order_alpha =
        boundedAlpha(inlet_outlet_bc->computeBoundaryValue(/* backflow = */ !upwind_is_elem));
  }
  else if (upwind_is_elem && dynamic_cast<LinearFVAdvectionDiffusionExtrapolatedBC *>(bc))
    high_order_alpha = boundaryValue(fi, face_type);
  else
    return std::abs(volumetric_flux) > libMesh::TOLERANCE
               ? donorFlux(fi) / (volumetric_flux * faceMeasure(fi))
               : cellAlpha(*fi.elemInfo());

  return high_order_alpha;
}

Real
ConservativeSharpInterfaceVOFMULESCorrector::sharedVanLeerFaceValue(const FaceInfo & fi,
                                                                    const bool upwind_is_elem) const
{
  mooseAssert(fi.neighborPtr(), "Van Leer correction requires an internal face with a neighbor.");

  const auto state = Moose::currentState();
  const auto & upwind_info = upwind_is_elem ? *fi.elemInfo() : *fi.neighborInfo();
  const auto & downwind_info = upwind_is_elem ? *fi.neighborInfo() : *fi.elemInfo();

  const Real phi_upwind = cellAlpha(upwind_info);
  const Real phi_downwind = cellAlpha(downwind_info);
  const VectorValue<Real> grad_upwind = _alpha_var->gradSln(upwind_info, state);
  const auto limiter = Moose::FV::Limiter<Real>::build(Moose::FV::LimiterType::VanLeer);
  const Real phi_face =
      Moose::FV::interpolate(*limiter, phi_upwind, phi_downwind, &grad_upwind, fi, upwind_is_elem);
  return boundedAlpha(phi_face);
}

ConservativeSharpInterfaceVOFMULESCorrector::BoundaryFaceKind
ConservativeSharpInterfaceVOFMULESCorrector::classifyBoundaryFace(
    const FaceInfo & fi,
    const FaceInfo::VarFaceNeighbors face_type,
    const Real volumetric_flux) const
{
  if (fi.neighborPtr() && face_type == FaceInfo::VarFaceNeighbors::BOTH)
    return BoundaryFaceKind::Internal;

  if (std::abs(volumetric_flux) <= libMesh::TOLERANCE)
    return BoundaryFaceKind::Closed;

  auto * bc = boundaryCondition(fi);
  if (!bc)
    return BoundaryFaceKind::Closed;

  if (dynamic_cast<LinearFVAdvectionDiffusionFunctorDirichletBC *>(bc))
    return volumetric_flux >= 0.0 ? BoundaryFaceKind::DirichletOutflow
                                  : BoundaryFaceKind::DirichletInflow;

  if (dynamic_cast<LinearFVInletOutletScalarBC *>(bc))
    return volumetric_flux >= 0.0 ? BoundaryFaceKind::OpenOutflow
                                  : BoundaryFaceKind::DirichletInflow;

  if (dynamic_cast<LinearFVAdvectionDiffusionExtrapolatedBC *>(bc))
    return volumetric_flux >= 0.0 ? BoundaryFaceKind::OpenOutflow : BoundaryFaceKind::Closed;

  return BoundaryFaceKind::Closed;
}

ConservativeSharpInterfaceVOFMULESCorrector::AlphaFluxData
ConservativeSharpInterfaceVOFMULESCorrector::buildAlphaFlux(
    const FaceInfo & fi,
    const Real elem_alpha,
    const Real neighbor_alpha,
    const BoundaryFaceKind boundary_kind) const
{
  AlphaFluxData flux;

  const Real face_phi = vofTransportVolumetricFaceFlux(fi);
  const Real face_measure = faceMeasure(fi);
  const Real integrated_phi = face_phi * face_measure;

  flux.donor_flux = donorFlux(fi);
  const Real high_order_flux = integrated_phi * highOrderFaceValue(fi);
  Real total_flux = flux.donor_flux;

  if (boundary_kind == BoundaryFaceKind::Closed)
    return flux;

  if (boundary_kind != BoundaryFaceKind::Internal)
  {
    if (boundary_kind == BoundaryFaceKind::DirichletOutflow)
      total_flux = high_order_flux;

    flux.correction_flux = total_flux - flux.donor_flux;
    return flux;
  }

  const auto state = Moose::currentState();
  const Moose::FaceArg face_arg{
      &fi, Moose::FV::LimiterType::CentralDifference, true, false, nullptr, nullptr};

  const RealVectorValue face_normal = fi.normal();
  const Real face_normal_mag = face_normal.norm();
  const RealVectorValue face_unit_normal =
      face_normal_mag > 0.0 ? face_normal / face_normal_mag : RealVectorValue();
  const RealVectorValue interface_normal =
      MetaPhysicL::raw_value(_interface_normal(face_arg, state));
  const Real interface_normal_alignment = interface_normal * face_unit_normal;

  if (std::abs(integrated_phi) > libMesh::TOLERANCE)
  {
    const Real bounded_elem_alpha = boundedAlpha(elem_alpha);
    const Real bounded_neighbor_alpha = boundedAlpha(neighbor_alpha);
    const Real linear_alpha =
        boundedAlpha(fi.gC() * bounded_elem_alpha + (1.0 - fi.gC()) * bounded_neighbor_alpha);
    const Real high_order_alpha = highOrderFaceValue(fi);
    const Real compression_factor = MetaPhysicL::raw_value(_compression_factor(face_arg, state));
    const Real compressed_alpha = boundedAlpha(
        high_order_alpha + compression_factor * MathUtils::sign(integrated_phi) * linear_alpha *
                               (1.0 - linear_alpha) * interface_normal_alignment);

    total_flux = integrated_phi * compressed_alpha;
  }
  else
    total_flux = high_order_flux;

  flux.correction_flux = total_flux - flux.donor_flux;
  return flux;
}

Real
ConservativeSharpInterfaceVOFMULESCorrector::faceFunctorAverage(
    const FaceInfo & fi, const Moose::Functor<Real> & functor) const
{
  const auto state = Moose::currentState();
  return MetaPhysicL::raw_value(functor(functorFaceArg(functor, fi), state));
}

Real
ConservativeSharpInterfaceVOFMULESCorrector::vofTransportVolumetricFaceFlux(
    const FaceInfo & fi) const
{
  return _face_flux(functorFaceArg(_face_flux, fi), Moose::currentState());
}

Real
ConservativeSharpInterfaceVOFMULESCorrector::rhoPhi(const FaceInfo & fi,
                                                    const Real limited_alpha_flux) const
{
  const Real gas_density = faceFunctorAverage(fi, _gas_density);
  const Real liquid_density = faceFunctorAverage(fi, _liquid_density);
  const Real volumetric_mass_flux =
      vofTransportVolumetricFaceFlux(fi) * faceMeasure(fi) * gas_density;
  return volumetric_mass_flux + (liquid_density - gas_density) * limited_alpha_flux;
}

ConservativeSharpInterfaceVOFMULESCorrector::FaceCorrectionData
ConservativeSharpInterfaceVOFMULESCorrector::buildFaceCorrectionData(const FaceInfo & fi) const
{
  FaceCorrectionData data;
  const auto face_type = fi.faceType(std::make_pair(_var_num, _sys_num));
  if (face_type == FaceInfo::VarFaceNeighbors::NEITHER)
    return data;

  data.face = &fi;
  data.elem_dof = fi.elemInfo()->dofIndices()[_sys_num][_var_num];
  data.has_neighbor = fi.neighborPtr() && face_type == FaceInfo::VarFaceNeighbors::BOTH &&
                      fi.neighborInfo()->dofIndices()[_sys_num][_var_num] != DofObject::invalid_id;
  if (data.has_neighbor)
    data.neighbor_dof = fi.neighborInfo()->dofIndices()[_sys_num][_var_num];

  const Real volumetric_flux = vofTransportVolumetricFaceFlux(fi);
  data.elem_alpha = cellAlpha(*fi.elemInfo());

  data.boundary_kind = classifyBoundaryFace(fi, face_type, volumetric_flux);
  data.neighbor_alpha = data.boundary_kind == BoundaryFaceKind::Internal
                            ? cellAlpha(*fi.neighborInfo())
                            : boundaryValue(fi, face_type);
  const auto flux = buildAlphaFlux(fi, data.elem_alpha, data.neighbor_alpha, data.boundary_kind);
  data.donor_flux = flux.donor_flux;
  data.correction_flux = flux.correction_flux;

  return data;
}

std::vector<ConservativeSharpInterfaceVOFMULESCorrector::FaceCorrectionData>
ConservativeSharpInterfaceVOFMULESCorrector::collectFaceCorrectionData() const
{
  std::vector<FaceCorrectionData> face_corrections;
  face_corrections.reserve(_fe_problem.mesh().faceInfo().size());

  for (const auto * fi : _fe_problem.mesh().faceInfo())
  {
    const auto face_type = fi->faceType(std::make_pair(_var_num, _sys_num));
    if (face_type == FaceInfo::VarFaceNeighbors::NEITHER)
      continue;
    if (!hasBlocks(fi->elemSubdomainID()) &&
        !(fi->neighborPtr() && hasBlocks(fi->neighborSubdomainID())))
      continue;

    auto data = buildFaceCorrectionData(*fi);
    if (data.face)
      face_corrections.push_back(data);
  }

  return face_corrections;
}

void
ConservativeSharpInterfaceVOFMULESCorrector::publishFaceFluxes(
    const std::vector<FaceCorrectionData> & face_corrections,
    const std::vector<Real> & accumulated_alpha_fluxes,
    const Real subcycle_fraction)
{
  for (const auto i : index_range(face_corrections))
  {
    const auto & data = face_corrections[i];
    const auto face_id = data.face->id();
    const Real limited_alpha_flux = accumulated_alpha_fluxes[i];
    const Real rho_phi = rhoPhi(*data.face, limited_alpha_flux);

    // Accumulate the published face fluxes with the same subcycle weighting so downstream
    // consumers see a timestep-consistent alphaPhi/rhoPhi pair after subcycling.
    _alpha_phi_limited[face_id] += subcycle_fraction * limited_alpha_flux;
    _rho_phi[face_id] += subcycle_fraction * rho_phi;
  }
}

bool
ConservativeSharpInterfaceVOFMULESCorrector::partitionFace(const FaceCorrectionData & data) const
{
  return data.has_neighbor && data.face->elemInfo()->elem()->processor_id() !=
                                  data.face->neighborInfo()->elem()->processor_id();
}

bool
ConservativeSharpInterfaceVOFMULESCorrector::locallyOwnedCell(const ElemInfo & elem_info) const
{
  return elem_info.elem()->processor_id() == processor_id();
}

bool
ConservativeSharpInterfaceVOFMULESCorrector::synchronizePartitionFaceLimiters(
    const std::vector<FaceCorrectionData> & face_corrections,
    std::vector<Real> & accepted_lambda) const
{
  if (n_processors() == 1)
    return false;

  using Datum = std::pair<dof_id_type, Real>;
  std::unordered_map<processor_id_type, std::vector<Datum>> push_data;

  for (const auto i : index_range(face_corrections))
    if (partitionFace(face_corrections[i]))
    {
      const auto & data = face_corrections[i];
      const auto elem_pid = data.face->elemInfo()->elem()->processor_id();
      const auto neighbor_pid = data.face->neighborInfo()->elem()->processor_id();
      const auto other_pid = elem_pid == processor_id() ? neighbor_pid : elem_pid;
      push_data[other_pid].push_back(std::make_pair(data.face->id(), accepted_lambda[i]));
    }

  std::unordered_map<dof_id_type, Real> received_minimum_lambda_by_face;
  auto receive_limiters = [&received_minimum_lambda_by_face](const processor_id_type,
                                                             const std::vector<Datum> & sent_data)
  {
    for (const auto & [face_id, lambda] : sent_data)
    {
      auto it = received_minimum_lambda_by_face.find(face_id);
      if (it == received_minimum_lambda_by_face.end())
        received_minimum_lambda_by_face.emplace(face_id, lambda);
      else
        it->second = std::min(it->second, lambda);
    }
  };

  TIMPI::push_parallel_vector_data(_communicator, push_data, receive_limiters);

  bool changed_lambda = false;
  for (const auto i : index_range(face_corrections))
    if (partitionFace(face_corrections[i]))
    {
      const auto face_id = face_corrections[i].face->id();
      const auto it = received_minimum_lambda_by_face.find(face_id);
      if (it == received_minimum_lambda_by_face.end())
        continue;

      const Real updated_lambda = std::min(accepted_lambda[i], it->second);
      changed_lambda = changed_lambda || updated_lambda + libMesh::TOLERANCE < accepted_lambda[i];
      accepted_lambda[i] = updated_lambda;
    }

  return changed_lambda;
}

void
ConservativeSharpInterfaceVOFMULESCorrector::applyCorrection(const Real dt,
                                                             const Real subcycle_fraction)
{
  if (!_system || !_alpha_var || dt <= 0.0)
    return;

  std::unordered_map<dof_id_type, Real> working_alpha_flux;

  if (_num_alpha_corrections == 0)
  {
    _system->computeGradients();

    const auto face_corrections = collectFaceCorrectionData();

    if (face_corrections.empty())
      return;

    std::vector<Real> donor_flux(face_corrections.size(), 0.0);
    for (const auto i : index_range(face_corrections))
      donor_flux[i] = face_corrections[i].donor_flux;

    publishFaceFluxes(face_corrections, donor_flux, subcycle_fraction);
    _system->computeGradients();
    return;
  }

  for (const auto correction_it : make_range(_num_alpha_corrections))
  {
    (void)correction_it;
    _system->computeGradients();

    const auto face_corrections = collectFaceCorrectionData();

    if (face_corrections.empty())
      return;

    auto & current_local_solution = *(_system->system().current_local_solution);
    std::unordered_map<dof_id_type, Real> applied_change;
    std::vector<Real> raw_correction_flux(face_corrections.size(), 0.0);
    std::vector<Real> limited_correction_flux(face_corrections.size(), 0.0);
    std::vector<Real> accepted_lambda(face_corrections.size(), 0.0);
    std::vector<Real> accumulated_alpha_flux(face_corrections.size(), 0.0);
    std::vector<Real> target_alpha_flux(face_corrections.size(), 0.0);

    for (const auto i : index_range(face_corrections))
    {
      const auto & data = face_corrections[i];
      const auto face_id = data.face->id();
      if (!working_alpha_flux.count(face_id))
        working_alpha_flux.emplace(face_id, data.donor_flux);

      target_alpha_flux[i] = data.donor_flux + data.correction_flux;
      raw_correction_flux[i] = target_alpha_flux[i] - libmesh_map_find(working_alpha_flux, face_id);
      accepted_lambda[i] = std::abs(raw_correction_flux[i]) > libMesh::TOLERANCE ? 1.0 : 0.0;
    }

    std::unordered_map<dof_id_type, Real> local_upper_bound;
    std::unordered_map<dof_id_type, Real> local_lower_bound;

    const auto initialize_local_bounds = [&](const dof_id_type dof)
    {
      if (local_upper_bound.count(dof))
        return;

      local_upper_bound.emplace(dof, alpha_min);
      local_lower_bound.emplace(dof, alpha_max);
    };

    const auto widen_local_bounds = [&](const dof_id_type dof, const Real alpha)
    {
      initialize_local_bounds(dof);
      const Real bounded_alpha = boundedAlpha(alpha);
      local_upper_bound[dof] = std::max(local_upper_bound[dof], bounded_alpha);
      local_lower_bound[dof] = std::min(local_lower_bound[dof], bounded_alpha);
    };

    for (const auto & data : face_corrections)
    {
      initialize_local_bounds(data.elem_dof);
      if (data.has_neighbor)
        initialize_local_bounds(data.neighbor_dof);

      if (data.boundary_kind == BoundaryFaceKind::Internal)
      {
        widen_local_bounds(data.elem_dof, data.neighbor_alpha);
        widen_local_bounds(data.neighbor_dof, data.elem_alpha);
      }
      else if (data.boundary_kind == BoundaryFaceKind::DirichletInflow ||
               data.boundary_kind == BoundaryFaceKind::DirichletOutflow)
        widen_local_bounds(data.elem_dof, data.neighbor_alpha);
    }

    for (auto & pair : local_upper_bound)
      pair.second = std::min(pair.second, alpha_max);
    for (auto & pair : local_lower_bound)
      pair.second = std::max(pair.second, alpha_min);

    std::unordered_map<dof_id_type, Real> cell_volume_by_dof;
    for (const auto & data : face_corrections)
    {
      cell_volume_by_dof.emplace(data.elem_dof, cellVolume(*data.face->elemInfo()));
      if (data.has_neighbor)
        cell_volume_by_dof.emplace(data.neighbor_dof, cellVolume(*data.face->neighborInfo()));
    }

    std::unordered_map<dof_id_type, Real> psi_maxn;
    std::unordered_map<dof_id_type, Real> psi_minn;
    std::unordered_map<dof_id_type, Real> sum_phip;
    std::unordered_map<dof_id_type, Real> m_sum_phim;

    const auto initialize_cmules_cell = [&](const dof_id_type dof)
    {
      if (psi_maxn.count(dof))
        return;

      const Real alpha = current_local_solution(dof);
      const Real cell_volume = libmesh_map_find(cell_volume_by_dof, dof);
      psi_maxn.emplace(dof, cell_volume * std::max(0.0, local_upper_bound[dof] - alpha) / dt);
      psi_minn.emplace(dof, cell_volume * std::max(0.0, alpha - local_lower_bound[dof]) / dt);
      sum_phip.emplace(dof, 0.0);
      m_sum_phim.emplace(dof, 0.0);
    };

    for (const auto & data : face_corrections)
    {
      initialize_cmules_cell(data.elem_dof);
      if (data.has_neighbor)
        initialize_cmules_cell(data.neighbor_dof);
    }

    for (const auto i : index_range(face_corrections))
    {
      const auto & data = face_corrections[i];
      const Real phi_corr_f = raw_correction_flux[i];
      if (std::abs(phi_corr_f) < libMesh::TOLERANCE)
        continue;

      if (phi_corr_f > 0.0)
      {
        sum_phip[data.elem_dof] += phi_corr_f;
        if (data.has_neighbor)
          m_sum_phim[data.neighbor_dof] += phi_corr_f;
      }
      else
      {
        m_sum_phim[data.elem_dof] -= phi_corr_f;
        if (data.has_neighbor)
          sum_phip[data.neighbor_dof] -= phi_corr_f;
      }
    }

    const auto clamp_limiter = [](const Real value) { return std::min(1.0, std::max(0.0, value)); };

    const Real root_v_small = std::sqrt(std::numeric_limits<Real>::min());

    for (const auto limiter_it : make_range(_num_limiter_iterations))
    {
      (void)limiter_it;
      std::unordered_map<dof_id_type, Real> sum_l_phip;
      std::unordered_map<dof_id_type, Real> m_sum_l_phim;

      for (const auto & pair : psi_maxn)
        sum_l_phip.emplace(pair.first, 0.0);
      for (const auto & pair : psi_minn)
        m_sum_l_phim.emplace(pair.first, 0.0);

      for (const auto i : index_range(face_corrections))
      {
        const auto & data = face_corrections[i];
        const Real phi_corr_f = accepted_lambda[i] * raw_correction_flux[i];
        if (std::abs(phi_corr_f) < libMesh::TOLERANCE)
          continue;

        if (phi_corr_f > 0.0)
        {
          sum_l_phip[data.elem_dof] += phi_corr_f;
          if (data.has_neighbor)
            m_sum_l_phim[data.neighbor_dof] += phi_corr_f;
        }
        else
        {
          m_sum_l_phim[data.elem_dof] -= phi_corr_f;
          if (data.has_neighbor)
            sum_l_phip[data.neighbor_dof] -= phi_corr_f;
        }
      }

      std::unordered_map<dof_id_type, Real> lambda_minus;
      std::unordered_map<dof_id_type, Real> lambda_plus;
      for (const auto & pair : psi_maxn)
      {
        const auto dof = pair.first;
        lambda_minus[dof] =
            clamp_limiter((sum_l_phip[dof] + psi_maxn[dof]) / (m_sum_phim[dof] + root_v_small));
        lambda_plus[dof] =
            clamp_limiter((m_sum_l_phim[dof] + psi_minn[dof]) / (sum_phip[dof] + root_v_small));
      }

      bool changed_lambda = false;
      for (const auto i : index_range(face_corrections))
      {
        const auto & data = face_corrections[i];
        const Real phi_corr_f = raw_correction_flux[i];
        if (std::abs(phi_corr_f) < libMesh::TOLERANCE || accepted_lambda[i] <= 0.0)
          continue;

        Real lambda = accepted_lambda[i];

        if (data.boundary_kind == BoundaryFaceKind::Internal)
        {
          if (phi_corr_f > 0.0)
            lambda = std::min(
                lambda, std::min(lambda_plus[data.elem_dof], lambda_minus[data.neighbor_dof]));
          else
            lambda = std::min(
                lambda, std::min(lambda_minus[data.elem_dof], lambda_plus[data.neighbor_dof]));
        }
        else
        {
          const Real corrected_boundary_flux = target_alpha_flux[i] + phi_corr_f;
          if (corrected_boundary_flux > libMesh::TOLERANCE * libMesh::TOLERANCE)
          {
            if (phi_corr_f > 0.0)
              lambda = std::min(lambda, lambda_plus[data.elem_dof]);
            else
              lambda = std::min(lambda, lambda_minus[data.elem_dof]);
          }
        }

        const Real updated_lambda = std::min(accepted_lambda[i], clamp_limiter(lambda));
        changed_lambda = changed_lambda || updated_lambda + libMesh::TOLERANCE < accepted_lambda[i];
        accepted_lambda[i] = updated_lambda;
      }

      changed_lambda =
          synchronizePartitionFaceLimiters(face_corrections, accepted_lambda) || changed_lambda;

      unsigned int changed_anywhere = changed_lambda ? 1 : 0;
      _communicator.max(changed_anywhere);
      if (!changed_anywhere)
        break;
    }

    for (const auto i : index_range(face_corrections))
      if (std::abs(raw_correction_flux[i]) > libMesh::TOLERANCE)
      {
        const Real correction_weight =
            correction_it == 0 ? first_correction_relaxation : later_correction_relaxation;
        limited_correction_flux[i] =
            correction_weight * correction_relaxation * accepted_lambda[i] * raw_correction_flux[i];
        const auto & data = face_corrections[i];
        const Real bounded_elem_delta =
            -dt * limited_correction_flux[i] / cellVolume(*data.face->elemInfo());
        if (locallyOwnedCell(*data.face->elemInfo()))
          applied_change[data.elem_dof] += bounded_elem_delta;

        if (data.has_neighbor)
        {
          const Real bounded_neighbor_delta =
              dt * limited_correction_flux[i] / cellVolume(*data.face->neighborInfo());
          if (locallyOwnedCell(*data.face->neighborInfo()))
            applied_change[data.neighbor_dof] += bounded_neighbor_delta;
        }
      }

    auto limited_update = current_local_solution.zero_clone();
    for (const auto & pair : applied_change)
      limited_update->add(pair.first, pair.second);
    limited_update->close();

    current_local_solution.add(*limited_update);
    current_local_solution.close();
    _system->computeGradients();

    for (const auto i : index_range(face_corrections))
    {
      const auto & data = face_corrections[i];
      const auto face_id = data.face->id();
      working_alpha_flux[face_id] += limited_correction_flux[i];
      accumulated_alpha_flux[i] = working_alpha_flux[face_id];
    }

    if (correction_it + 1 == _num_alpha_corrections)
      publishFaceFluxes(face_corrections, accumulated_alpha_flux, subcycle_fraction);
  }

  _system->computeGradients();
}
