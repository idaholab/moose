//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConservativeDiffuseInterfaceVOFMULESCorrector.h"

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
#include "NSFVUtils.h"

#include "timpi/parallel_sync.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <tuple>
#include <unordered_map>
#include <vector>

registerMooseObject("NavierStokesApp", ConservativeDiffuseInterfaceVOFMULESCorrector);

namespace
{
constexpr Real alpha_min = 0.0;
constexpr Real alpha_max = 1.0;
/// Correction sweeps after the first apply half of their correction. A later sweep forms its
/// target from the field the previous sweep already corrected, so applying its full difference
/// repeats part of the antidiffusion already applied; halving it keeps successive sweeps from
/// alternating about the limited solution.
constexpr Real later_correction_relaxation = 0.5;
/// A face flux or flux correction below this fraction of the largest face flux of the step
/// counts as zero. The floor has to scale with the problem: an absolute cutoff switched the
/// high-order, compression and limiter corrections off entirely in a millimeter-scale problem,
/// whose face fluxes are all far below any fixed number.
constexpr Real flux_floor_relative = 1e-10;
}

InputParameters
ConservativeDiffuseInterfaceVOFMULESCorrector::validParams()
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
  params.addParam<bool>(
      "outflow_two_term_expansion",
      false,
      "Whether the volume fraction leaving through an open outflow boundary is extrapolated from "
      "the cell with its gradient, as a limited correction to the upwind flux. The outflow "
      "boundary conditions must then use the cell value (use_two_term_expansion = false).");
  ExecFlagEnum & exec_enum = params.set<ExecFlagEnum>("execute_on", true);
  exec_enum.addAvailableFlags(EXEC_NONE);
  exec_enum = {EXEC_NONE};
  params.suppressParameter<ExecFlagEnum>("execute_on");
  return params;
}

ConservativeDiffuseInterfaceVOFMULESCorrector::ConservativeDiffuseInterfaceVOFMULESCorrector(
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
    _outflow_two_term_expansion(getParam<bool>("outflow_two_term_expansion")),
    _alpha_phi_limited(_fe_problem.mesh(), blockIDs(), "alpha_phi_limited"),
    _rho_phi(_fe_problem.mesh(), blockIDs(), "rho_phi"),
    _accepted_alpha_phi_limited(declareRestartableData<std::unordered_map<dof_id_type, Real>>(
        "accepted_alpha_phi_limited")),
    _accepted_rho_phi(
        declareRestartableData<std::unordered_map<dof_id_type, Real>>("accepted_rho_phi")),
    _accepted_fluxes_valid(declareRestartableData<bool>("accepted_fluxes_valid", false))
{
  for (const auto tid : make_range(libMesh::n_threads()))
  {
    UserObject::_subproblem.addFunctor("alpha_phi_limited", _alpha_phi_limited, tid);
    UserObject::_subproblem.addFunctor("rho_phi", _rho_phi, tid);
  }

  // The volume fraction's cell gradient is state, not a function of the field: the outflow
  // boundary's two-term expansion reads it, and the Green-Gauss gradient's boundary faces read
  // that boundary value back, so each gradient depends on the one before. Keeping one old gradient
  // state lets the framework restore it when a step is solved again and carry it through a
  // checkpoint, the same way it does for the solution. Old states can only be requested during
  // setup, hence here rather than in initialSetup().
  if (auto * alpha_var =
          dynamic_cast<MooseLinearVariableFVReal *>(&_fe_problem.getVariable(_tid, _variable_name)))
    alpha_var->requestCellGradients(/*oldest_state=*/1);
}

void
ConservativeDiffuseInterfaceVOFMULESCorrector::initialSetup()
{
  cacheSystemData();
  initializeFluxStorage();
}

void
ConservativeDiffuseInterfaceVOFMULESCorrector::meshChanged()
{
  cacheSystemData();
  initializeFluxStorage();
}

void
ConservativeDiffuseInterfaceVOFMULESCorrector::cacheSystemData()
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
ConservativeDiffuseInterfaceVOFMULESCorrector::checkBoundaryConditions() const
{
  // A two-term outflow condition would put the extrapolation into the upwind solve, unbounded,
  // while the correction measures the outflow against the cell value
  for (const auto * fi : _fe_problem.mesh().faceInfo())
    if (const auto * bc = dynamic_cast<const LinearFVAdvectionDiffusionExtrapolatedBC *>(
            NS::linearFVBoundaryCondition(*_alpha_var, *fi));
        bc && bc->getParam<bool>("use_two_term_expansion"))
      mooseError("The volume-fraction boundary condition '",
                 bc->name(),
                 "' uses a two-term expansion. ",
                 type(),
                 " needs the outflow face value of the upwind solve to be the cell value; set "
                 "use_two_term_expansion = false on it and outflow_two_term_expansion = true here "
                 "to extrapolate the outflow as a limited correction.");
}

void
ConservativeDiffuseInterfaceVOFMULESCorrector::initializeFluxStorage()
{
  for (const auto * fi : _fe_problem.mesh().faceInfo())
  {
    _alpha_phi_limited[fi->id()] = 0.0;
    _rho_phi[fi->id()] = 0.0;
  }
}

void
ConservativeDiffuseInterfaceVOFMULESCorrector::commitAcceptedTimestepFluxes()
{
  _accepted_alpha_phi_limited.clear();
  _accepted_rho_phi.clear();
  _accepted_alpha_phi_limited.insert(_alpha_phi_limited.begin(), _alpha_phi_limited.end());
  _accepted_rho_phi.insert(_rho_phi.begin(), _rho_phi.end());
  _accepted_fluxes_valid = true;
}

void
ConservativeDiffuseInterfaceVOFMULESCorrector::timestepSetup()
{
  // The variable maps its boundary conditions in its own initialSetup, which may run after this
  // object's, so they are checked at the first step instead
  if (!_boundary_conditions_checked)
  {
    checkBoundaryConditions();
    _boundary_conditions_checked = true;
  }

  _outer_iteration_lambda_bound.clear();
  if (!_accepted_fluxes_valid)
    return;

  for (const auto & [face_id, value] : _accepted_alpha_phi_limited)
    _alpha_phi_limited[face_id] = value;
  for (const auto & [face_id, value] : _accepted_rho_phi)
    _rho_phi[face_id] = value;
}

void
ConservativeDiffuseInterfaceVOFMULESCorrector::resetSubcycleFluxes()
{
  _subcycle_index = 0;
  // rho_phi is rebuilt from the accumulated alpha flux by refreshPublishedRhoPhi()
  for (auto & pair : _alpha_phi_limited)
    pair.second = 0.0;
}

void
ConservativeDiffuseInterfaceVOFMULESCorrector::refreshPublishedRhoPhi()
{
  // The alpha flux accumulated over the subcycles is kept; only its volumetric part is re-read,
  // because the flux functor is evaluated at the problem time, which the subcycles moved and the
  // executioner has now restored. The momentum equation reads rho_phi at the global time.

  for (const auto * fi : _fe_problem.mesh().faceInfo())
  {
    if (transportFaceType(*fi) == FaceInfo::VarFaceNeighbors::NEITHER)
      continue;

    const auto face_id = fi->id();
    const auto state = Moose::currentState();
    const Real volumetric_flux = _face_flux(
        makeFace(_face_flux, *fi, Moose::FV::LimiterType::CentralDifference, true), state);
    const Real gas_density = MetaPhysicL::raw_value(_gas_density(
        makeFace(_gas_density, *fi, Moose::FV::LimiterType::CentralDifference, true), state));
    const Real liquid_density = MetaPhysicL::raw_value(_liquid_density(
        makeFace(_liquid_density, *fi, Moose::FV::LimiterType::CentralDifference, true), state));
    // The mass flux consistent with the limited alpha flux: rho_g phi + (rho_l - rho_g) alpha_phi
    _rho_phi[face_id] =
        volumetric_flux * NS::faceMeasure(*fi) * gas_density +
        (liquid_density - gas_density) * libmesh_map_find(_alpha_phi_limited, face_id);
  }
}

Real
ConservativeDiffuseInterfaceVOFMULESCorrector::boundaryValue(
    const FaceInfo & fi, const FaceTransportData & face_data) const
{
  if (auto * bc = face_data.boundary_condition)
    return std::clamp(bc->computeBoundaryValue(), alpha_min, alpha_max);

  return _alpha_var->getElemValue(face_data.face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR
                                      ? *fi.neighborInfo()
                                      : *fi.elemInfo(),
                                  Moose::currentState());
}

FaceInfo::VarFaceNeighbors
ConservativeDiffuseInterfaceVOFMULESCorrector::transportFaceType(const FaceInfo & fi) const
{
  const auto face_type = fi.faceType(std::make_pair(_var_num, _sys_num));
  if (face_type == FaceInfo::VarFaceNeighbors::NEITHER)
    return face_type;

  return hasBlocks(fi.elemSubdomainID()) ||
                 (fi.neighborPtr() && hasBlocks(fi.neighborSubdomainID()))
             ? face_type
             : FaceInfo::VarFaceNeighbors::NEITHER;
}

ConservativeDiffuseInterfaceVOFMULESCorrector::FaceTransportData
ConservativeDiffuseInterfaceVOFMULESCorrector::faceTransportData(const FaceInfo & fi) const
{
  FaceTransportData data;
  data.face_type = transportFaceType(fi);
  if (data.face_type == FaceInfo::VarFaceNeighbors::NEITHER)
    return data;

  data.volumetric_flux =
      _face_flux(makeFace(_face_flux, fi, Moose::FV::LimiterType::CentralDifference, true),
                 Moose::currentState());
  data.integrated_flux = data.volumetric_flux * NS::faceMeasure(fi);
  data.upwind_is_elem = data.volumetric_flux >= 0.0;
  data.boundary_condition = NS::linearFVBoundaryCondition(*_alpha_var, fi);
  data.boundary_kind =
      classifyBoundaryFace(fi, data.face_type, data.volumetric_flux, data.boundary_condition);
  return data;
}

Real
ConservativeDiffuseInterfaceVOFMULESCorrector::fluxScale() const
{
  Real scale = 0.0;
  for (const auto * fi : _fe_problem.mesh().faceInfo())
    if (transportFaceType(*fi) != FaceInfo::VarFaceNeighbors::NEITHER)
      scale = std::max(
          scale,
          std::abs(
              _face_flux(makeFace(_face_flux, *fi, Moose::FV::LimiterType::CentralDifference, true),
                         Moose::currentState()) *
              NS::faceMeasure(*fi)));
  // The floor must agree on every rank, as the limiter iteration does
  _communicator.max(scale);
  return scale;
}

bool
ConservativeDiffuseInterfaceVOFMULESCorrector::hasFaceSide(const FaceInfo & fi,
                                                           const bool fi_elem_side) const
{
  // The faces this object produces are the volume fraction's
  return _alpha_var->hasFaceSide(fi, fi_elem_side);
}

Real
ConservativeDiffuseInterfaceVOFMULESCorrector::donorFlux(const FaceInfo & fi,
                                                         const FaceTransportData & face_data,
                                                         const Real elem_alpha) const
{
  // The value the implicit upwind solve transported through this face: the cell the flux
  // leaves, or on an inflow boundary the value the boundary condition brings in. A wall or an
  // open outflow carries the cell value either way.
  Real donor_alpha = elem_alpha;
  if (face_data.boundary_kind == BoundaryFaceKind::Internal)
    donor_alpha = face_data.upwind_is_elem
                      ? elem_alpha
                      : _alpha_var->getElemValue(*fi.neighborInfo(), Moose::currentState());
  else if (auto * inlet_outlet_bc =
               dynamic_cast<LinearFVInletOutletScalarBC *>(face_data.boundary_condition))
  {
    donor_alpha = face_data.upwind_is_elem
                      ? elem_alpha
                      : std::clamp(inlet_outlet_bc->computeBoundaryValue(/* backflow = */ true),
                                   alpha_min,
                                   alpha_max);
  }
  else if (_alpha_var->isDirichletBoundaryFace(fi))
    donor_alpha = face_data.upwind_is_elem ? elem_alpha : boundaryValue(fi, face_data);

  return face_data.integrated_flux * donor_alpha;
}

ConservativeDiffuseInterfaceVOFMULESCorrector::BoundaryFaceKind
ConservativeDiffuseInterfaceVOFMULESCorrector::classifyBoundaryFace(
    const FaceInfo & fi,
    const FaceInfo::VarFaceNeighbors face_type,
    const Real volumetric_flux,
    const LinearFVBoundaryCondition * const bc) const
{
  if (fi.neighborPtr() && face_type == FaceInfo::VarFaceNeighbors::BOTH)
    return BoundaryFaceKind::Internal;

  if (std::abs(volumetric_flux * NS::faceMeasure(fi)) <= _flux_floor)
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

ConservativeDiffuseInterfaceVOFMULESCorrector::FaceCorrectionData
ConservativeDiffuseInterfaceVOFMULESCorrector::buildFaceCorrectionData(const FaceInfo & fi) const
{
  FaceCorrectionData data;
  const auto face_data = faceTransportData(fi);
  if (face_data.face_type == FaceInfo::VarFaceNeighbors::NEITHER)
    return data;

  data.face = &fi;
  data.elem_dof = fi.elemInfo()->dofIndices()[_sys_num][_var_num];
  // An internal face has the variable on both sides, so the neighbor has a dof of it
  data.has_neighbor = face_data.boundary_kind == BoundaryFaceKind::Internal;
  if (data.has_neighbor)
    data.neighbor_dof = fi.neighborInfo()->dofIndices()[_sys_num][_var_num];
  data.elem_owned = locallyOwnedCell(*fi.elemInfo());
  data.neighbor_owned = data.has_neighbor && locallyOwnedCell(*fi.neighborInfo());

  data.elem_alpha = _alpha_var->getElemValue(*fi.elemInfo(), Moose::currentState());

  data.boundary_kind = face_data.boundary_kind;
  data.neighbor_alpha = data.boundary_kind == BoundaryFaceKind::Internal
                            ? _alpha_var->getElemValue(*fi.neighborInfo(), Moose::currentState())
                            : boundaryValue(fi, face_data);

  const Real integrated_phi = face_data.integrated_flux;
  data.donor_flux = donorFlux(fi, face_data, data.elem_alpha);
  Real total_flux = data.donor_flux;

  if (face_data.boundary_kind == BoundaryFaceKind::Closed)
    return data;

  // On a boundary face the upwind solve carried the cell value out of the domain and the
  // prescribed value in. An outflow is corrected to the prescribed value on a fixed-value face, and
  // with outflow_two_term_expansion to alpha_P + grad(alpha).d on an open one; the limiter bounds
  // either. An inflow already carries the prescribed value, so its correction is zero.
  if (face_data.boundary_kind != BoundaryFaceKind::Internal)
  {
    if (face_data.boundary_kind == BoundaryFaceKind::DirichletOutflow)
      total_flux = integrated_phi * data.neighbor_alpha;
    else if (face_data.boundary_kind == BoundaryFaceKind::OpenOutflow &&
             _outflow_two_term_expansion)
    {
      const Real extrapolated_alpha =
          std::clamp(data.elem_alpha + _alpha_var->gradSln(*fi.elemInfo(), Moose::currentState()) *
                                           (fi.faceCentroid() - fi.elemCentroid()),
                     alpha_min,
                     alpha_max);
      total_flux = integrated_phi * extrapolated_alpha;
    }

    data.correction_flux = total_flux - data.donor_flux;
    return data;
  }

  const auto state = Moose::currentState();
  // The target face value before compression: the Van Leer limited face value
  const Real high_order_alpha = std::clamp(
      MetaPhysicL::raw_value((*_alpha_var)(
          makeFace(fi, Moose::FV::LimiterType::VanLeer, face_data.upwind_is_elem), state)),
      alpha_min,
      alpha_max);
  const auto face_arg = makeCDFace(fi);
  const RealVectorValue face_normal = fi.normal();
  const Real face_normal_mag = face_normal.norm();
  const RealVectorValue face_unit_normal =
      face_normal_mag > 0.0 ? face_normal / face_normal_mag : RealVectorValue();
  const RealVectorValue interface_normal =
      MetaPhysicL::raw_value(_interface_normal(face_arg, state));
  const Real interface_normal_alignment = interface_normal * face_unit_normal;

  if (std::abs(integrated_phi) > _flux_floor)
  {
    // The interface compression, written as a face value rather than a separate flux:
    // the compression flux c_alpha |phi| alpha_f (1 - alpha_f) (n_interface . n_face) divided by
    // phi is a shift of the face value by c_alpha sign(phi) alpha_f (1 - alpha_f) (n_i . n_f). The
    // factor alpha_f (1 - alpha_f) confines it to the interface, where the linearly interpolated
    // face value is between 0 and 1, and the alignment makes it act across the interface, not
    // along it. It moves alpha toward the side it already leans to, against the diffusion of the
    // donor flux, and the limiter below keeps the result within the cell bounds.
    const Real bounded_elem_alpha = std::clamp(data.elem_alpha, alpha_min, alpha_max);
    const Real bounded_neighbor_alpha = std::clamp(data.neighbor_alpha, alpha_min, alpha_max);
    const Real linear_alpha =
        std::clamp(fi.gC() * bounded_elem_alpha + (1.0 - fi.gC()) * bounded_neighbor_alpha,
                   alpha_min,
                   alpha_max);
    const Real compression_factor = MetaPhysicL::raw_value(_compression_factor(face_arg, state));
    const Real compressed_alpha = std::clamp(
        high_order_alpha + compression_factor * MathUtils::sign(integrated_phi) * linear_alpha *
                               (1.0 - linear_alpha) * interface_normal_alignment,
        alpha_min,
        alpha_max);

    total_flux = integrated_phi * compressed_alpha;
  }
  else
    total_flux = integrated_phi * high_order_alpha;

  data.correction_flux = total_flux - data.donor_flux;

  return data;
}

std::vector<ConservativeDiffuseInterfaceVOFMULESCorrector::FaceCorrectionData>
ConservativeDiffuseInterfaceVOFMULESCorrector::collectFaceCorrectionData() const
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

void
ConservativeDiffuseInterfaceVOFMULESCorrector::publishFaceFluxes(
    const std::vector<FaceCorrectionData> & face_corrections,
    const std::vector<Real> & accumulated_alpha_fluxes,
    const Real subcycle_fraction)
{
  for (const auto i : index_range(face_corrections))
  {
    const auto & data = face_corrections[i];
    const auto face_id = data.face->id();
    // Accumulate the published alpha flux with the subcycle weighting, so that after subcycling it
    // is the step average refreshPublishedRhoPhi() builds rho_phi from
    _alpha_phi_limited[face_id] += subcycle_fraction * accumulated_alpha_fluxes[i];
  }
}

bool
ConservativeDiffuseInterfaceVOFMULESCorrector::locallyOwnedCell(const ElemInfo & elem_info) const
{
  return elem_info.elem()->processor_id() == processor_id();
}

void
ConservativeDiffuseInterfaceVOFMULESCorrector::exchangePartitionCellLimiters(
    const std::vector<FaceCorrectionData> & face_corrections,
    std::unordered_map<dof_id_type, Real> & lambda_plus,
    std::unordered_map<dof_id_type, Real> & lambda_minus) const
{
  if (n_processors() == 1)
    return;

  using Datum = std::tuple<dof_id_type, Real, Real>;
  std::unordered_map<processor_id_type, std::vector<Datum>> push_data;

  // A partition face has its two cells on different processes
  for (const auto & data : face_corrections)
    if (data.has_neighbor && data.face->elemInfo()->elem()->processor_id() !=
                                 data.face->neighborInfo()->elem()->processor_id())
    {
      const auto owned_dof = data.elem_owned ? data.elem_dof : data.neighbor_dof;
      const auto & ghost_info =
          data.elem_owned ? *data.face->neighborInfo() : *data.face->elemInfo();
      push_data[ghost_info.elem()->processor_id()].emplace_back(
          owned_dof,
          libmesh_map_find(lambda_plus, owned_dof),
          libmesh_map_find(lambda_minus, owned_dof));
    }

  auto receive_limiters = [&](const processor_id_type, const std::vector<Datum> & sent_data)
  {
    for (const auto & [dof, plus, minus] : sent_data)
    {
      lambda_plus[dof] = plus;
      lambda_minus[dof] = minus;
    }
  };

  TIMPI::push_parallel_vector_data(_communicator, push_data, receive_limiters);
}

void
ConservativeDiffuseInterfaceVOFMULESCorrector::applyCorrection(const Real dt,
                                                               const Real subcycle_fraction)
{
  if (dt <= 0.0)
    return;

  _flux_floor = flux_floor_relative * fluxScale();

  // The flux that produced the current alpha on each face: the donor flux after the implicit
  // solve, plus the limited corrections of the sweeps already applied. The corrections below are
  // measured against it, and it is what gets published at the end.
  // The face set and its order are the same in every sweep, so the flux is indexed like
  // face_corrections and filled from the donor flux by the first sweep.
  std::vector<Real> working_alpha_flux;

  // Without a correction sweep the donor flux is the published flux.
  if (_num_alpha_corrections == 0)
  {
    _system->computeGradients();
    const auto face_corrections = collectFaceCorrectionData();
    std::vector<Real> donor_flux(face_corrections.size());
    for (const auto i : index_range(face_corrections))
      donor_flux[i] = face_corrections[i].donor_flux;
    publishFaceFluxes(face_corrections, donor_flux, subcycle_fraction);
  }

  if (_outer_iteration_lambda_bound.size() <= _subcycle_index)
    _outer_iteration_lambda_bound.resize(_subcycle_index + 1);
  auto & lambda_bound = _outer_iteration_lambda_bound[_subcycle_index++];
  for (const auto correction_it : make_range(_num_alpha_corrections))
  {
    _system->computeGradients();

    // A process without transport faces still runs the sweep: the limiter exchange, the
    // reduction and the vector closes below are collective
    const auto face_corrections = collectFaceCorrectionData();

    auto & current_local_solution = *(_system->system().current_local_solution);
    std::unordered_map<dof_id_type, Real> applied_change;
    std::vector<Real> raw_correction_flux(face_corrections.size(), 0.0);
    std::vector<Real> limited_correction_flux(face_corrections.size(), 0.0);
    std::vector<Real> accepted_lambda(face_corrections.size(), 0.0);
    if (correction_it == 0)
    {
      working_alpha_flux.resize(face_corrections.size());
      for (const auto i : index_range(face_corrections))
        working_alpha_flux[i] = face_corrections[i].donor_flux;
    }

    for (const auto i : index_range(face_corrections))
    {
      const auto & data = face_corrections[i];
      const auto face_id = data.face->id();

      // The correction still to apply on this face, positive when it carries alpha from the
      // element to the neighbour. Its limiter starts at one and the passes below only lower it.
      const Real target_alpha_flux = data.donor_flux + data.correction_flux;
      raw_correction_flux[i] = target_alpha_flux - working_alpha_flux[i];
      accepted_lambda[i] = std::abs(raw_correction_flux[i]) > _flux_floor ? 1.0 : 0.0;
      if (const auto it = lambda_bound.find(face_id); it != lambda_bound.end())
        accepted_lambda[i] = std::min(accepted_lambda[i], it->second);
    }

    // The admissible range of each owned cell, stored as (local maximum, local minimum): the
    // extremes of the neighbouring cell values and of the prescribed boundary values, clamped to
    // [0, 1]. The corrected alpha may not leave this range, which is what keeps the antidiffusive
    // correction from creating new extrema.

    std::unordered_map<dof_id_type, std::pair<Real, Real>> local_bounds;

    const auto local_bounds_for = [&](const dof_id_type dof) -> std::pair<Real, Real> &
    { return local_bounds.try_emplace(dof, alpha_min, alpha_max).first->second; };

    const auto widen_local_bounds = [&](const dof_id_type dof, const Real alpha)
    {
      const Real bounded_alpha = std::clamp(alpha, alpha_min, alpha_max);
      auto & bounds = local_bounds_for(dof);
      bounds.first = std::max(bounds.first, bounded_alpha);
      bounds.second = std::min(bounds.second, bounded_alpha);
    };

    // The cell bounds, capacities and correction sums below are built for owned cells only. A
    // process sees every face of a cell it owns but only the partition faces of a ghost cell, so
    // ghost-cell sums would be partial; the ghost limiters come from their owners instead.
    for (const auto & data : face_corrections)
    {
      if (data.elem_owned)
        local_bounds_for(data.elem_dof);
      if (data.neighbor_owned)
        local_bounds_for(data.neighbor_dof);

      if (data.boundary_kind == BoundaryFaceKind::Internal)
      {
        if (data.elem_owned)
          widen_local_bounds(data.elem_dof, data.neighbor_alpha);
        if (data.neighbor_owned)
          widen_local_bounds(data.neighbor_dof, data.elem_alpha);
      }
      else if (data.elem_owned && (data.boundary_kind == BoundaryFaceKind::DirichletInflow ||
                                   data.boundary_kind == BoundaryFaceKind::DirichletOutflow))
        widen_local_bounds(data.elem_dof, data.neighbor_alpha);
    }

    std::unordered_map<dof_id_type, Real> cell_volume_by_dof;
    for (const auto & data : face_corrections)
    {
      if (data.elem_owned)
        cell_volume_by_dof.emplace(data.elem_dof, NS::cellMeasure(*data.face->elemInfo()));
      if (data.neighbor_owned)
        cell_volume_by_dof.emplace(data.neighbor_dof, NS::cellMeasure(*data.face->neighborInfo()));
    }

    // Zalesak's limiter quantities, per cell and per unit
    // time: psi_maxn and psi_minn are the net inflow and outflow of alpha the cell can still take
    // before reaching its local maximum and minimum, sum_phip is the sum of the raw corrections
    // leaving the cell and m_sum_phim the sum of those entering it.
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
      if (data.elem_owned)
        initialize_cmules_cell(data.elem_dof);
      if (data.neighbor_owned)
        initialize_cmules_cell(data.neighbor_dof);
    }

    const auto accumulate_correction_fluxes =
        [&](const auto & correction_flux, auto & positive_outflow, auto & negative_inflow)
    {
      for (const auto i : index_range(face_corrections))
      {
        const auto & data = face_corrections[i];
        const Real phi_corr_f = correction_flux(i);
        if (std::abs(phi_corr_f) <= _flux_floor)
          continue;

        if (phi_corr_f > 0.0)
        {
          if (data.elem_owned)
            positive_outflow[data.elem_dof] += phi_corr_f;
          if (data.neighbor_owned)
            negative_inflow[data.neighbor_dof] += phi_corr_f;
        }
        else
        {
          if (data.elem_owned)
            negative_inflow[data.elem_dof] -= phi_corr_f;
          if (data.neighbor_owned)
            positive_outflow[data.neighbor_dof] -= phi_corr_f;
        }
      }
    };

    accumulate_correction_fluxes(
        [&](const auto i) { return raw_correction_flux[i]; }, sum_phip, m_sum_phim);

    const Real root_v_small = std::sqrt(std::numeric_limits<Real>::min());

    // Each pass recomputes the cell limiters from the corrections the previous pass accepted,
    // then lowers the face limiters to them. The cell limiter lambda_minus is the fraction of the
    // raw inflow the cell can accept: what it may still rise by, plus the outflow already
    // accepted, over the raw inflow; lambda_plus is the same for the outflow. Crediting the
    // accepted counter-flow is what lets a cell pass more than its own capacity through, which
    // a single pass would refuse.
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
            std::clamp((sum_l_phip[dof] + psi_maxn[dof]) / (m_sum_phim[dof] + root_v_small),
                       alpha_min,
                       alpha_max);
        lambda_plus[dof] =
            std::clamp((m_sum_l_phim[dof] + psi_minn[dof]) / (sum_phip[dof] + root_v_small),
                       alpha_min,
                       alpha_max);
      }
      // Both processes sharing a partition face now limit it from the same cell limiters, so they
      // accept the same face limiter without a further reduction.
      exchangePartitionCellLimiters(face_corrections, lambda_plus, lambda_minus);

      bool changed_lambda = false;
      for (const auto i : index_range(face_corrections))
      {
        const auto & data = face_corrections[i];
        const Real phi_corr_f = raw_correction_flux[i];
        if (std::abs(phi_corr_f) <= _flux_floor || accepted_lambda[i] <= 0.0)
          continue;

        Real lambda = accepted_lambda[i];

        // A correction leaving the element into the neighbour is limited by the element's
        // outflow limiter and the neighbour's inflow limiter, and the other way round. A boundary
        // face is limited by its cell alone, and only when the corrected flux leaves the domain:
        // an inflow carries a prescribed value that is bounded already.
        if (data.boundary_kind == BoundaryFaceKind::Internal)
        {
          if (phi_corr_f > 0.0)
            lambda = std::min(lambda,
                              std::min(libmesh_map_find(lambda_plus, data.elem_dof),
                                       libmesh_map_find(lambda_minus, data.neighbor_dof)));
          else
            lambda = std::min(lambda,
                              std::min(libmesh_map_find(lambda_minus, data.elem_dof),
                                       libmesh_map_find(lambda_plus, data.neighbor_dof)));
        }
        else
        {
          // The target flux, which the working flux reaches once the raw correction is applied
          const Real corrected_boundary_flux = data.donor_flux + data.correction_flux;
          if (corrected_boundary_flux > _flux_floor)
          {
            if (phi_corr_f > 0.0)
              lambda = std::min(lambda, libmesh_map_find(lambda_plus, data.elem_dof));
            else
              lambda = std::min(lambda, libmesh_map_find(lambda_minus, data.elem_dof));
          }
        }

        // lambda only decreased from accepted_lambda[i], through cell limiters clamped to [0, 1]
        const Real updated_lambda = lambda;
        changed_lambda = changed_lambda || updated_lambda + libMesh::TOLERANCE < accepted_lambda[i];
        accepted_lambda[i] = updated_lambda;
      }

      unsigned int changed_anywhere = changed_lambda ? 1 : 0;
      _communicator.max(changed_anywhere);
      if (!changed_anywhere)
        break;
    }

    // Apply the limited correction to alpha explicitly: the flux carries alpha from the element
    // to the neighbour, so the element loses dt lambda phi_corr / V and the neighbour gains it.
    // The same limited flux is what gets published, so alpha and alpha_phi_limited stay
    // consistent to round-off.
    for (const auto i : index_range(face_corrections))
      lambda_bound[face_corrections[i].face->id()] = accepted_lambda[i];
    for (const auto i : index_range(face_corrections))
      if (std::abs(raw_correction_flux[i]) > _flux_floor)
      {
        const Real correction_weight = correction_it == 0 ? 1.0 : later_correction_relaxation;
        limited_correction_flux[i] =
            correction_weight * accepted_lambda[i] * raw_correction_flux[i];
        const auto & data = face_corrections[i];
        if (data.elem_owned)
          applied_change[data.elem_dof] +=
              -dt * limited_correction_flux[i] / NS::cellMeasure(*data.face->elemInfo());
        if (data.neighbor_owned)
          applied_change[data.neighbor_dof] +=
              dt * limited_correction_flux[i] / NS::cellMeasure(*data.face->neighborInfo());
      }

    // Owned entries only; the close brings the ghosted copies up to date
    for (const auto & [dof, change] : applied_change)
      current_local_solution.add(dof, change);
    current_local_solution.close();
    _system->solution() = current_local_solution;
    _system->solution().close();
    _system->setSolution(current_local_solution);
    _system->computeGradients();

    for (const auto i : index_range(face_corrections))
      working_alpha_flux[i] += limited_correction_flux[i];

    if (correction_it + 1 == _num_alpha_corrections)
      publishFaceFluxes(face_corrections, working_alpha_flux, subcycle_fraction);
  }

  _system->computeGradients();
}
