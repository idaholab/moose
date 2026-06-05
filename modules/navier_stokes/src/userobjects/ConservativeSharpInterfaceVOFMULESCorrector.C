//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConservativeSharpInterfaceVOFMULESCorrector.h"
#include "ConservativeSharpInterfaceCurvatureCalculator.h"
#include "ConservativeSharpInterfaceRhieChowMassFlux.h"

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
#include "SegregatedSolverUtils.h"
#include "MathFVUtils.h"
#include "Limiter.h"

#include "timpi/parallel_sync.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <sstream>
#include <unordered_map>
#include <vector>

registerMooseObject("NavierStokesApp", ConservativeSharpInterfaceVOFMULESCorrector);

InputParameters
ConservativeSharpInterfaceVOFMULESCorrector::validParams()
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
  MooseEnum high_order_correction_scheme("venkatakrishnan vanLeer", "vanLeer");
  params.addParam<MooseEnum>(
      "high_order_correction_scheme",
      high_order_correction_scheme,
      "Higher-order correction flux added on top of the donor/upwind base flux before MULES-style "
      "limiting.");
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
  params.addRangeCheckedParam<Real>(
      "correction_relaxation",
      1.0,
      "correction_relaxation>0 & correction_relaxation<=1",
      "Under-relaxation applied to each bounded explicit correction update.");
  params.addRangeCheckedParam<Real>(
      "later_correction_relaxation",
      0.5,
      "later_correction_relaxation>0 & later_correction_relaxation<=1",
      "Additional damping applied to correction sweeps after the first one, mirroring "
      "interFoam's later-alpha-corrector relaxation more closely.");
  params.addParam<bool>(
      "alpha_apply_prev_corr",
      true,
      "Whether to reuse the previous limited correction flux as the initial correction guess on "
      "the next alpha solve, mirroring interFoam's alphaApplyPrevCorr lifecycle.");
  params.addParam<bool>(
      "use_cell_summed_mules_limiter",
      true,
      "Use a classic cell-summed MULES-style limiter based on per-cell positive/negative "
      "correction totals instead of sequential face-budget depletion.");
  params.addParam<bool>(
      "use_local_mules_bounds",
      true,
      "When true, limit each correction against local neighbor extrema. When false, limit against "
      "the admissible global volume-fraction bounds.");
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
  params.addParam<std::vector<unsigned int>>(
      "debug_face_ids",
      {},
      "Optional list of face ids to include in the alpha debug dump. If empty, the usual "
      "interface-face filter is used.");
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
  params.addParam<MooseFunctorName>(
      "rho_phi_mass_flux_density_functor_name",
      "rho_phi_mass_flux_density",
      "Published density-weighted face mass-flux density accumulated over alpha subcycles.");

  ExecFlagEnum & exec_enum = params.set<ExecFlagEnum>("execute_on", true);
  exec_enum.addAvailableFlags(EXEC_NONE);
  exec_enum = {EXEC_NONE};
  params.suppressParameter<ExecFlagEnum>("execute_on");
  return params;
}

ConservativeSharpInterfaceVOFMULESCorrector::ConservativeSharpInterfaceVOFMULESCorrector(const InputParameters & params)
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
    _later_correction_relaxation(getParam<Real>("later_correction_relaxation")),
    _min_value(getParam<Real>("min_value")),
    _max_value(getParam<Real>("max_value")),
    _alpha_apply_prev_corr(getParam<bool>("alpha_apply_prev_corr")),
    _use_cell_summed_mules_limiter(getParam<bool>("use_cell_summed_mules_limiter")),
    _use_local_mules_bounds(getParam<bool>("use_local_mules_bounds")),
    _debug_dump_subcycle(getParam<bool>("debug_dump_subcycle")),
    _debug_only_first_subcycle(getParam<bool>("debug_only_first_subcycle")),
    _debug_dump_max_faces(getParam<unsigned int>("debug_dump_max_faces")),
    _debug_interface_alpha_tolerance(getParam<Real>("debug_interface_alpha_tolerance")),
    _debug_face_ids([&params]()
                    {
                      std::unordered_set<dof_id_type> ids;
                      for (const auto id : params.get<std::vector<unsigned int>>("debug_face_ids"))
                        ids.insert(static_cast<dof_id_type>(id));
                      return ids;
                    }()),
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
    _rho_phi(_fe_problem.mesh(), blockIDs(), getParam<MooseFunctorName>("rho_phi_functor_name")),
    _rho_phi_mass_flux_density(
        _fe_problem.mesh(), blockIDs(), getParam<MooseFunctorName>("rho_phi_mass_flux_density_functor_name"))
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
    UserObject::_subproblem.addFunctor(getParam<MooseFunctorName>("rho_phi_mass_flux_density_functor_name"),
                                       _rho_phi_mass_flux_density,
                                       tid);
  }
}

void
ConservativeSharpInterfaceVOFMULESCorrector::initialSetup()
{
  cacheSystemData();
  initializeFluxStorage();
  if (_debug_dump_subcycle)
  {
    std::cerr << "[" << name() << "] debug_dump_subcycle=1 debug_only_first_subcycle="
              << (_debug_only_first_subcycle ? 1 : 0)
              << " debug_dump_max_faces=" << _debug_dump_max_faces << std::endl;
    _console << name() << " debug_dump_subcycle=1 debug_only_first_subcycle="
             << (_debug_only_first_subcycle ? 1 : 0)
             << " debug_dump_max_faces=" << _debug_dump_max_faces << std::endl;
  }
}

void
ConservativeSharpInterfaceVOFMULESCorrector::meshChanged()
{
  cacheSystemData();
  initializeFluxStorage();
  invalidatePreviousCorrectionFluxes();
}

void
ConservativeSharpInterfaceVOFMULESCorrector::cacheSystemData()
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
ConservativeSharpInterfaceVOFMULESCorrector::initializeFluxStorage()
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
    _rho_phi_mass_flux_density[fi->id()] = 0.0;
    _alpha_phi_corr_prev.try_emplace(fi->id(), 0.0);
  }
}

void
ConservativeSharpInterfaceVOFMULESCorrector::invalidatePreviousCorrectionFluxes()
{
  for (auto & pair : _alpha_phi_corr_prev)
    pair.second = 0.0;

  _previous_correction_flux_valid = false;
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
ConservativeSharpInterfaceVOFMULESCorrector::solutionAlpha(const NumericVector<Number> & solution,
                                               const ElemInfo & elem_info) const
{
  const auto dof = elem_info.dofIndices()[_sys_num][_var_num];
  if (dof == DofObject::invalid_id)
    return 0.0;

  return static_cast<Real>(solution(dof));
}

Real
ConservativeSharpInterfaceVOFMULESCorrector::integrateLiquidVolume(const NumericVector<Number> & solution) const
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
ConservativeSharpInterfaceVOFMULESCorrector::cellAlpha(const ElemInfo & elem_info) const
{
  return _alpha_var->getElemValue(elem_info, Moose::currentState());
}

Real
ConservativeSharpInterfaceVOFMULESCorrector::boundedAlpha(const Real value) const
{
  return std::min(_max_value, std::max(_min_value, value));
}

ConservativeSharpInterfaceVOFMULESCorrector::RhoPhiConsistencyAudit
ConservativeSharpInterfaceVOFMULESCorrector::rhoPhiConsistencyAudit() const
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
    const Real volumetric_phi = _mass_flux_provider.getVOFTransportVolumetricFaceFlux(*fi);
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

Real
ConservativeSharpInterfaceVOFMULESCorrector::alphaPhiWorkingBeforeIntegrated(const FaceInfo & fi) const
{
  const auto it = _alpha_phi_working_before_debug.find(fi.id());
  return it != _alpha_phi_working_before_debug.end() ? it->second : 0.0;
}

Real
ConservativeSharpInterfaceVOFMULESCorrector::alphaPhiTargetIntegrated(const FaceInfo & fi) const
{
  const auto it = _alpha_phi_target_debug.find(fi.id());
  return it != _alpha_phi_target_debug.end() ? it->second : 0.0;
}

Real
ConservativeSharpInterfaceVOFMULESCorrector::alphaPhiRawCorrectionIntegrated(const FaceInfo & fi) const
{
  return libmesh_map_find(_alpha_phi_corr_raw, fi.id());
}

Real
ConservativeSharpInterfaceVOFMULESCorrector::alphaPhiLimitedDeltaIntegrated(const FaceInfo & fi) const
{
  const auto it = _alpha_phi_limited_delta_debug.find(fi.id());
  return it != _alpha_phi_limited_delta_debug.end() ? it->second : 0.0;
}

Real
ConservativeSharpInterfaceVOFMULESCorrector::alphaPhiAcceptedLambda(const FaceInfo & fi) const
{
  const auto it = _alpha_phi_lambda_debug.find(fi.id());
  return it != _alpha_phi_lambda_debug.end() ? it->second : 0.0;
}

void
ConservativeSharpInterfaceVOFMULESCorrector::resetSubcycleFluxes()
{
  _subcycle_counter = 0;
  // Mirror reference solver's createAlphaFluxes.H lifecycle: clear the working
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
  for (auto & pair : _rho_phi_mass_flux_density)
    pair.second = 0.0;
  _alpha_phi_working_before_debug.clear();
  _alpha_phi_target_debug.clear();
  _alpha_phi_limited_delta_debug.clear();
  _alpha_phi_lambda_debug.clear();
}

void
ConservativeSharpInterfaceVOFMULESCorrector::invalidateOuterCorrectionFluxSeed()
{
  invalidatePreviousCorrectionFluxes();
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
    const Real integrated_rho_phi = rhoPhi(*fi, limited_alpha_flux);
    const Real face_measure = faceMeasure(*fi);

    _rho_phi[face_id] = integrated_rho_phi;
    _rho_phi_mass_flux_density[face_id] =
        face_measure > 0.0 ? integrated_rho_phi / face_measure : 0.0;
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
ConservativeSharpInterfaceVOFMULESCorrector::boundaryValue(const FaceInfo & fi,
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
ConservativeSharpInterfaceVOFMULESCorrector::functorFaceArg(
    const Moose::Functor<Real> & functor, const FaceInfo & fi) const
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

  const Real volumetric_flux = _mass_flux_provider.getVOFTransportVolumetricFaceFlux(fi);
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
                      : boundedAlpha(
                            inlet_outlet_bc->computeBoundaryValue(/* backflow = */ true));
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

  const Real volumetric_flux = _mass_flux_provider.getVOFTransportVolumetricFaceFlux(fi);
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
        high_order_alpha = sharedVanLeerFaceValue(fi, upwind_is_elem);
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
  else if (auto * inlet_outlet_bc = dynamic_cast<LinearFVInletOutletScalarBC *>(bc))
  {
    inlet_outlet_bc->setupFaceData(&fi, face_type);
    high_order_alpha =
        boundedAlpha(inlet_outlet_bc->computeBoundaryValue(/* backflow = */ !upwind_is_elem));
  }
  else if (upwind_is_elem &&
           dynamic_cast<LinearFVAdvectionDiffusionExtrapolatedBC *>(bc))
    high_order_alpha = boundaryValue(fi, face_type);
  else
    return std::abs(volumetric_flux) > libMesh::TOLERANCE
               ? donorFlux(fi) / (volumetric_flux * faceMeasure(fi))
               : cellAlpha(*fi.elemInfo());

  return high_order_alpha;
}

Real
ConservativeSharpInterfaceVOFMULESCorrector::highOrderFlux(const FaceInfo & fi) const
{
  const auto face_type = fi.faceType(std::make_pair(_var_num, _sys_num));
  if (face_type == FaceInfo::VarFaceNeighbors::NEITHER)
    return 0.0;

  const Real volumetric_flux = _mass_flux_provider.getVOFTransportVolumetricFaceFlux(fi);
  const Real high_order_alpha = highOrderFaceValue(fi);
  return volumetric_flux * high_order_alpha * faceMeasure(fi);
}

Real
ConservativeSharpInterfaceVOFMULESCorrector::venkatakrishnanFaceValue(const FaceInfo & fi,
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
  return std::min(_max_value, std::max(_min_value, phi_face));
}

ConservativeSharpInterfaceVOFMULESCorrector::BoundaryFaceKind
ConservativeSharpInterfaceVOFMULESCorrector::classifyBoundaryFace(const FaceInfo & fi,
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

const char *
ConservativeSharpInterfaceVOFMULESCorrector::boundaryFaceKindName(const BoundaryFaceKind kind) const
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
ConservativeSharpInterfaceVOFMULESCorrector::compressionFlux(const FaceInfo & fi,
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
  const Real bounded_elem_alpha = boundedAlpha(elem_alpha);
  const Real bounded_neighbor_alpha = boundedAlpha(neighbor_alpha);
  const Real alpha_face =
      boundedAlpha(fi.gC() * bounded_elem_alpha + (1.0 - fi.gC()) * bounded_neighbor_alpha);
  const Real compressive_speed =
      std::abs(_mass_flux_provider.getVOFTransportVolumetricFaceFlux(fi)) *
      MetaPhysicL::raw_value(_compression_factor(face_arg, state)) *
      (interface_normal * face_unit_normal);

  return compressive_speed * alpha_face * (1.0 - alpha_face) * faceMeasure(fi);
}

Real
ConservativeSharpInterfaceVOFMULESCorrector::compressionAlignment(const FaceInfo & fi) const
{
  const auto state = Moose::currentState();
  const Moose::FaceArg face_arg{
      &fi, Moose::FV::LimiterType::CentralDifference, true, false, nullptr, nullptr};

  const RealVectorValue face_normal = fi.normal();
  const Real face_normal_mag = face_normal.norm();
  const RealVectorValue face_unit_normal =
      face_normal_mag > 0.0 ? face_normal / face_normal_mag : RealVectorValue();
  const RealVectorValue interface_normal = MetaPhysicL::raw_value(_interface_normal(face_arg, state));
  return interface_normal * face_unit_normal;
}

Real
ConservativeSharpInterfaceVOFMULESCorrector::faceFunctorAverage(const FaceInfo & fi,
                                                    const Moose::Functor<Real> & functor) const
{
  const auto state = Moose::currentState();
  return MetaPhysicL::raw_value(functor(functorFaceArg(functor, fi), state));
}

Real
ConservativeSharpInterfaceVOFMULESCorrector::rhoPhi(const FaceInfo & fi, const Real limited_alpha_flux) const
{
  const Real gas_density = faceFunctorAverage(fi, _gas_density);
  const Real liquid_density = faceFunctorAverage(fi, _liquid_density);
  const Real volumetric_mass_flux =
      _mass_flux_provider.getVOFTransportVolumetricFaceFlux(fi) * faceMeasure(fi) * gas_density;
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

  data.volumetric_flux = _mass_flux_provider.getVOFTransportVolumetricFaceFlux(fi);
  data.elem_alpha = cellAlpha(*fi.elemInfo());
  data.donor_flux = donorFlux(fi);
  data.high_order_flux = highOrderFlux(fi);

  data.boundary_kind = classifyBoundaryFace(fi, face_type, data.volumetric_flux);
  data.boundary_face = data.boundary_kind != BoundaryFaceKind::Internal;

  if (data.boundary_kind == BoundaryFaceKind::Internal)
  {
    data.neighbor_alpha = cellAlpha(*fi.neighborInfo());
    data.interface_normal_alignment = compressionAlignment(fi);
    const Real raw_compressive_flux = compressionFlux(fi, data.elem_alpha, data.neighbor_alpha);
    Real compressed_high_order_flux = data.high_order_flux;
    const Real phi_face_area = data.volumetric_flux * faceMeasure(fi);
    if (std::abs(phi_face_area) > libMesh::TOLERANCE)
    {
      const Real compressed_alpha =
          boundedAlpha(highOrderFaceValue(fi) + raw_compressive_flux / phi_face_area);
      compressed_high_order_flux = phi_face_area * compressed_alpha;
    }
    data.compressive_flux = compressed_high_order_flux - data.high_order_flux;
    data.advective_correction_flux = data.high_order_flux - data.donor_flux;
    data.correction_flux = compressed_high_order_flux - data.donor_flux;
    return data;
  }

  data.neighbor_alpha = boundaryValue(fi, face_type);

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
ConservativeSharpInterfaceVOFMULESCorrector::publishFaceFluxes(
    const std::vector<FaceCorrectionData> & face_corrections,
    const std::vector<Real> & raw_correction_fluxes,
    const std::vector<Real> & accumulated_alpha_fluxes,
    const std::vector<Real> & accumulated_correction_fluxes,
    const Real subcycle_fraction)
{
  for (const auto i : index_range(face_corrections))
  {
    const auto & data = face_corrections[i];
    const auto face_id = data.face->id();
    const Real face_measure = faceMeasure(*data.face);
    const Real limited_correction = accumulated_correction_fluxes[i];
    const Real limited_alpha_flux = accumulated_alpha_fluxes[i];
    const Real rho_phi = rhoPhi(*data.face, limited_alpha_flux);

    // Accumulate the published face fluxes with the same subcycle weighting as rhoPhi so
    // downstream consumers see a timestep-consistent alphaPhi/rhoPhi pair after subcycling.
    _alpha_phi_bd[face_id] += subcycle_fraction * data.donor_flux;
    _alpha_phi_ho[face_id] += subcycle_fraction * data.high_order_flux;
    _alpha_phi_comp[face_id] += subcycle_fraction * data.compressive_flux;
    _alpha_phi_corr_raw[face_id] += subcycle_fraction * raw_correction_fluxes[i];
    _alpha_phi_corr[face_id] += subcycle_fraction * limited_correction;
    _alpha_phi_limited[face_id] += subcycle_fraction * limited_alpha_flux;
    _rho_phi[face_id] += subcycle_fraction * rho_phi;
    _rho_phi_mass_flux_density[face_id] +=
        subcycle_fraction * (face_measure > 0.0 ? rho_phi / face_measure : 0.0);
  }
}

bool
ConservativeSharpInterfaceVOFMULESCorrector::partitionFace(const FaceCorrectionData & data) const
{
  return data.has_neighbor &&
         data.face->elemInfo()->elem()->processor_id() !=
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
  auto receive_limiters =
      [&received_minimum_lambda_by_face](const processor_id_type,
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
      changed_lambda =
          changed_lambda || updated_lambda + libMesh::TOLERANCE < accepted_lambda[i];
      accepted_lambda[i] = updated_lambda;
    }

  return changed_lambda;
}

void
ConservativeSharpInterfaceVOFMULESCorrector::applyCorrection(
    const Real dt,
    const Real subcycle_fraction,
    ConservativeSharpInterfaceCurvatureCalculator * const curvature)
{
  ++_subcycle_counter;
  if (!_system || !_alpha_var || dt <= 0.0)
    return;

  if (_debug_dump_subcycle && (!_debug_only_first_subcycle || _subcycle_counter == 1))
  {
    std::cerr << "[" << name() << "] entering applyCorrection subcycle=" << _subcycle_counter
              << " dt=" << dt << " subcycle_fraction=" << subcycle_fraction << std::endl;
    _console << name() << " entering applyCorrection subcycle=" << _subcycle_counter
             << " dt=" << dt << " subcycle_fraction=" << subcycle_fraction << std::endl;
  }

  std::vector<FaceCorrectionData> published_face_corrections;
  std::vector<Real> published_raw_correction_fluxes;
  std::vector<Real> published_accumulated_alpha_fluxes;
  std::vector<Real> published_accumulated_correction_fluxes;
  std::vector<Real> published_working_alpha_fluxes_before;
  std::vector<Real> published_target_alpha_fluxes;
  std::vector<Real> published_limited_correction_fluxes;
  std::vector<Real> published_accepted_lambda;
  std::unordered_map<dof_id_type, Real> alpha_before_correction;
  std::unordered_map<dof_id_type, Real> alpha_after_correction;
  const bool use_previous_correction_flux = _alpha_apply_prev_corr && _previous_correction_flux_valid;
  std::unordered_map<dof_id_type, Real> working_alpha_flux;

  if (_num_alpha_corrections == 0)
  {
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
      if (data.face)
        face_corrections.push_back(data);
    }

    if (face_corrections.empty())
      return;

    std::vector<Real> zero_flux(face_corrections.size(), 0.0);
    std::vector<Real> donor_flux(face_corrections.size(), 0.0);
    for (const auto i : index_range(face_corrections))
    {
      donor_flux[i] = face_corrections[i].donor_flux;
      _alpha_phi_working_before_debug[face_corrections[i].face->id()] = donor_flux[i];
      _alpha_phi_target_debug[face_corrections[i].face->id()] = donor_flux[i];
      _alpha_phi_limited_delta_debug[face_corrections[i].face->id()] = 0.0;
      _alpha_phi_lambda_debug[face_corrections[i].face->id()] = 0.0;
    }

    publishFaceFluxes(face_corrections, zero_flux, donor_flux, zero_flux, subcycle_fraction);
    invalidatePreviousCorrectionFluxes();
    _system->computeGradients();
    return;
  }

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
    std::vector<Real> accumulated_alpha_flux(face_corrections.size(), 0.0);
    std::vector<Real> accumulated_correction_flux(face_corrections.size(), 0.0);
    std::vector<Real> working_alpha_flux_before(face_corrections.size(), 0.0);
    std::vector<Real> target_alpha_flux(face_corrections.size(), 0.0);

    for (const auto i : index_range(face_corrections))
    {
      const auto & data = face_corrections[i];
      const auto face_id = data.face->id();
      if (!working_alpha_flux.count(face_id))
      {
        const Real previous_correction_seed =
            use_previous_correction_flux ? _alpha_phi_corr_prev[face_id] : 0.0;
        working_alpha_flux.emplace(face_id, data.donor_flux + previous_correction_seed);
      }

      working_alpha_flux_before[i] = libmesh_map_find(working_alpha_flux, face_id);
      target_alpha_flux[i] = data.donor_flux + data.correction_flux;
      raw_correction_flux[i] = target_alpha_flux[i] - working_alpha_flux_before[i];
      accepted_lambda[i] = std::abs(raw_correction_flux[i]) > libMesh::TOLERANCE ? 1.0 : 0.0;
    }

    if (_debug_dump_subcycle && correction_it == 0 &&
        (!_debug_only_first_subcycle || _subcycle_counter == 1))
      dumpCandidateFaceDebug(
          face_corrections, raw_correction_flux, accepted_lambda, _subcycle_counter);

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

    std::unordered_map<dof_id_type, Real> local_upper_bound;
    std::unordered_map<dof_id_type, Real> local_lower_bound;

    const auto initialize_local_bounds = [&](const dof_id_type dof)
    {
      if (local_upper_bound.count(dof))
        return;

      local_upper_bound.emplace(dof, _min_value);
      local_lower_bound.emplace(dof, _max_value);
    };

    const auto widen_local_bounds = [&](const dof_id_type dof, const Real alpha)
    {
      initialize_local_bounds(dof);
      const Real bounded_alpha = boundedAlpha(alpha);
      local_upper_bound[dof] = std::max(local_upper_bound[dof], bounded_alpha);
      local_lower_bound[dof] = std::min(local_lower_bound[dof], bounded_alpha);
    };

    if (_use_local_mules_bounds)
    {
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
        pair.second = std::min(pair.second, _max_value);
      for (auto & pair : local_lower_bound)
        pair.second = std::max(pair.second, _min_value);
    }
    else
      for (const auto & data : face_corrections)
      {
        local_upper_bound[data.elem_dof] = _max_value;
        local_lower_bound[data.elem_dof] = _min_value;
        if (data.has_neighbor)
        {
          local_upper_bound[data.neighbor_dof] = _max_value;
          local_lower_bound[data.neighbor_dof] = _min_value;
        }
      }

    std::unordered_map<dof_id_type, Real> cell_volume_by_dof;
    for (const auto & data : face_corrections)
    {
      cell_volume_by_dof.emplace(data.elem_dof, cellVolume(*data.face->elemInfo()));
      if (data.has_neighbor)
        cell_volume_by_dof.emplace(data.neighbor_dof, cellVolume(*data.face->neighborInfo()));
    }

    if (_use_cell_summed_mules_limiter)
    {
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
        psi_maxn.emplace(
            dof, cell_volume * std::max(0.0, local_upper_bound[dof] - alpha) / dt);
        psi_minn.emplace(
            dof, cell_volume * std::max(0.0, alpha - local_lower_bound[dof]) / dt);
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

      const auto clamp_limiter = [](const Real value)
      {
        return std::min(1.0, std::max(0.0, value));
      };

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
          changed_lambda =
              changed_lambda || updated_lambda + libMesh::TOLERANCE < accepted_lambda[i];
          accepted_lambda[i] = updated_lambda;
        }

        changed_lambda =
            synchronizePartitionFaceLimiters(face_corrections, accepted_lambda) || changed_lambda;

        unsigned int changed_anywhere = changed_lambda ? 1 : 0;
        _communicator.max(changed_anywhere);
        if (!changed_anywhere)
          break;
      }
    }
    else
      for (const auto limiter_it : make_range(_num_limiter_iterations))
      {
        (void)limiter_it;
        std::unordered_map<dof_id_type, Real> remaining_increase_budget;
        std::unordered_map<dof_id_type, Real> remaining_decrease_budget;

        const auto initialize_cell = [&](const dof_id_type dof)
        {
          if (remaining_increase_budget.count(dof))
            return;

          const Real alpha = current_local_solution(dof);
          remaining_increase_budget.emplace(dof, std::max(0.0, local_upper_bound[dof] - alpha));
          remaining_decrease_budget.emplace(dof, std::max(0.0, alpha - local_lower_bound[dof]));
        };

        for (const auto & data : face_corrections)
        {
          initialize_cell(data.elem_dof);
          if (data.has_neighbor)
            initialize_cell(data.neighbor_dof);
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
          changed_lambda =
              changed_lambda || updated_lambda + libMesh::TOLERANCE < accepted_lambda[i];
          accepted_lambda[i] = updated_lambda;

          if (updated_lambda <= 0.0)
            continue;

          const Real limited_flux = updated_lambda * raw_flux;
          const Real limited_elem_delta =
              std::abs(-dt * limited_flux / cellVolume(*data.face->elemInfo()));
          if (limited_elem_delta > libMesh::TOLERANCE)
          {
            auto & elem_budget =
                elem_delta >= 0.0 ? remaining_increase_budget[data.elem_dof]
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
            correction_it == 0 ? 1.0 : _later_correction_relaxation;
        limited_correction_flux[i] =
            correction_weight * _correction_relaxation * accepted_lambda[i] * raw_correction_flux[i];
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

    if (curvature)
      curvature->updateCurvatureMaps(_debug_dump_subcycle);

    for (const auto i : index_range(face_corrections))
    {
      const auto & data = face_corrections[i];
      const auto face_id = data.face->id();
      working_alpha_flux[face_id] += limited_correction_flux[i];
      accumulated_alpha_flux[i] = working_alpha_flux[face_id];
      accumulated_correction_flux[i] = working_alpha_flux[face_id] - data.donor_flux;
    }

    published_face_corrections = face_corrections;
    published_raw_correction_fluxes = raw_correction_flux;
    published_accumulated_alpha_fluxes = accumulated_alpha_flux;
    published_accumulated_correction_fluxes = accumulated_correction_flux;
    published_working_alpha_fluxes_before = working_alpha_flux_before;
    published_target_alpha_fluxes = target_alpha_flux;
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
                      published_accumulated_alpha_fluxes,
                      published_accumulated_correction_fluxes,
                      subcycle_fraction);

    if (_alpha_apply_prev_corr)
      for (auto & pair : _alpha_phi_corr_prev)
        pair.second = 0.0;
    else
      invalidatePreviousCorrectionFluxes();

    for (const auto i : index_range(published_face_corrections))
    {
      if (_alpha_apply_prev_corr)
        _alpha_phi_corr_prev[published_face_corrections[i].face->id()] =
            published_accumulated_correction_fluxes[i];
      _alpha_phi_working_before_debug[published_face_corrections[i].face->id()] =
          published_working_alpha_fluxes_before[i];
      _alpha_phi_target_debug[published_face_corrections[i].face->id()] =
          published_target_alpha_fluxes[i];
      _alpha_phi_limited_delta_debug[published_face_corrections[i].face->id()] =
          published_limited_correction_fluxes[i];
      _alpha_phi_lambda_debug[published_face_corrections[i].face->id()] =
          published_accepted_lambda[i];
    }
    _previous_correction_flux_valid = _alpha_apply_prev_corr;

    if (_debug_dump_subcycle && (!_debug_only_first_subcycle || _subcycle_counter == 1))
      dumpFaceDebug(published_face_corrections,
                    published_raw_correction_fluxes,
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
ConservativeSharpInterfaceVOFMULESCorrector::dumpCandidateFaceDebug(
    const std::vector<FaceCorrectionData> & face_corrections,
    const std::vector<Real> & raw_correction_fluxes,
    const std::vector<Real> & accepted_lambda,
    const unsigned int subcycle_index) const
{
  unsigned int dumped = 0;
  for (const auto i : index_range(face_corrections))
  {
    if (dumped >= _debug_dump_max_faces)
      break;

    const auto & data = face_corrections[i];
    if (!shouldDebugFace(data))
      continue;
    if (std::abs(data.elem_alpha - data.neighbor_alpha) < _debug_interface_alpha_tolerance)
      continue;

    const auto * const sharp_mass_flux =
        dynamic_cast<const ConservativeSharpInterfaceRhieChowMassFlux *>(&_mass_flux_provider);
    const Real rc_corrected_phi =
        sharp_mass_flux ? sharp_mass_flux->storedCorrectedFacePhi(*data.face) : 0.0;
    const Real vof_transport_phi =
        sharp_mass_flux ? sharp_mass_flux->storedVOFTransportPhi(*data.face) : 0.0;
    const Real rc_outer_phi =
        sharp_mass_flux ? sharp_mass_flux->storedOuterIterationPhi(*data.face) : 0.0;
    const Real rc_predictor_phi =
        sharp_mass_flux ? sharp_mass_flux->storedPredictorConvectivePhi(*data.face) : 0.0;

    std::ostringstream line;
    line << name() << " stage=prelimit"
         << " subcycle=" << subcycle_index << " face=" << data.face->id()
         << " elem_dof=" << data.elem_dof;
    if (data.has_neighbor)
      line << " neighbor_dof=" << data.neighbor_dof;
    else
      line << " boundary_face=1 boundary_kind=" << boundaryFaceKindName(data.boundary_kind);
    line << " volumetric_flux=" << data.volumetric_flux
         << " elem_alpha=" << data.elem_alpha << " neighbor_alpha=" << data.neighbor_alpha
         << " interface_normal_alignment=" << data.interface_normal_alignment
         << " rc_corrected_phi=" << rc_corrected_phi
         << " vof_transport_phi=" << vof_transport_phi
         << " rc_outer_phi=" << rc_outer_phi
         << " rc_predictor_phi=" << rc_predictor_phi
         << " donor_flux=" << data.donor_flux << " high_order_flux=" << data.high_order_flux
         << " compressive_flux=" << data.compressive_flux
         << " advective_correction_flux=" << data.advective_correction_flux
         << " correction_flux=" << data.correction_flux
         << " raw_correction_flux="
         << (i < raw_correction_fluxes.size() ? raw_correction_fluxes[i] : 0.0)
         << " accepted_lambda=" << (i < accepted_lambda.size() ? accepted_lambda[i] : 0.0);
    std::cerr << "[" << line.str() << "]" << std::endl;

    _console << name() << " stage=prelimit"
             << " subcycle=" << subcycle_index << " face=" << data.face->id()
             << " elem_dof=" << data.elem_dof;
    if (data.has_neighbor)
      _console << " neighbor_dof=" << data.neighbor_dof;
    else
      _console << " boundary_face=1 boundary_kind=" << boundaryFaceKindName(data.boundary_kind);
    _console << " volumetric_flux=" << data.volumetric_flux
             << " elem_alpha=" << data.elem_alpha << " neighbor_alpha=" << data.neighbor_alpha
             << " interface_normal_alignment=" << data.interface_normal_alignment
             << " rc_corrected_phi=" << rc_corrected_phi
             << " vof_transport_phi=" << vof_transport_phi
             << " rc_outer_phi=" << rc_outer_phi
             << " rc_predictor_phi=" << rc_predictor_phi
             << " donor_flux=" << data.donor_flux << " high_order_flux=" << data.high_order_flux
             << " compressive_flux=" << data.compressive_flux
             << " advective_correction_flux=" << data.advective_correction_flux
             << " correction_flux=" << data.correction_flux
             << " raw_correction_flux=" << (i < raw_correction_fluxes.size() ? raw_correction_fluxes[i] : 0.0)
             << " accepted_lambda=" << (i < accepted_lambda.size() ? accepted_lambda[i] : 0.0)
             << std::endl;
    ++dumped;
  }
}

void
ConservativeSharpInterfaceVOFMULESCorrector::dumpFaceDebug(
    const std::vector<FaceCorrectionData> & face_corrections,
    const std::vector<Real> & raw_correction_fluxes,
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
    if (!shouldDebugFace(data))
      continue;
    const auto * const sharp_mass_flux =
        dynamic_cast<const ConservativeSharpInterfaceRhieChowMassFlux *>(&_mass_flux_provider);
    const Real rc_corrected_phi =
        sharp_mass_flux ? sharp_mass_flux->storedCorrectedFacePhi(*data.face) : 0.0;
    const Real vof_transport_phi =
        sharp_mass_flux ? sharp_mass_flux->storedVOFTransportPhi(*data.face) : 0.0;
    const Real rc_outer_phi =
        sharp_mass_flux ? sharp_mass_flux->storedOuterIterationPhi(*data.face) : 0.0;
    const Real rc_predictor_phi =
        sharp_mass_flux ? sharp_mass_flux->storedPredictorConvectivePhi(*data.face) : 0.0;
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

    std::ostringstream line;
    line << name() << " subcycle=" << subcycle_index << " face=" << data.face->id()
         << " elem_dof=" << data.elem_dof;
    if (data.has_neighbor)
      line << " neighbor_dof=" << data.neighbor_dof;
    else
      line << " boundary_face=1 boundary_kind=" << boundaryFaceKindName(data.boundary_kind);
    line << " volumetric_flux=" << data.volumetric_flux
         << " interface_normal_alignment=" << data.interface_normal_alignment
         << " rc_corrected_phi=" << rc_corrected_phi
         << " vof_transport_phi=" << vof_transport_phi
         << " rc_outer_phi=" << rc_outer_phi
         << " rc_predictor_phi=" << rc_predictor_phi
         << " donor_flux=" << data.donor_flux << " high_order_flux=" << data.high_order_flux
         << " compressive_flux=" << data.compressive_flux
         << " correction_flux=" << data.correction_flux
         << " raw_correction_flux="
         << (i < raw_correction_fluxes.size() ? raw_correction_fluxes[i] : 0.0)
         << " accepted_lambda=" << (i < accepted_lambda.size() ? accepted_lambda[i] : 0.0)
         << " elem_alpha_before=" << elem_before << " elem_alpha_after=" << elem_after
         << " neighbor_alpha_before=" << neighbor_before
         << " neighbor_alpha_after=" << neighbor_after;
    std::cerr << "[" << line.str() << "]" << std::endl;

    _console << name() << " subcycle=" << subcycle_index << " face=" << data.face->id()
             << " elem_dof=" << data.elem_dof;
    if (data.has_neighbor)
      _console << " neighbor_dof=" << data.neighbor_dof;
    else
      _console << " boundary_face=1 boundary_kind=" << boundaryFaceKindName(data.boundary_kind);
    _console << " volumetric_flux=" << data.volumetric_flux
             << " interface_normal_alignment=" << data.interface_normal_alignment
             << " rc_corrected_phi=" << rc_corrected_phi
             << " vof_transport_phi=" << vof_transport_phi
             << " rc_outer_phi=" << rc_outer_phi
             << " rc_predictor_phi=" << rc_predictor_phi
             << " donor_flux=" << data.donor_flux << " high_order_flux=" << data.high_order_flux
             << " compressive_flux=" << data.compressive_flux
             << " correction_flux=" << data.correction_flux
             << " raw_correction_flux="
             << (i < raw_correction_fluxes.size() ? raw_correction_fluxes[i] : 0.0)
             << " accepted_lambda=" << (i < accepted_lambda.size() ? accepted_lambda[i] : 0.0)
             << " elem_alpha_before=" << elem_before << " elem_alpha_after=" << elem_after
             << " neighbor_alpha_before=" << neighbor_before
             << " neighbor_alpha_after=" << neighbor_after << std::endl;
    ++dumped;
  }
}

bool
ConservativeSharpInterfaceVOFMULESCorrector::shouldDebugFace(const FaceCorrectionData & data) const
{
  return _debug_face_ids.empty() || _debug_face_ids.count(data.face->id());
}
