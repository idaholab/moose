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
  params.addParam<std::vector<VariableName>>(
      "confined_scalar_variables",
      {},
      "Scalar amount variables q = alpha c to transport conservatively with the limited "
      "volume-fraction flux.");
  params.addParam<MooseFunctorName>(
      "confined_scalar_backflow_concentration",
      "0",
      "Concentration imposed for confined scalar backflow on open volume-fraction boundaries.");
  params.addRangeCheckedParam<Real>(
      "confined_scalar_alpha_floor",
      1e-12,
      "confined_scalar_alpha_floor > 0",
      "Minimum alpha used to recover confined scalar concentration c = q / alpha.");
  params.addParam<Real>(
      "confined_scalar_concentration_min", 0.0, "Minimum allowed confined scalar concentration.");
  params.addParam<Real>(
      "confined_scalar_concentration_max", 1.0, "Maximum allowed confined scalar concentration.");
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
    _confined_scalar_variable_names(
        getParam<std::vector<VariableName>>("confined_scalar_variables")),
    _face_flux(getFunctor<Real>("face_flux")),
    _compression_factor(getFunctor<Real>("compression_factor")),
    _interface_normal(getFunctor<RealVectorValue>("interface_normal")),
    _liquid_density(getFunctor<Real>("liquid_density")),
    _gas_density(getFunctor<Real>("gas_density")),
    _confined_scalar_backflow_concentration(
        getFunctor<Real>("confined_scalar_backflow_concentration")),
    _confined_scalar_alpha_floor(getParam<Real>("confined_scalar_alpha_floor")),
    _confined_scalar_concentration_min(getParam<Real>("confined_scalar_concentration_min")),
    _confined_scalar_concentration_max(getParam<Real>("confined_scalar_concentration_max")),
    _num_alpha_corrections(getParam<unsigned int>("n_alpha_corrections")),
    _num_limiter_iterations(getParam<unsigned int>("n_limiter_iterations")),
    _alpha_phi_limited(_fe_problem.mesh(), blockIDs(), "alpha_phi_limited"),
    _rho_phi(_fe_problem.mesh(), blockIDs(), "rho_phi")
{
  if (_confined_scalar_concentration_max < _confined_scalar_concentration_min)
    paramError("confined_scalar_concentration_max",
               "The confined scalar concentration maximum must be greater than or equal to the "
               "minimum.");

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

  _confined_scalars.clear();
  for (const auto & scalar_name : _confined_scalar_variable_names)
  {
    auto * scalar_var = dynamic_cast<MooseLinearVariableFVReal *>(
        &UserObject::_subproblem.getVariable(0, scalar_name));

    if (!scalar_var)
      paramError("confined_scalar_variables",
                 "Confined scalar variable '",
                 scalar_name,
                 "' must be a MooseLinearVariableFVReal.");

    auto & scalar_system = _fe_problem.getLinearSystem(scalar_var->sys().number());
    _confined_scalars.push_back({scalar_name,
                                 scalar_var,
                                 &scalar_system,
                                 scalar_var->sys().number(),
                                 scalar_var->number()});
  }
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
    if (transportFaceType(*fi) == FaceInfo::VarFaceNeighbors::NEITHER)
      continue;

    const auto face_id = fi->id();
    const Real limited_alpha_flux = libmesh_map_find(_alpha_phi_limited, face_id);
    _rho_phi[face_id] = rhoPhi(*fi, limited_alpha_flux);
  }
}

Real
ConservativeSharpInterfaceVOFMULESCorrector::boundaryValue(
    const FaceInfo & fi, const FaceTransportData & face_data) const
{
  if (auto * bc = face_data.boundary_condition)
    return boundedAlpha(bc->computeBoundaryValue());

  const auto * fluid_info = face_data.face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR
                                ? fi.neighborInfo()
                                : fi.elemInfo();
  return fluid_info ? cellAlpha(*fluid_info) : 0.0;
}

FaceInfo::VarFaceNeighbors
ConservativeSharpInterfaceVOFMULESCorrector::transportFaceType(const FaceInfo & fi) const
{
  const auto face_type = fi.faceType(std::make_pair(_var_num, _sys_num));
  if (face_type == FaceInfo::VarFaceNeighbors::NEITHER)
    return face_type;

  return hasBlocks(fi.elemSubdomainID()) ||
                 (fi.neighborPtr() && hasBlocks(fi.neighborSubdomainID()))
             ? face_type
             : FaceInfo::VarFaceNeighbors::NEITHER;
}

ConservativeSharpInterfaceVOFMULESCorrector::FaceTransportData
ConservativeSharpInterfaceVOFMULESCorrector::faceTransportData(const FaceInfo & fi) const
{
  FaceTransportData data;
  data.face_type = transportFaceType(fi);
  if (data.face_type == FaceInfo::VarFaceNeighbors::NEITHER)
    return data;

  data.volumetric_flux = _face_flux(functorFaceArg(_face_flux, fi), Moose::currentState());
  data.integrated_flux = data.volumetric_flux * faceMeasure(fi);
  data.upwind_is_elem = data.volumetric_flux >= 0.0;
  data.boundary_condition = _alpha_var->getBoundaryCondition(fi);
  data.boundary_kind =
      classifyBoundaryFace(fi, data.face_type, data.volumetric_flux, data.boundary_condition);
  return data;
}

bool
ConservativeSharpInterfaceVOFMULESCorrector::hasFaceSide(const FaceInfo & fi,
                                                         const bool fi_elem_side) const
{
  const auto face_type = fi.faceType(std::make_pair(_var_num, _sys_num));
  if (fi_elem_side)
    return face_type == FaceInfo::VarFaceNeighbors::ELEM ||
           face_type == FaceInfo::VarFaceNeighbors::BOTH;

  return face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR ||
         face_type == FaceInfo::VarFaceNeighbors::BOTH;
}

Real
ConservativeSharpInterfaceVOFMULESCorrector::donorFlux(const FaceInfo & fi,
                                                       const FaceTransportData & face_data,
                                                       const Real elem_alpha) const
{
  Real donor_alpha = elem_alpha;
  if (face_data.boundary_kind == BoundaryFaceKind::Internal)
    donor_alpha = face_data.upwind_is_elem ? elem_alpha : cellAlpha(*fi.neighborInfo());
  else if (auto * inlet_outlet_bc =
               dynamic_cast<LinearFVInletOutletScalarBC *>(face_data.boundary_condition))
  {
    donor_alpha = face_data.upwind_is_elem
                      ? elem_alpha
                      : boundedAlpha(inlet_outlet_bc->computeBoundaryValue(/* backflow = */ true));
  }
  else if (_alpha_var->isDirichletBoundaryFace(fi))
    donor_alpha = face_data.upwind_is_elem ? elem_alpha : boundaryValue(fi, face_data);

  return face_data.integrated_flux * donor_alpha;
}

Real
ConservativeSharpInterfaceVOFMULESCorrector::highOrderFaceValue(const FaceInfo & fi,
                                                                const FaceTransportData & face_data,
                                                                const Real elem_alpha) const
{
  Real high_order_alpha = 0.0;
  if (face_data.boundary_kind == BoundaryFaceKind::Internal)
    high_order_alpha = sharedVanLeerFaceValue(fi, face_data.upwind_is_elem);
  else if (dynamic_cast<LinearFVAdvectionDiffusionFunctorDirichletBC *>(
               face_data.boundary_condition))
    high_order_alpha = boundaryValue(fi, face_data);
  else if (auto * inlet_outlet_bc =
               dynamic_cast<LinearFVInletOutletScalarBC *>(face_data.boundary_condition))
  {
    high_order_alpha = boundedAlpha(
        inlet_outlet_bc->computeBoundaryValue(/* backflow = */ !face_data.upwind_is_elem));
  }
  else if (face_data.upwind_is_elem &&
           dynamic_cast<LinearFVAdvectionDiffusionExtrapolatedBC *>(face_data.boundary_condition))
    high_order_alpha = boundaryValue(fi, face_data);
  else
    return std::abs(face_data.volumetric_flux) > libMesh::TOLERANCE
               ? donorFlux(fi, face_data, elem_alpha) / face_data.integrated_flux
               : elem_alpha;

  return high_order_alpha;
}

Real
ConservativeSharpInterfaceVOFMULESCorrector::sharedVanLeerFaceValue(const FaceInfo & fi,
                                                                    const bool upwind_is_elem) const
{
  mooseAssert(fi.neighborPtr(), "Van Leer correction requires an internal face with a neighbor.");

  const auto state = Moose::currentState();
  const auto face_arg = makeFace(fi, Moose::FV::LimiterType::VanLeer, upwind_is_elem);
  return boundedAlpha(MetaPhysicL::raw_value((*_alpha_var)(face_arg, state)));
}

ConservativeSharpInterfaceVOFMULESCorrector::BoundaryFaceKind
ConservativeSharpInterfaceVOFMULESCorrector::classifyBoundaryFace(
    const FaceInfo & fi,
    const FaceInfo::VarFaceNeighbors face_type,
    const Real volumetric_flux,
    const LinearFVBoundaryCondition * const bc) const
{
  if (fi.neighborPtr() && face_type == FaceInfo::VarFaceNeighbors::BOTH)
    return BoundaryFaceKind::Internal;

  if (std::abs(volumetric_flux) <= libMesh::TOLERANCE)
    return BoundaryFaceKind::Closed;

  if (!bc)
    return BoundaryFaceKind::Closed;

  if (dynamic_cast<const LinearFVAdvectionDiffusionFunctorDirichletBC *>(bc))
    return volumetric_flux >= 0.0 ? BoundaryFaceKind::DirichletOutflow
                                  : BoundaryFaceKind::DirichletInflow;

  if (dynamic_cast<const LinearFVInletOutletScalarBC *>(bc))
    return volumetric_flux >= 0.0 ? BoundaryFaceKind::OpenOutflow
                                  : BoundaryFaceKind::DirichletInflow;

  if (dynamic_cast<const LinearFVAdvectionDiffusionExtrapolatedBC *>(bc))
    return volumetric_flux >= 0.0 ? BoundaryFaceKind::OpenOutflow : BoundaryFaceKind::Closed;

  return BoundaryFaceKind::Closed;
}

Real
ConservativeSharpInterfaceVOFMULESCorrector::rhoPhi(const FaceInfo & fi,
                                                    const Real limited_alpha_flux) const
{
  const auto state = Moose::currentState();
  const Real gas_density =
      MetaPhysicL::raw_value(_gas_density(functorFaceArg(_gas_density, fi), state));
  const Real liquid_density =
      MetaPhysicL::raw_value(_liquid_density(functorFaceArg(_liquid_density, fi), state));
  const Real volumetric_mass_flux =
      _face_flux(functorFaceArg(_face_flux, fi), Moose::currentState()) * faceMeasure(fi) *
      gas_density;
  return volumetric_mass_flux + (liquid_density - gas_density) * limited_alpha_flux;
}

ConservativeSharpInterfaceVOFMULESCorrector::FaceCorrectionData
ConservativeSharpInterfaceVOFMULESCorrector::buildFaceCorrectionData(const FaceInfo & fi) const
{
  FaceCorrectionData data;
  const auto face_data = faceTransportData(fi);
  if (face_data.face_type == FaceInfo::VarFaceNeighbors::NEITHER)
    return data;

  data.face = &fi;
  data.elem_dof = fi.elemInfo()->dofIndices()[_sys_num][_var_num];
  data.has_neighbor = face_data.boundary_kind == BoundaryFaceKind::Internal &&
                      fi.neighborInfo()->dofIndices()[_sys_num][_var_num] != DofObject::invalid_id;
  if (data.has_neighbor)
    data.neighbor_dof = fi.neighborInfo()->dofIndices()[_sys_num][_var_num];

  data.elem_alpha = cellAlpha(*fi.elemInfo());

  data.boundary_kind = face_data.boundary_kind;
  data.neighbor_alpha = data.boundary_kind == BoundaryFaceKind::Internal
                            ? cellAlpha(*fi.neighborInfo())
                            : boundaryValue(fi, face_data);

  const Real integrated_phi = face_data.integrated_flux;
  data.donor_flux = donorFlux(fi, face_data, data.elem_alpha);
  const Real high_order_alpha = highOrderFaceValue(fi, face_data, data.elem_alpha);
  const Real high_order_flux = integrated_phi * high_order_alpha;
  Real total_flux = data.donor_flux;

  if (face_data.boundary_kind == BoundaryFaceKind::Closed)
    return data;

  if (face_data.boundary_kind != BoundaryFaceKind::Internal)
  {
    if (face_data.boundary_kind == BoundaryFaceKind::DirichletOutflow)
      total_flux = high_order_flux;

    data.correction_flux = total_flux - data.donor_flux;
    return data;
  }

  const auto state = Moose::currentState();
  const auto face_arg = makeCDFace(fi);
  const RealVectorValue face_normal = fi.normal();
  const Real face_normal_mag = face_normal.norm();
  const RealVectorValue face_unit_normal =
      face_normal_mag > 0.0 ? face_normal / face_normal_mag : RealVectorValue();
  const RealVectorValue interface_normal =
      MetaPhysicL::raw_value(_interface_normal(face_arg, state));
  const Real interface_normal_alignment = interface_normal * face_unit_normal;

  if (std::abs(integrated_phi) > libMesh::TOLERANCE)
  {
    const Real bounded_elem_alpha = boundedAlpha(data.elem_alpha);
    const Real bounded_neighbor_alpha = boundedAlpha(data.neighbor_alpha);
    const Real linear_alpha =
        boundedAlpha(fi.gC() * bounded_elem_alpha + (1.0 - fi.gC()) * bounded_neighbor_alpha);
    const Real compression_factor = MetaPhysicL::raw_value(_compression_factor(face_arg, state));
    const Real compressed_alpha = boundedAlpha(
        high_order_alpha + compression_factor * MathUtils::sign(integrated_phi) * linear_alpha *
                               (1.0 - linear_alpha) * interface_normal_alignment);

    total_flux = integrated_phi * compressed_alpha;
  }
  else
    total_flux = high_order_flux;

  data.correction_flux = total_flux - data.donor_flux;

  return data;
}

std::vector<ConservativeSharpInterfaceVOFMULESCorrector::FaceCorrectionData>
ConservativeSharpInterfaceVOFMULESCorrector::collectFaceCorrectionData() const
{
  std::vector<FaceCorrectionData> face_corrections;
  face_corrections.reserve(_fe_problem.mesh().faceInfo().size());

  for (const auto * fi : _fe_problem.mesh().faceInfo())
  {
    auto data = buildFaceCorrectionData(*fi);
    if (data.face)
      face_corrections.push_back(data);
  }

  return face_corrections;
}

Real
ConservativeSharpInterfaceVOFMULESCorrector::confinedScalarConcentration(
    const ConfinedScalarData & scalar, const ElemInfo & elem_info) const
{
  const auto dof = elem_info.dofIndices()[scalar.sys_num][scalar.var_num];
  if (dof == DofObject::invalid_id)
    return 0.0;

  const Real alpha = boundedAlpha(cellAlpha(elem_info));
  if (alpha <= _confined_scalar_alpha_floor)
    return 0.0;

  return std::min(_confined_scalar_concentration_max,
                  std::max(_confined_scalar_concentration_min,
                           (*scalar.system->system().current_local_solution)(dof) / alpha));
}

void
ConservativeSharpInterfaceVOFMULESCorrector::applyConfinedScalarTransport(
    const std::vector<FaceCorrectionData> & face_corrections,
    const std::vector<Real> & limited_alpha_fluxes,
    const Real dt)
{
  if (_confined_scalars.empty() || dt <= 0.0)
    return;

  mooseAssert(face_corrections.size() == limited_alpha_fluxes.size(),
              "Confined scalar transport needs one alpha flux per face correction.");

  const auto state = Moose::currentState();
  for (const auto & scalar : _confined_scalars)
  {
    struct ScalarFaceFlux
    {
      Real flux = 0.0;
      dof_id_type from_dof = DofObject::invalid_id;
      dof_id_type to_dof = DofObject::invalid_id;
    };

    auto & current_local_solution = *scalar.system->system().current_local_solution;
    std::unordered_map<dof_id_type, Real> applied_change;
    std::unordered_map<dof_id_type, Real> old_value_by_dof;
    std::unordered_map<dof_id_type, Real> lower_bound_by_dof;
    std::unordered_map<dof_id_type, Real> upper_bound_by_dof;
    std::unordered_map<dof_id_type, Real> cell_volume_by_dof;
    std::unordered_map<dof_id_type, Real> out_flux_by_dof;
    std::unordered_map<dof_id_type, Real> in_flux_by_dof;
    std::vector<ScalarFaceFlux> scalar_fluxes(face_corrections.size());

    const auto cache_cell = [&](const ElemInfo & elem_info)
    {
      const auto dof = elem_info.dofIndices()[scalar.sys_num][scalar.var_num];
      if (dof == DofObject::invalid_id || old_value_by_dof.count(dof))
        return;

      const Real alpha = boundedAlpha(cellAlpha(elem_info));
      old_value_by_dof.emplace(dof, current_local_solution(dof));
      lower_bound_by_dof.emplace(dof, alpha * _confined_scalar_concentration_min);
      upper_bound_by_dof.emplace(dof, alpha * _confined_scalar_concentration_max);
      cell_volume_by_dof.emplace(dof, cellVolume(elem_info));
      out_flux_by_dof.emplace(dof, 0.0);
      in_flux_by_dof.emplace(dof, 0.0);
    };

    for (const auto i : index_range(face_corrections))
    {
      const auto & data = face_corrections[i];
      const Real alpha_flux = limited_alpha_fluxes[i];
      if (std::abs(alpha_flux) <= libMesh::TOLERANCE)
        continue;

      const auto elem_dof = data.face->elemInfo()->dofIndices()[scalar.sys_num][scalar.var_num];
      if (elem_dof == DofObject::invalid_id)
        continue;
      const auto neighbor_dof =
          data.has_neighbor
              ? data.face->neighborInfo()->dofIndices()[scalar.sys_num][scalar.var_num]
              : DofObject::invalid_id;
      cache_cell(*data.face->elemInfo());
      if (data.has_neighbor)
        cache_cell(*data.face->neighborInfo());

      Real concentration = 0.0;
      if (alpha_flux > 0.0)
        concentration = confinedScalarConcentration(scalar, *data.face->elemInfo());
      else if (data.has_neighbor)
        concentration = confinedScalarConcentration(scalar, *data.face->neighborInfo());
      else
        concentration = std::min(
            _confined_scalar_concentration_max,
            std::max(
                _confined_scalar_concentration_min,
                _confined_scalar_backflow_concentration(
                    functorFaceArg(_confined_scalar_backflow_concentration, *data.face), state)));

      const Real scalar_flux = alpha_flux * concentration;
      if (std::abs(scalar_flux) <= libMesh::TOLERANCE)
        continue;

      auto & flux_data = scalar_fluxes[i];
      flux_data.flux = scalar_flux;
      if (scalar_flux > 0.0)
      {
        flux_data.from_dof = elem_dof;
        flux_data.to_dof = neighbor_dof;
      }
      else
      {
        flux_data.from_dof = neighbor_dof;
        flux_data.to_dof = elem_dof;
      }

      if (flux_data.from_dof != DofObject::invalid_id)
        out_flux_by_dof[flux_data.from_dof] += std::abs(scalar_flux);
      if (flux_data.to_dof != DofObject::invalid_id)
        in_flux_by_dof[flux_data.to_dof] += std::abs(scalar_flux);
    }

    std::unordered_map<dof_id_type, Real> out_limiter_by_dof;
    std::unordered_map<dof_id_type, Real> in_limiter_by_dof;
    for (const auto & pair : old_value_by_dof)
    {
      const auto dof = pair.first;
      const Real cell_volume = libmesh_map_find(cell_volume_by_dof, dof);
      const Real old_value = pair.second;
      const Real lower_bound = libmesh_map_find(lower_bound_by_dof, dof);
      const Real upper_bound = libmesh_map_find(upper_bound_by_dof, dof);
      const Real out_flux = libmesh_map_find(out_flux_by_dof, dof);
      const Real in_flux = libmesh_map_find(in_flux_by_dof, dof);
      const Real loss_capacity = cell_volume * std::max(0.0, old_value - lower_bound) / dt;
      const Real gain_capacity = cell_volume * std::max(0.0, upper_bound - old_value) / dt;
      out_limiter_by_dof.emplace(
          dof, out_flux > libMesh::TOLERANCE ? boundedAlpha(loss_capacity / out_flux) : 1.0);
      in_limiter_by_dof.emplace(
          dof, in_flux > libMesh::TOLERANCE ? boundedAlpha(gain_capacity / in_flux) : 1.0);
    }

    for (const auto i : index_range(face_corrections))
    {
      const auto raw_scalar_flux = scalar_fluxes[i].flux;
      if (std::abs(raw_scalar_flux) <= libMesh::TOLERANCE)
        continue;

      Real limiter = 1.0;
      const auto from_dof = scalar_fluxes[i].from_dof;
      const auto to_dof = scalar_fluxes[i].to_dof;
      if (from_dof != DofObject::invalid_id)
        limiter = std::min(limiter, libmesh_map_find(out_limiter_by_dof, from_dof));
      if (to_dof != DofObject::invalid_id)
        limiter = std::min(limiter, libmesh_map_find(in_limiter_by_dof, to_dof));

      const Real scalar_flux = limiter * raw_scalar_flux;
      if (std::abs(scalar_flux) <= libMesh::TOLERANCE)
        continue;

      const auto & data = face_corrections[i];
      const auto elem_dof = data.face->elemInfo()->dofIndices()[scalar.sys_num][scalar.var_num];
      if (locallyOwnedCell(*data.face->elemInfo()))
        applied_change[elem_dof] -= dt * scalar_flux / cellVolume(*data.face->elemInfo());

      if (data.has_neighbor)
      {
        const auto neighbor_dof =
            data.face->neighborInfo()->dofIndices()[scalar.sys_num][scalar.var_num];
        if (neighbor_dof != DofObject::invalid_id && locallyOwnedCell(*data.face->neighborInfo()))
          applied_change[neighbor_dof] += dt * scalar_flux / cellVolume(*data.face->neighborInfo());
      }
    }

    if (applied_change.empty())
      continue;

    auto scalar_update = current_local_solution.zero_clone();
    for (const auto & pair : applied_change)
      scalar_update->add(pair.first, pair.second);
    scalar_update->close();

    current_local_solution.add(*scalar_update);
    current_local_solution.close();
    scalar.system->solution() = current_local_solution;
    scalar.system->solution().close();
    scalar.system->setSolution(current_local_solution);
    scalar.system->computeGradients();
  }
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
      push_data[other_pid].emplace_back(data.face->id(), accepted_lambda[i]);
    }

  std::unordered_map<dof_id_type, Real> received_minimum_lambda_by_face;
  auto receive_limiters = [&received_minimum_lambda_by_face](const processor_id_type,
                                                             const std::vector<Datum> & sent_data)
  {
    for (const auto & [face_id, lambda] : sent_data)
    {
      auto & received_lambda =
          received_minimum_lambda_by_face.try_emplace(face_id, lambda).first->second;
      received_lambda = std::min(received_lambda, lambda);
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
    applyConfinedScalarTransport(face_corrections, donor_flux, dt);
    _system->computeGradients();
    return;
  }

  for (const auto correction_it : make_range(_num_alpha_corrections))
  {
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

    for (const auto i : index_range(face_corrections))
    {
      const auto & data = face_corrections[i];
      const auto face_id = data.face->id();
      working_alpha_flux.try_emplace(face_id, data.donor_flux);

      const Real target_alpha_flux = data.donor_flux + data.correction_flux;
      raw_correction_flux[i] = target_alpha_flux - libmesh_map_find(working_alpha_flux, face_id);
      accepted_lambda[i] = std::abs(raw_correction_flux[i]) > libMesh::TOLERANCE ? 1.0 : 0.0;
    }

    std::unordered_map<dof_id_type, std::pair<Real, Real>> local_bounds;

    const auto local_bounds_for = [&](const dof_id_type dof) -> std::pair<Real, Real> &
    { return local_bounds.try_emplace(dof, alpha_min, alpha_max).first->second; };

    const auto widen_local_bounds = [&](const dof_id_type dof, const Real alpha)
    {
      const Real bounded_alpha = boundedAlpha(alpha);
      auto & bounds = local_bounds_for(dof);
      bounds.first = std::max(bounds.first, bounded_alpha);
      bounds.second = std::min(bounds.second, bounded_alpha);
    };

    for (const auto & data : face_corrections)
    {
      local_bounds_for(data.elem_dof);
      if (data.has_neighbor)
        local_bounds_for(data.neighbor_dof);

      if (data.boundary_kind == BoundaryFaceKind::Internal)
      {
        widen_local_bounds(data.elem_dof, data.neighbor_alpha);
        widen_local_bounds(data.neighbor_dof, data.elem_alpha);
      }
      else if (data.boundary_kind == BoundaryFaceKind::DirichletInflow ||
               data.boundary_kind == BoundaryFaceKind::DirichletOutflow)
        widen_local_bounds(data.elem_dof, data.neighbor_alpha);
    }

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
      const auto & bounds = libmesh_map_find(local_bounds, dof);
      psi_maxn.emplace(dof, cell_volume * std::max(0.0, bounds.first - alpha) / dt);
      psi_minn.emplace(dof, cell_volume * std::max(0.0, alpha - bounds.second) / dt);
      sum_phip.emplace(dof, 0.0);
      m_sum_phim.emplace(dof, 0.0);
    };

    for (const auto & data : face_corrections)
    {
      initialize_cmules_cell(data.elem_dof);
      if (data.has_neighbor)
        initialize_cmules_cell(data.neighbor_dof);
    }

    const auto accumulate_correction_fluxes =
        [&](const auto & correction_flux, auto & positive_outflow, auto & negative_inflow)
    {
      for (const auto i : index_range(face_corrections))
      {
        const auto & data = face_corrections[i];
        const Real phi_corr_f = correction_flux(i);
        if (std::abs(phi_corr_f) < libMesh::TOLERANCE)
          continue;

        if (phi_corr_f > 0.0)
        {
          positive_outflow[data.elem_dof] += phi_corr_f;
          if (data.has_neighbor)
            negative_inflow[data.neighbor_dof] += phi_corr_f;
        }
        else
        {
          negative_inflow[data.elem_dof] -= phi_corr_f;
          if (data.has_neighbor)
            positive_outflow[data.neighbor_dof] -= phi_corr_f;
        }
      }
    };

    accumulate_correction_fluxes(
        [&](const auto i) { return raw_correction_flux[i]; }, sum_phip, m_sum_phim);

    const Real root_v_small = std::sqrt(std::numeric_limits<Real>::min());

    for (const auto limiter_it : make_range(_num_limiter_iterations))
    {
      (void)limiter_it;
      std::unordered_map<dof_id_type, Real> sum_l_phip;
      std::unordered_map<dof_id_type, Real> m_sum_l_phim;

      for (const auto & pair : psi_maxn)
      {
        sum_l_phip.emplace(pair.first, 0.0);
        m_sum_l_phim.emplace(pair.first, 0.0);
      }

      accumulate_correction_fluxes([&](const auto i)
                                   { return accepted_lambda[i] * raw_correction_flux[i]; },
                                   sum_l_phip,
                                   m_sum_l_phim);

      std::unordered_map<dof_id_type, Real> lambda_minus;
      std::unordered_map<dof_id_type, Real> lambda_plus;
      for (const auto & pair : psi_maxn)
      {
        const auto dof = pair.first;
        lambda_minus[dof] =
            boundedAlpha((sum_l_phip[dof] + psi_maxn[dof]) / (m_sum_phim[dof] + root_v_small));
        lambda_plus[dof] =
            boundedAlpha((m_sum_l_phim[dof] + psi_minn[dof]) / (sum_phip[dof] + root_v_small));
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
          const Real corrected_boundary_flux = data.donor_flux + data.correction_flux + phi_corr_f;
          if (corrected_boundary_flux > libMesh::TOLERANCE * libMesh::TOLERANCE)
          {
            if (phi_corr_f > 0.0)
              lambda = std::min(lambda, lambda_plus[data.elem_dof]);
            else
              lambda = std::min(lambda, lambda_minus[data.elem_dof]);
          }
        }

        const Real updated_lambda = std::min(accepted_lambda[i], boundedAlpha(lambda));
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
        const Real correction_weight = correction_it == 0 ? 1.0 : later_correction_relaxation;
        limited_correction_flux[i] =
            correction_weight * accepted_lambda[i] * raw_correction_flux[i];
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
    {
      publishFaceFluxes(face_corrections, accumulated_alpha_flux, subcycle_fraction);
      applyConfinedScalarTransport(face_corrections, accumulated_alpha_flux, dt);
    }
  }

  _system->computeGradients();
}
