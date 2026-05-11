//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SharpInterfaceVOFMULESCorrector.h"

#include "LinearFVBoundaryCondition.h"
#include "LinearFVAdvectionDiffusionExtrapolatedBC.h"
#include "LinearFVAdvectionDiffusionFunctorDirichletBC.h"
#include "LinearSystem.h"
#include "MooseLinearVariableFV.h"
#include "MooseFunctorArguments.h"
#include "FEProblemBase.h"
#include "SubProblem.h"
#include "MooseMesh.h"
#include "FaceInfo.h"
#include "ElemInfo.h"
#include "SegregatedSolverUtils.h"
#include "MathFVUtils.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <unordered_map>
#include <vector>

registerMooseObject("NavierStokesApp", SharpInterfaceVOFMULESCorrector);

InputParameters
SharpInterfaceVOFMULESCorrector::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params += NonADFunctorInterface::validParams();
  params += BlockRestrictable::validParams();
  params.addClassDescription(
      "Applies an interFoam-style explicit bounded correction to a donor/upwind volume-fraction "
      "transport solve.");
  params.addRequiredParam<SolverSystemName>("system_name",
                                            "The linear system transporting the volume fraction.");
  params.addRequiredParam<VariableName>("variable",
                                        "The transported volume-fraction variable.");
  params.addRequiredParam<UserObjectName>(
      "rhie_chow_user_object",
      "The Rhie-Chow user object used to obtain physical volumetric face fluxes.");
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
  MooseEnum high_order_correction_scheme("venkatakrishnan vanLeer", "venkatakrishnan");
  params.addParam<MooseEnum>(
      "high_order_correction_scheme",
      high_order_correction_scheme,
      "Higher-order correction flux added on top of the donor/upwind base flux before MULES-style "
      "limiting.");
  params.addRangeCheckedParam<unsigned int>(
      "n_alpha_corrections", 2, "n_alpha_corrections>0", "Number of explicit correction sweeps.");
  params.addRangeCheckedParam<unsigned int>(
      "n_limiter_iterations",
      5,
      "n_limiter_iterations>0",
      "Number of limiter tightening passes for each correction sweep.");
  params.addRangeCheckedParam<Real>(
      "correction_relaxation",
      1.0,
      "correction_relaxation>0 & correction_relaxation<=1",
      "Under-relaxation applied to each bounded explicit correction update.");
  params.addParam<bool>(
      "alpha_apply_prev_corr",
      true,
      "Whether to reuse the previous limited correction flux as the initial correction guess on "
      "the next alpha solve, mirroring interFoam's alphaApplyPrevCorr lifecycle.");
  params.addParam<Real>("min_value", 0.0, "Lower admissible volume-fraction bound.");
  params.addParam<Real>("max_value", 1.0, "Upper admissible volume-fraction bound.");
  params.addParam<bool>("debug_dump_subcycle",
                        false,
                        "Dump donor/correction/limiter data for interface faces during alpha "
                        "subcycling.");
  params.addParam<bool>("debug_only_first_subcycle",
                        true,
                        "If true, only dump debug data for the first alpha subcycle.");
  params.addRangeCheckedParam<unsigned int>(
      "debug_dump_max_faces", 12, "debug_dump_max_faces>0", "Maximum number of faces to dump.");
  params.addRangeCheckedParam<Real>(
      "debug_interface_alpha_tolerance",
      1e-10,
      "debug_interface_alpha_tolerance>=0",
      "Minimum alpha jump across a face before it is treated as an interface face for debug "
      "output.");
  params.addParam<MooseFunctorName>("alpha_phi_bd_functor_name",
                                    "alpha_phi_bd",
                                    "Published donor/base alpha face flux.");
  params.addParam<MooseFunctorName>("alpha_phi_ho_functor_name",
                                    "alpha_phi_ho",
                                    "Published high-order advective alpha face flux.");
  params.addParam<MooseFunctorName>("alpha_phi_comp_functor_name",
                                    "alpha_phi_comp",
                                    "Published explicit compressive alpha face flux.");
  params.addParam<MooseFunctorName>("alpha_phi_corr_raw_functor_name",
                                    "alpha_phi_corr_raw",
                                    "Published raw correction alpha face flux prior to limiting.");
  params.addParam<MooseFunctorName>("alpha_phi_corr_functor_name",
                                    "alpha_phi_corr",
                                    "Published limited correction alpha face flux.");
  params.addParam<MooseFunctorName>("alpha_phi_limited_functor_name",
                                    "alpha_phi_limited",
                                    "Published limited alpha face flux.");
  params.addParam<MooseFunctorName>("rho_phi_functor_name",
                                    "rho_phi",
                                    "Published density-weighted face flux accumulated over alpha "
                                    "subcycles.");

  ExecFlagEnum & exec_enum = params.set<ExecFlagEnum>("execute_on", true);
  exec_enum.addAvailableFlags(EXEC_NONE);
  exec_enum = {EXEC_NONE};
  params.suppressParameter<ExecFlagEnum>("execute_on");
  return params;
}

SharpInterfaceVOFMULESCorrector::SharpInterfaceVOFMULESCorrector(const InputParameters & params)
  : GeneralUserObject(params),
    NonADFunctorInterface(this),
    BlockRestrictable(this),
    _system_name(getParam<SolverSystemName>("system_name")),
    _variable_name(getParam<VariableName>("variable")),
    _mass_flux_provider(getUserObject<RhieChowMassFlux>("rhie_chow_user_object")),
    _compression_factor(getFunctor<Real>("compression_factor")),
    _interface_normal(getFunctor<RealVectorValue>("interface_normal")),
    _liquid_density(getFunctor<Real>("liquid_density")),
    _gas_density(getFunctor<Real>("gas_density")),
    _high_order_correction_scheme(
        getParam<MooseEnum>("high_order_correction_scheme") == "vanLeer"
            ? HighOrderCorrectionScheme::VanLeer
            : HighOrderCorrectionScheme::Venkatakrishnan),
    _num_alpha_corrections(getParam<unsigned int>("n_alpha_corrections")),
    _num_limiter_iterations(getParam<unsigned int>("n_limiter_iterations")),
    _correction_relaxation(getParam<Real>("correction_relaxation")),
    _min_value(getParam<Real>("min_value")),
    _max_value(getParam<Real>("max_value")),
    _alpha_apply_prev_corr(getParam<bool>("alpha_apply_prev_corr")),
    _debug_dump_subcycle(getParam<bool>("debug_dump_subcycle")),
    _debug_only_first_subcycle(getParam<bool>("debug_only_first_subcycle")),
    _debug_dump_max_faces(getParam<unsigned int>("debug_dump_max_faces")),
    _debug_interface_alpha_tolerance(getParam<Real>("debug_interface_alpha_tolerance")),
    _alpha_phi_bd(_fe_problem.mesh(), blockIDs(), getParam<MooseFunctorName>("alpha_phi_bd_functor_name")),
    _alpha_phi_ho(_fe_problem.mesh(), blockIDs(), getParam<MooseFunctorName>("alpha_phi_ho_functor_name")),
    _alpha_phi_comp(_fe_problem.mesh(),
                    blockIDs(),
                    getParam<MooseFunctorName>("alpha_phi_comp_functor_name")),
    _alpha_phi_corr_raw(
        _fe_problem.mesh(), blockIDs(), getParam<MooseFunctorName>("alpha_phi_corr_raw_functor_name")),
    _alpha_phi_corr(_fe_problem.mesh(), blockIDs(), getParam<MooseFunctorName>("alpha_phi_corr_functor_name")),
    _alpha_phi_limited(_fe_problem.mesh(),
                       blockIDs(),
                       getParam<MooseFunctorName>("alpha_phi_limited_functor_name")),
    _rho_phi(_fe_problem.mesh(), blockIDs(), getParam<MooseFunctorName>("rho_phi_functor_name"))
{
  if (_min_value > _max_value)
    paramError("max_value", "max_value must be >= min_value.");

  for (const auto tid : make_range(libMesh::n_threads()))
  {
    UserObject::_subproblem.addFunctor(
        getParam<MooseFunctorName>("alpha_phi_bd_functor_name"), _alpha_phi_bd, tid);
    UserObject::_subproblem.addFunctor(
        getParam<MooseFunctorName>("alpha_phi_ho_functor_name"), _alpha_phi_ho, tid);
    UserObject::_subproblem.addFunctor(
        getParam<MooseFunctorName>("alpha_phi_comp_functor_name"), _alpha_phi_comp, tid);
    UserObject::_subproblem.addFunctor(
        getParam<MooseFunctorName>("alpha_phi_corr_raw_functor_name"), _alpha_phi_corr_raw, tid);
    UserObject::_subproblem.addFunctor(
        getParam<MooseFunctorName>("alpha_phi_corr_functor_name"), _alpha_phi_corr, tid);
    UserObject::_subproblem.addFunctor(
        getParam<MooseFunctorName>("alpha_phi_limited_functor_name"), _alpha_phi_limited, tid);
    UserObject::_subproblem.addFunctor(
        getParam<MooseFunctorName>("rho_phi_functor_name"), _rho_phi, tid);
  }
}

void
SharpInterfaceVOFMULESCorrector::initialSetup()
{
  cacheSystemData();
  initializeFluxStorage();
}

void
SharpInterfaceVOFMULESCorrector::meshChanged()
{
  cacheSystemData();
  initializeFluxStorage();
  invalidatePreviousCorrectionFluxes();
}

void
SharpInterfaceVOFMULESCorrector::cacheSystemData()
{
  _system = &_fe_problem.getLinearSystem(_fe_problem.linearSysNum(_system_name));
  _alpha_var =
      dynamic_cast<MooseLinearVariableFVReal *>(&UserObject::_subproblem.getVariable(0, _variable_name));

  if (!_alpha_var)
    paramError("variable", "The volume-fraction variable must be a MooseLinearVariableFVReal.");

  _sys_num = _alpha_var->sys().number();
  _var_num = _alpha_var->number();
  _alpha_var->computeCellLimitedGradients(Moose::FV::GradientLimiterType::Venkatakrishnan);
}

void
SharpInterfaceVOFMULESCorrector::initializeFluxStorage()
{
  for (const auto * fi : _fe_problem.mesh().faceInfo())
  {
    _alpha_phi_bd[fi->id()] = 0.0;
    _alpha_phi_ho[fi->id()] = 0.0;
    _alpha_phi_comp[fi->id()] = 0.0;
    _alpha_phi_corr_raw[fi->id()] = 0.0;
    _alpha_phi_corr[fi->id()] = 0.0;
    _alpha_phi_limited[fi->id()] = 0.0;
    _rho_phi[fi->id()] = 0.0;
    _alpha_phi_corr_prev.try_emplace(fi->id(), 0.0);
  }
}

void
SharpInterfaceVOFMULESCorrector::invalidatePreviousCorrectionFluxes()
{
  for (auto & pair : _alpha_phi_corr_prev)
    pair.second = 0.0;

  _previous_correction_flux_valid = false;
}

Real
SharpInterfaceVOFMULESCorrector::cellVolume(const ElemInfo & elem_info) const
{
  return elem_info.volume() * elem_info.coordFactor();
}

Real
SharpInterfaceVOFMULESCorrector::faceMeasure(const FaceInfo & fi) const
{
  return fi.faceArea() * fi.faceCoord();
}

Real
SharpInterfaceVOFMULESCorrector::solutionAlpha(const NumericVector<Number> & solution,
                                               const ElemInfo & elem_info) const
{
  const auto dof = elem_info.dofIndices()[_sys_num][_var_num];
  if (dof == DofObject::invalid_id)
    return 0.0;

  return std::min(_max_value, std::max(_min_value, static_cast<Real>(solution(dof))));
}

Real
SharpInterfaceVOFMULESCorrector::integrateLiquidVolume(const NumericVector<Number> & solution) const
{
  Real local_liquid_volume = 0.0;

  for (const auto & elem_info : _fe_problem.mesh().elemInfoVector())
  {
    if (!elem_info || !hasBlocks(elem_info->subdomain_id()))
      continue;

    const auto * const elem = elem_info->elem();
    if (!elem || elem->processor_id() != processor_id())
      continue;

    local_liquid_volume += solutionAlpha(solution, *elem_info) * cellVolume(*elem_info);
  }

  _communicator.sum(local_liquid_volume);
  return local_liquid_volume;
}

Real
SharpInterfaceVOFMULESCorrector::cellAlpha(const ElemInfo & elem_info) const
{
  return std::min(_max_value,
                  std::max(_min_value, _alpha_var->getElemValue(elem_info, Moose::currentState())));
}

SharpInterfaceVOFMULESCorrector::LiquidVolumeAudit
SharpInterfaceVOFMULESCorrector::liquidVolumeAudit() const
{
  LiquidVolumeAudit audit;
  if (!_system)
    return audit;

  audit.current = integrateLiquidVolume(*(_system->system().current_local_solution));
  audit.timestep_old = integrateLiquidVolume(_system->solutionOld());

  if (_system->hasSolutionState(1, Moose::SolutionIterationType::Nonlinear))
  {
    audit.previous_outer = integrateLiquidVolume(
        _system->solutionState(1, Moose::SolutionIterationType::Nonlinear));
    audit.has_previous_outer = true;
  }

  return audit;
}

SharpInterfaceVOFMULESCorrector::RhoPhiConsistencyAudit
SharpInterfaceVOFMULESCorrector::rhoPhiConsistencyAudit() const
{
  RhoPhiConsistencyAudit audit;

  Real local_rho_phi_mismatch_sq_sum = 0.0;
  Real local_max_abs_mismatch = 0.0;

  for (const auto * fi : _fe_problem.mesh().faceInfo())
  {
    if (!fi || fi->processor_id() != processor_id())
      continue;

    const auto face_type = fi->faceType(std::make_pair(_var_num, _sys_num));
    if (face_type == FaceInfo::VarFaceNeighbors::NEITHER)
      continue;

    if (!hasBlocks(fi->elemSubdomainID()) &&
        !(fi->neighborPtr() && hasBlocks(fi->neighborSubdomainID())))
      continue;

    const auto face_id = fi->id();
    const bool have_rho_phi = _rho_phi.count(face_id);
    const bool have_limited_alpha_flux = _alpha_phi_limited.count(face_id);
    if (!have_rho_phi && !have_limited_alpha_flux)
      continue;

    const Real stored_rho_phi = have_rho_phi ? libmesh_map_find(_rho_phi, face_id) : 0.0;
    const Real limited_alpha_flux =
        have_limited_alpha_flux ? libmesh_map_find(_alpha_phi_limited, face_id) : 0.0;
    const Real gas_density = faceFunctorAverage(*fi, _gas_density);
    const Real liquid_density = faceFunctorAverage(*fi, _liquid_density);
    const Real volumetric_phi = _mass_flux_provider.getVolumetricFaceFlux(*fi);
    const Real reconstructed_rho_phi =
        volumetric_phi * faceMeasure(*fi) * gas_density +
        (liquid_density - gas_density) * limited_alpha_flux;
    const Real mismatch = stored_rho_phi - reconstructed_rho_phi;
    const Real abs_mismatch = std::abs(mismatch);

    local_rho_phi_mismatch_sq_sum += mismatch * mismatch;

    if (abs_mismatch > local_max_abs_mismatch)
    {
      local_max_abs_mismatch = abs_mismatch;
      audit.has_worst_face = true;
      audit.worst_face_id = face_id;
      audit.worst_face_centroid = fi->faceCentroid();
      audit.stored_rho_phi = stored_rho_phi;
      audit.reconstructed_rho_phi = reconstructed_rho_phi;
      audit.volumetric_phi = volumetric_phi;
      audit.limited_alpha_flux = limited_alpha_flux;
      audit.gas_density = gas_density;
      audit.liquid_density = liquid_density;
    }
  }

  _communicator.sum(local_rho_phi_mismatch_sq_sum);
  audit.l2_norm = std::sqrt(local_rho_phi_mismatch_sq_sum);

  processor_id_type worst_face_pid = processor_id();
  _communicator.maxloc(local_max_abs_mismatch, worst_face_pid);
  audit.max_abs_mismatch = local_max_abs_mismatch;
  if (worst_face_pid != processor_id())
    audit.has_worst_face = false;

  return audit;
}

void
SharpInterfaceVOFMULESCorrector::resetSubcycleFluxes()
{
  _subcycle_counter = 0;
  // Mirror OpenFOAM's createAlphaFluxes.H lifecycle: clear the working
  // current-solve alpha/rhoPhi accumulators, but keep the previous limited
  // correction flux stored for the next alpha solve.
  for (auto & pair : _alpha_phi_bd)
    pair.second = 0.0;
  for (auto & pair : _alpha_phi_ho)
    pair.second = 0.0;
  for (auto & pair : _alpha_phi_comp)
    pair.second = 0.0;
  for (auto & pair : _alpha_phi_corr_raw)
    pair.second = 0.0;
  for (auto & pair : _alpha_phi_corr)
    pair.second = 0.0;
  for (auto & pair : _alpha_phi_limited)
    pair.second = 0.0;
  for (auto & pair : _rho_phi)
    pair.second = 0.0;
}

void
SharpInterfaceVOFMULESCorrector::invalidateOuterCorrectionFluxSeed()
{
  invalidatePreviousCorrectionFluxes();
}

LinearFVBoundaryCondition *
SharpInterfaceVOFMULESCorrector::boundaryCondition(const FaceInfo & fi) const
{
  for (const auto bnd_id : fi.boundaryIDs())
    if (const auto & bc_map = _alpha_var->getBoundaryConditionMap();
        bc_map.find(bnd_id) != bc_map.end())
      return bc_map.at(bnd_id);

  return nullptr;
}

Real
SharpInterfaceVOFMULESCorrector::boundaryValue(const FaceInfo & fi,
                                               FaceInfo::VarFaceNeighbors face_type) const
{
  if (auto * bc = boundaryCondition(fi))
  {
    bc->setupFaceData(&fi, face_type);
    return std::min(_max_value, std::max(_min_value, bc->computeBoundaryValue()));
  }

  const auto * fluid_info = face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR ? fi.neighborInfo()
                                                                               : fi.elemInfo();
  return fluid_info ? cellAlpha(*fluid_info) : 0.0;
}

Real
SharpInterfaceVOFMULESCorrector::donorFlux(const FaceInfo & fi) const
{
  const auto face_type = fi.faceType(std::make_pair(_var_num, _sys_num));
  if (face_type == FaceInfo::VarFaceNeighbors::NEITHER)
    return 0.0;

  const Real volumetric_flux = _mass_flux_provider.getVolumetricFaceFlux(fi);
  const bool upwind_is_elem = volumetric_flux >= 0.0;
  const Real elem_alpha = cellAlpha(*fi.elemInfo());

  Real donor_alpha = elem_alpha;
  if (fi.neighborPtr() && face_type == FaceInfo::VarFaceNeighbors::BOTH)
    donor_alpha = upwind_is_elem ? elem_alpha : cellAlpha(*fi.neighborInfo());
  else if (_alpha_var->isDirichletBoundaryFace(fi))
    donor_alpha = upwind_is_elem ? elem_alpha : boundaryValue(fi, face_type);

  return volumetric_flux * donor_alpha * faceMeasure(fi);
}

Real
SharpInterfaceVOFMULESCorrector::highOrderFlux(const FaceInfo & fi) const
{
  const auto face_type = fi.faceType(std::make_pair(_var_num, _sys_num));
  if (face_type == FaceInfo::VarFaceNeighbors::NEITHER)
    return 0.0;

  const Real volumetric_flux = _mass_flux_provider.getVolumetricFaceFlux(fi);
  const bool upwind_is_elem = volumetric_flux >= 0.0;
  auto * bc = boundaryCondition(fi);

  Real high_order_alpha = 0.0;
  if (fi.neighborPtr() && face_type == FaceInfo::VarFaceNeighbors::BOTH)
    switch (_high_order_correction_scheme)
    {
      case HighOrderCorrectionScheme::Venkatakrishnan:
        high_order_alpha = venkatakrishnanFaceValue(fi, upwind_is_elem);
        break;
      case HighOrderCorrectionScheme::VanLeer:
        high_order_alpha = vanLeerFaceValue(fi, upwind_is_elem);
        break;
    }
  else if (auto * dirichlet_bc = dynamic_cast<LinearFVAdvectionDiffusionFunctorDirichletBC *>(bc))
  {
    (void)dirichlet_bc;
    // Keep the one-sided boundary reconstruction on open outflow even when Van Leer is selected.
    // The Van Leer correction is only defined here for internal faces with a true downwind cell.
    high_order_alpha =
        upwind_is_elem ? venkatakrishnanFaceValue(fi, true) : boundaryValue(fi, face_type);
  }
  else if (upwind_is_elem &&
           dynamic_cast<LinearFVAdvectionDiffusionExtrapolatedBC *>(bc))
    high_order_alpha = boundaryValue(fi, face_type);
  else
    return donorFlux(fi);

  return volumetric_flux * high_order_alpha * faceMeasure(fi);
}

Real
SharpInterfaceVOFMULESCorrector::venkatakrishnanFaceValue(const FaceInfo & fi,
                                                          const bool upwind_is_elem) const
{
  const auto state = Moose::currentState();
  const auto & upwind_info = upwind_is_elem ? *fi.elemInfo() : *fi.neighborInfo();
  const auto alpha_upwind = cellAlpha(upwind_info);
  const auto grad_upwind =
      _alpha_var->limitedGradSln(upwind_info, state, Moose::FV::GradientLimiterType::Venkatakrishnan);
  const Point & upwind_centroid = upwind_is_elem ? fi.elemCentroid() : fi.neighborCentroid();
  const Real reconstructed = alpha_upwind + grad_upwind * (fi.faceCentroid() - upwind_centroid);
  return std::min(_max_value, std::max(_min_value, reconstructed));
}

Real
SharpInterfaceVOFMULESCorrector::vanLeerFaceValue(const FaceInfo & fi,
                                                  const bool upwind_is_elem) const
{
  mooseAssert(fi.neighborPtr(), "Van Leer correction requires an internal face with a neighbor.");

  const auto state = Moose::currentState();
  const auto & upwind_info = upwind_is_elem ? *fi.elemInfo() : *fi.neighborInfo();
  const auto & downwind_info = upwind_is_elem ? *fi.neighborInfo() : *fi.elemInfo();

  const Real phi_upwind = cellAlpha(upwind_info);
  const Real phi_downwind = cellAlpha(downwind_info);
  const VectorValue<Real> grad_upwind = _alpha_var->gradSln(upwind_info, state);
  const Point upwind_to_downwind = upwind_is_elem ? fi.dCN() : Point(-fi.dCN());

  const Real r_f = Moose::FV::rF(phi_upwind, phi_downwind, grad_upwind, upwind_to_downwind);
  const Real beta = (r_f + std::abs(r_f)) / (1.0 + std::abs(r_f));

  const Real w_f = upwind_is_elem ? fi.gC() : (1.0 - fi.gC());
  const Real g_unclamped = beta * (1.0 - w_f);
  const Real g = std::min(std::max(g_unclamped, 0.0), 1.0 - w_f);

  const Real phi_face = (1.0 - g) * phi_upwind + g * phi_downwind;
  return std::min(_max_value, std::max(_min_value, phi_face));
}

SharpInterfaceVOFMULESCorrector::BoundaryFaceKind
SharpInterfaceVOFMULESCorrector::classifyBoundaryFace(const FaceInfo & fi,
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

  if (dynamic_cast<LinearFVAdvectionDiffusionExtrapolatedBC *>(bc))
    return volumetric_flux >= 0.0 ? BoundaryFaceKind::OpenOutflow : BoundaryFaceKind::Closed;

  return BoundaryFaceKind::Closed;
}

const char *
SharpInterfaceVOFMULESCorrector::boundaryFaceKindName(const BoundaryFaceKind kind) const
{
  switch (kind)
  {
    case BoundaryFaceKind::Internal:
      return "internal";
    case BoundaryFaceKind::DirichletInflow:
      return "dirichlet_inflow";
    case BoundaryFaceKind::DirichletOutflow:
      return "dirichlet_outflow";
    case BoundaryFaceKind::OpenOutflow:
      return "open_outflow";
    case BoundaryFaceKind::Closed:
      return "closed";
  }

  return "unknown";
}

Real
SharpInterfaceVOFMULESCorrector::compressionFlux(const FaceInfo & fi,
                                                 const Real elem_alpha,
                                                 const Real neighbor_alpha) const
{
  const auto state = Moose::currentState();
  const Moose::FaceArg face_arg{
      &fi, Moose::FV::LimiterType::CentralDifference, true, false, nullptr, nullptr};

  const RealVectorValue face_normal = fi.normal();
  const Real face_normal_mag = face_normal.norm();
  const RealVectorValue face_unit_normal =
      face_normal_mag > 0.0 ? face_normal / face_normal_mag : RealVectorValue();
  const RealVectorValue interface_normal = MetaPhysicL::raw_value(_interface_normal(face_arg, state));
  const Real alpha_face =
      std::min(_max_value, std::max(_min_value, 0.5 * (elem_alpha + neighbor_alpha)));

  return std::abs(_mass_flux_provider.getVolumetricFaceFlux(fi)) *
         MetaPhysicL::raw_value(_compression_factor(face_arg, state)) *
         (interface_normal * face_unit_normal) * alpha_face * (1.0 - alpha_face) *
         faceMeasure(fi);
}

Real
SharpInterfaceVOFMULESCorrector::faceFunctorAverage(const FaceInfo & fi,
                                                    const Moose::Functor<Real> & functor) const
{
  const auto state = Moose::currentState();
  if (!fi.neighborPtr())
  {
    const Elem * fluid_elem = fi.elemPtr();
    if (!hasBlocks(fluid_elem->subdomain_id()) && fi.neighborPtr())
      fluid_elem = fi.neighborPtr();
    const Moose::FaceArg face_arg{&fi,
                                  Moose::FV::LimiterType::CentralDifference,
                                  true,
                                  false,
                                  fluid_elem,
                                  nullptr};
    return MetaPhysicL::raw_value(functor(face_arg, state));
  }

  const Real elem_value = MetaPhysicL::raw_value(functor(Moose::ElemArg{fi.elemPtr(), false}, state));

  const Real neighbor_value =
      MetaPhysicL::raw_value(functor(Moose::ElemArg{fi.neighborPtr(), false}, state));
  return 0.5 * (elem_value + neighbor_value);
}

Real
SharpInterfaceVOFMULESCorrector::rhoPhi(const FaceInfo & fi, const Real limited_alpha_flux) const
{
  const Real gas_density = faceFunctorAverage(fi, _gas_density);
  const Real liquid_density = faceFunctorAverage(fi, _liquid_density);
  const Real volumetric_mass_flux =
      _mass_flux_provider.getVolumetricFaceFlux(fi) * faceMeasure(fi) * gas_density;
  return volumetric_mass_flux + (liquid_density - gas_density) * limited_alpha_flux;
}

SharpInterfaceVOFMULESCorrector::FaceCorrectionData
SharpInterfaceVOFMULESCorrector::buildFaceCorrectionData(const FaceInfo & fi) const
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

  data.donor_flux = donorFlux(fi);
  data.high_order_flux = highOrderFlux(fi);

  const Real volumetric_flux = _mass_flux_provider.getVolumetricFaceFlux(fi);
  const Real elem_alpha = cellAlpha(*fi.elemInfo());
  data.boundary_kind = classifyBoundaryFace(fi, face_type, volumetric_flux);
  data.boundary_face = data.boundary_kind != BoundaryFaceKind::Internal;

  if (data.boundary_kind == BoundaryFaceKind::Internal)
  {
    const Real neighbor_alpha = cellAlpha(*fi.neighborInfo());
    data.compressive_flux = compressionFlux(fi, elem_alpha, neighbor_alpha);
    data.advective_correction_flux = data.high_order_flux - data.donor_flux;
    data.correction_flux = data.advective_correction_flux + data.compressive_flux;
    return data;
  }

  // Phase-7 boundary policy:
  // - prescribed inflow alpha remains donor-only in the explicit correction stage
  // - prescribed outflow alpha may reuse its one-sided advective face value, but no compression
  // - extrapolated/open outlets remain donor-only in the explicit correction stage
  // - closed boundaries remain donor-only in the explicit correction stage
  if (data.boundary_kind == BoundaryFaceKind::DirichletOutflow)
  {
    data.advective_correction_flux = data.high_order_flux - data.donor_flux;
    data.correction_flux = data.advective_correction_flux;
  }

  return data;
}

void
SharpInterfaceVOFMULESCorrector::clampSolution() const
{
  auto & current_local_solution = *(_system->system().current_local_solution);
  NS::FV::limitSolutionUpdate(current_local_solution, _min_value, _max_value);
  _system->setSolution(current_local_solution);
}

void
SharpInterfaceVOFMULESCorrector::publishFaceFluxes(
    const std::vector<FaceCorrectionData> & face_corrections,
    const std::vector<Real> & raw_correction_fluxes,
    const std::vector<Real> & limited_correction_fluxes,
    const Real subcycle_fraction)
{
  for (const auto i : index_range(face_corrections))
  {
    const auto & data = face_corrections[i];
    const auto face_id = data.face->id();
    const Real limited_correction = limited_correction_fluxes[i];
    const Real limited_alpha_flux = data.donor_flux + limited_correction;

    // Accumulate the published face fluxes with the same subcycle weighting as rhoPhi so
    // downstream consumers see a timestep-consistent alphaPhi/rhoPhi pair after subcycling.
    _alpha_phi_bd[face_id] += subcycle_fraction * data.donor_flux;
    _alpha_phi_ho[face_id] += subcycle_fraction * data.high_order_flux;
    _alpha_phi_comp[face_id] += subcycle_fraction * data.compressive_flux;
    _alpha_phi_corr_raw[face_id] += subcycle_fraction * raw_correction_fluxes[i];
    _alpha_phi_corr[face_id] += subcycle_fraction * limited_correction;
    _alpha_phi_limited[face_id] += subcycle_fraction * limited_alpha_flux;
    _rho_phi[face_id] += subcycle_fraction * rhoPhi(*data.face, limited_alpha_flux);
  }
}

void
SharpInterfaceVOFMULESCorrector::applyCorrection(const Real dt, const Real subcycle_fraction)
{
  ++_subcycle_counter;
  if (!_system || !_alpha_var || dt <= 0.0)
    return;

  std::vector<FaceCorrectionData> published_face_corrections;
  std::vector<Real> published_raw_correction_fluxes;
  std::vector<Real> published_limited_correction_fluxes;
  std::vector<Real> published_accepted_lambda;
  std::unordered_map<dof_id_type, Real> alpha_before_correction;
  std::unordered_map<dof_id_type, Real> alpha_after_correction;
  const bool use_previous_correction_flux = _alpha_apply_prev_corr && _previous_correction_flux_valid;

  for (const auto correction_it : make_range(_num_alpha_corrections))
  {
    (void)correction_it;
    _system->computeGradients();

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
      if (!data.face)
        continue;

      face_corrections.push_back(data);
    }

    if (face_corrections.empty())
      return;

    auto & current_local_solution = *(_system->system().current_local_solution);
    std::unordered_map<dof_id_type, Real> applied_change;
    std::vector<Real> raw_correction_flux(face_corrections.size(), 0.0);
    std::vector<Real> limited_correction_flux(face_corrections.size(), 0.0);
    std::vector<Real> accepted_lambda(face_corrections.size(), 0.0);

    for (const auto i : index_range(face_corrections))
    {
      raw_correction_flux[i] = face_corrections[i].correction_flux;
      accepted_lambda[i] = std::abs(raw_correction_flux[i]) > libMesh::TOLERANCE ? 1.0 : 0.0;
    }

    if (correction_it == 0 && use_previous_correction_flux)
      for (const auto i : index_range(face_corrections))
      {
        raw_correction_flux[i] += _alpha_phi_corr_prev[face_corrections[i].face->id()];
        accepted_lambda[i] = std::abs(raw_correction_flux[i]) > libMesh::TOLERANCE ? 1.0 : 0.0;
      }

    if (_debug_dump_subcycle && correction_it == 0)
    {
      alpha_before_correction.clear();
      for (const auto & data : face_corrections)
      {
        alpha_before_correction[data.elem_dof] = current_local_solution(data.elem_dof);
        if (data.has_neighbor)
          alpha_before_correction[data.neighbor_dof] = current_local_solution(data.neighbor_dof);
      }
    }

    for (const auto limiter_it : make_range(_num_limiter_iterations))
    {
      (void)limiter_it;
      std::unordered_map<dof_id_type, Real> remaining_increase_budget;
      std::unordered_map<dof_id_type, Real> remaining_decrease_budget;

      const auto initialize_cell_budgets = [&](const dof_id_type dof)
      {
        if (remaining_increase_budget.count(dof))
          return;

        const Real alpha = std::min(_max_value, std::max(_min_value, current_local_solution(dof)));
        remaining_increase_budget.emplace(dof, _max_value - alpha);
        remaining_decrease_budget.emplace(dof, alpha - _min_value);
      };

      for (const auto & data : face_corrections)
      {
        initialize_cell_budgets(data.elem_dof);
        if (data.has_neighbor)
          initialize_cell_budgets(data.neighbor_dof);
      }

      bool changed_lambda = false;
      for (const auto i : index_range(face_corrections))
      {
        const auto & data = face_corrections[i];
        const Real raw_flux = raw_correction_flux[i];
        if (std::abs(raw_flux) < libMesh::TOLERANCE || accepted_lambda[i] <= 0.0)
          continue;

        Real lambda = accepted_lambda[i];

        const Real elem_delta = -dt * raw_flux / cellVolume(*data.face->elemInfo());
        const Real elem_delta_mag = std::abs(elem_delta);
        if (elem_delta_mag > libMesh::TOLERANCE)
        {
          const Real elem_budget =
              elem_delta >= 0.0 ? remaining_increase_budget[data.elem_dof]
                                : remaining_decrease_budget[data.elem_dof];
          lambda = std::min(lambda, std::max(0.0, elem_budget) / elem_delta_mag);
        }

        if (data.has_neighbor)
        {
          const Real neighbor_delta = dt * raw_flux / cellVolume(*data.face->neighborInfo());
          const Real neighbor_delta_mag = std::abs(neighbor_delta);
          if (neighbor_delta_mag > libMesh::TOLERANCE)
          {
            const Real neighbor_budget =
                neighbor_delta >= 0.0 ? remaining_increase_budget[data.neighbor_dof]
                                      : remaining_decrease_budget[data.neighbor_dof];
            lambda = std::min(lambda, std::max(0.0, neighbor_budget) / neighbor_delta_mag);
          }
        }

        lambda = std::min(1.0, std::max(0.0, lambda));
        const Real updated_lambda = std::min(accepted_lambda[i], lambda);
        changed_lambda = changed_lambda || updated_lambda + libMesh::TOLERANCE < accepted_lambda[i];
        accepted_lambda[i] = updated_lambda;

        if (updated_lambda <= 0.0)
          continue;

        const Real limited_flux = updated_lambda * raw_flux;
        const Real limited_elem_delta =
            std::abs(-dt * limited_flux / cellVolume(*data.face->elemInfo()));
        if (limited_elem_delta > libMesh::TOLERANCE)
        {
          auto & elem_budget = elem_delta >= 0.0 ? remaining_increase_budget[data.elem_dof]
                                                 : remaining_decrease_budget[data.elem_dof];
          elem_budget = std::max(0.0, elem_budget - limited_elem_delta);
        }

        if (data.has_neighbor)
        {
          const Real neighbor_delta = dt * raw_flux / cellVolume(*data.face->neighborInfo());
          const Real limited_neighbor_delta =
              std::abs(dt * limited_flux / cellVolume(*data.face->neighborInfo()));
          if (limited_neighbor_delta > libMesh::TOLERANCE)
          {
            auto & neighbor_budget =
                neighbor_delta >= 0.0 ? remaining_increase_budget[data.neighbor_dof]
                                      : remaining_decrease_budget[data.neighbor_dof];
            neighbor_budget = std::max(0.0, neighbor_budget - limited_neighbor_delta);
          }
        }
      }

      if (!changed_lambda)
        break;
    }

    for (const auto i : index_range(face_corrections))
      if (std::abs(raw_correction_flux[i]) > libMesh::TOLERANCE)
      {
        limited_correction_flux[i] =
            _correction_relaxation * accepted_lambda[i] * raw_correction_flux[i];
        const auto & data = face_corrections[i];
        const Real bounded_elem_delta =
            -dt * limited_correction_flux[i] / cellVolume(*data.face->elemInfo());
        applied_change[data.elem_dof] += bounded_elem_delta;

        if (data.has_neighbor)
        {
          const Real bounded_neighbor_delta =
              dt * limited_correction_flux[i] / cellVolume(*data.face->neighborInfo());
          applied_change[data.neighbor_dof] += bounded_neighbor_delta;
        }
      }

    auto limited_update = current_local_solution.zero_clone();
    for (const auto & pair : applied_change)
      limited_update->set(pair.first, pair.second);
    limited_update->close();

    current_local_solution.add(*limited_update);
    current_local_solution.close();
    clampSolution();

    published_face_corrections = face_corrections;
    published_raw_correction_fluxes = raw_correction_flux;
    published_limited_correction_fluxes = limited_correction_flux;
    published_accepted_lambda = accepted_lambda;

    if (_debug_dump_subcycle && correction_it == 0)
    {
      alpha_after_correction.clear();
      for (const auto & data : face_corrections)
      {
        alpha_after_correction[data.elem_dof] = current_local_solution(data.elem_dof);
        if (data.has_neighbor)
          alpha_after_correction[data.neighbor_dof] = current_local_solution(data.neighbor_dof);
      }
    }
  }

  if (!published_face_corrections.empty())
  {
    publishFaceFluxes(published_face_corrections,
                      published_raw_correction_fluxes,
                      published_limited_correction_fluxes,
                      subcycle_fraction);

    for (auto & pair : _alpha_phi_corr_prev)
      pair.second = 0.0;
    for (const auto i : index_range(published_face_corrections))
      _alpha_phi_corr_prev[published_face_corrections[i].face->id()] =
          published_limited_correction_fluxes[i];
    _previous_correction_flux_valid = true;

    if (_debug_dump_subcycle && (!_debug_only_first_subcycle || _subcycle_counter == 1))
      dumpFaceDebug(published_face_corrections,
                    published_accepted_lambda,
                    alpha_before_correction,
                    alpha_after_correction,
                    _subcycle_counter);
  }
  else
    invalidatePreviousCorrectionFluxes();

  _system->computeGradients();
}

void
SharpInterfaceVOFMULESCorrector::dumpFaceDebug(
    const std::vector<FaceCorrectionData> & face_corrections,
    const std::vector<Real> & accepted_lambda,
    const std::unordered_map<dof_id_type, Real> & alpha_before,
    const std::unordered_map<dof_id_type, Real> & alpha_after,
    const unsigned int subcycle_index) const
{
  unsigned int dumped = 0;
  for (const auto i : index_range(face_corrections))
  {
    if (dumped >= _debug_dump_max_faces)
      break;

    const auto & data = face_corrections[i];
    const auto elem_before_it = alpha_before.find(data.elem_dof);
    const auto elem_after_it = alpha_after.find(data.elem_dof);
    if (elem_before_it == alpha_before.end() || elem_after_it == alpha_after.end())
      continue;

    const Real elem_before = elem_before_it->second;
    const Real elem_after = elem_after_it->second;
    Real neighbor_before = boundaryValue(
        *data.face, data.face->faceType(std::make_pair(_var_num, _sys_num)));
    Real neighbor_after = neighbor_before;

    if (data.has_neighbor)
    {
      const auto neighbor_before_it = alpha_before.find(data.neighbor_dof);
      const auto neighbor_after_it = alpha_after.find(data.neighbor_dof);
      if (neighbor_before_it == alpha_before.end() || neighbor_after_it == alpha_after.end())
        continue;
      neighbor_before = neighbor_before_it->second;
      neighbor_after = neighbor_after_it->second;
    }

    if (std::abs(elem_before - neighbor_before) < _debug_interface_alpha_tolerance)
      continue;

    _console << name() << " subcycle=" << subcycle_index << " face=" << data.face->id()
             << " elem_dof=" << data.elem_dof;
    if (data.has_neighbor)
      _console << " neighbor_dof=" << data.neighbor_dof;
    else
      _console << " boundary_face=1 boundary_kind=" << boundaryFaceKindName(data.boundary_kind);
    _console << " donor_flux=" << data.donor_flux << " high_order_flux=" << data.high_order_flux
             << " compressive_flux=" << data.compressive_flux
             << " correction_flux=" << data.correction_flux
             << " accepted_lambda=" << (i < accepted_lambda.size() ? accepted_lambda[i] : 0.0)
             << " elem_alpha_before=" << elem_before << " elem_alpha_after=" << elem_after
             << " neighbor_alpha_before=" << neighbor_before
             << " neighbor_alpha_after=" << neighbor_after << std::endl;
    ++dumped;
  }
}
