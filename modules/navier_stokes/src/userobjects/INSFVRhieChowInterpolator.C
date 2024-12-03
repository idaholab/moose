//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVRhieChowInterpolator.h"
#include "INSFVAttributes.h"
#include "GatherRCDataElementThread.h"
#include "GatherRCDataFaceThread.h"
#include "MooseMesh.h"
#include "SystemBase.h"
#include "NS.h"
#include "Assembly.h"
#include "INSFVVelocityVariable.h"
#include "PiecewiseByBlockLambdaFunctor.h"
#include "VectorCompositeFunctor.h"
#include "FVElementalKernel.h"
#include "NSFVUtils.h"
#include "DisplacedProblem.h"

#include "libmesh/mesh_base.h"
#include "libmesh/elem_range.h"
#include "libmesh/parallel_algebra.h"
#include "libmesh/remote_elem.h"
#include "metaphysicl/dualsemidynamicsparsenumberarray.h"
#include "metaphysicl/parallel_dualnumber.h"
#include "metaphysicl/parallel_dynamic_std_array_wrapper.h"
#include "metaphysicl/parallel_semidynamicsparsenumberarray.h"
#include "timpi/parallel_sync.h"

using namespace libMesh;

registerMooseObject("NavierStokesApp", INSFVRhieChowInterpolator);

InputParameters
INSFVRhieChowInterpolator::uniqueParams()
{
  auto params = emptyInputParameters();
  params.addParam<bool>(
      "pull_all_nonlocal_a",
      false,
      "Whether to pull all nonlocal 'a' coefficient data to our process. Note that 'nonlocal' "
      "means elements that we have access to (this may not be all the elements in the mesh if the "
      "mesh is distributed) but that we do not own.");
  params.addParamNamesToGroup("pull_all_nonlocal_a", "Parallel Execution Tuning");

  params.addParam<bool>(
      "correct_volumetric_force", false, "Flag to activate volume force corrections.");
  MooseEnum volume_force_correction_method("force-consistent pressure-consistent",
                                           "force-consistent");
  params.addParam<MooseEnum>(
      "volume_force_correction_method",
      volume_force_correction_method,
      "The method used for correcting the Rhie-Chow coefficients for a volume force.");
  params.addParam<std::vector<MooseFunctorName>>(
      "volumetric_force_functors", "The names of the functors with the volumetric force sources.");
  return params;
}

std::vector<std::string>
INSFVRhieChowInterpolator::listOfCommonParams()
{
  return {"pull_all_nonlocal_a",
          "correct_volumetric_force",
          "volume_force_correction_method",
          "volumetric_force_functors"};
}

InputParameters
INSFVRhieChowInterpolator::validParams()
{
  auto params = RhieChowInterpolatorBase::validParams();
  params += INSFVRhieChowInterpolator::uniqueParams();

  params.addClassDescription(
      "Computes the Rhie-Chow velocity based on gathered 'a' coefficient data.");

  ExecFlagEnum & exec_enum = params.set<ExecFlagEnum>("execute_on", true);
  exec_enum.addAvailableFlags(EXEC_PRE_KERNELS);
  exec_enum = {EXEC_PRE_KERNELS};
  params.suppressParameter<ExecFlagEnum>("execute_on");

  params.addParam<MooseFunctorName>(
      "a_u",
      "For simulations in which the advecting velocities are aux variables, this parameter must be "
      "supplied. It represents the on-diagonal coefficients for the 'x' component velocity, solved "
      "via the Navier-Stokes equations.");
  params.addParam<MooseFunctorName>(
      "a_v",
      "For simulations in which the advecting velocities are aux variables, this parameter must be "
      "supplied when the mesh dimension is greater than 1. It represents the on-diagonal "
      "coefficients for the 'y' component velocity, solved via the Navier-Stokes equations.");
  params.addParam<MooseFunctorName>(
      "a_w",
      "For simulations in which the advecting velocities are aux variables, this parameter must be "
      "supplied when the mesh dimension is greater than 2. It represents the on-diagonal "
      "coefficients for the 'z' component velocity, solved via the Navier-Stokes equations.");
  params.addParam<VariableName>("disp_x", "The x-component of displacement");
  params.addParam<VariableName>("disp_y", "The y-component of displacement");
  params.addParam<VariableName>("disp_z", "The z-component of displacement");
  return params;
}

INSFVRhieChowInterpolator::INSFVRhieChowInterpolator(const InputParameters & params)
  : RhieChowInterpolatorBase(params),
    _vel(libMesh::n_threads()),
    _a(_moose_mesh, blockIDs(), "a", /*extrapolated_boundary*/ true),
    _ax(_a, 0),
    _ay(_a, 1),
    _az(_a, 2),
    _momentum_sys_number(_fe_problem.systemNumForVariable(getParam<VariableName>("u"))),
    _example(0),
    _a_data_provided(false),
    _pull_all_nonlocal(getParam<bool>("pull_all_nonlocal_a")),
    _bool_correct_vf(getParam<bool>("correct_volumetric_force")),
    _volume_force_correction_method(getParam<MooseEnum>("volume_force_correction_method")),
    _volumetric_force_functors(
        isParamValid("volumetric_force_functors")
            ? &getParam<std::vector<MooseFunctorName>>("volumetric_force_functors")
            : nullptr)
{
  auto process_displacement = [this](const auto & disp_name, auto & disp_container)
  {
    if (!_displaced)
      paramError(disp_name,
                 "Displacement provided but we are not running on the displaced mesh. If you "
                 "really want this object to run on the displaced mesh, then set "
                 "'use_displaced_mesh = true', otherwise remove this displacement parameter");
    disp_container.resize(libMesh::n_threads());
    fillContainer(disp_name, disp_container);
    checkBlocks(*disp_container[0]);
  };

  if (isParamValid("disp_x"))
    process_displacement("disp_x", _disp_xs);

  if (_dim >= 2)
  {
    if (isParamValid("disp_y"))
      process_displacement("disp_y", _disp_ys);
    else if (isParamValid("disp_x"))
      paramError("disp_y", "If 'disp_x' is provided, then 'disp_y' must be as well");
  }

  if (_dim >= 3)
  {
    if (isParamValid("disp_z"))
      process_displacement("disp_z", _disp_zs);
    else if (isParamValid("disp_x"))
      paramError("disp_z", "If 'disp_x' is provided, then 'disp_z' must be as well");
  }

  for (const auto tid : make_range(libMesh::n_threads()))
  {
    _vel[tid] = std::make_unique<PiecewiseByBlockLambdaFunctor<ADRealVectorValue>>(
        name() + std::to_string(tid),
        [this, tid](const auto & r, const auto & t) -> ADRealVectorValue
        {
          ADRealVectorValue velocity((*_us[tid])(r, t));
          if (_dim >= 2)
            velocity(1) = (*_vs[tid])(r, t);
          if (_dim >= 3)
            velocity(2) = (*_ws[tid])(r, t);
          return velocity;
        },
        std::set<ExecFlagType>({EXEC_ALWAYS}),
        _moose_mesh,
        blockIDs());

    if (_disp_xs.size())
      _disps.push_back(std::make_unique<Moose::VectorCompositeFunctor<ADReal>>(
          name() + "_disp_" + std::to_string(tid),
          *_disp_xs[tid],
          _dim >= 2 ? static_cast<const Moose::FunctorBase<ADReal> &>(*_disp_ys[tid])
                    : static_cast<const Moose::FunctorBase<ADReal> &>(_zero_functor),
          _dim >= 3 ? static_cast<const Moose::FunctorBase<ADReal> &>(*_disp_zs[tid])
                    : static_cast<const Moose::FunctorBase<ADReal> &>(_zero_functor)));
  }

  if (_velocity_interp_method == Moose::FV::InterpMethod::Average && isParamValid("a_u"))
    paramError("a_u",
               "Rhie Chow coefficients may not be specified for average velocity interpolation");

  if (_velocity_interp_method != Moose::FV::InterpMethod::Average)
    fillARead();

  if (_bool_correct_vf && !_volumetric_force_functors)
    paramError("volumetric_force_functors",
               "At least one volumetric force functor must be specified if "
               "'correct_volumetric_force' is true.");

  // Volume correction related
  if (_bool_correct_vf)
  {
    const unsigned int num_volume_forces = (*_volumetric_force_functors).size();
    _volumetric_force.resize(num_volume_forces);
    for (const auto i : make_range(num_volume_forces))
      _volumetric_force[i] = &getFunctor<Real>((*_volumetric_force_functors)[i]);
  }
}

void
INSFVRhieChowInterpolator::fillARead()
{
  _a_read.resize(libMesh::n_threads());

  if (isParamValid("a_u"))
  {
    if (_dim > 1 && !isParamValid("a_v"))
      mooseError("If a_u is provided, then a_v must be provided");

    if (_dim > 2 && !isParamValid("a_w"))
      mooseError("If a_u is provided, then a_w must be provided");

    _a_data_provided = true;
    _a_aux.resize(libMesh::n_threads());
  }
  else if (isParamValid("a_v"))
    paramError("a_v", "If the a_v coefficients are provided, then a_u must be provided");
  else if (isParamValid("a_w"))
    paramError("a_w", "If the a_w coefficients are provided, then a_u must be provided");

  if (_a_data_provided)
  {
    for (const auto tid : make_range(libMesh::n_threads()))
    {
      const Moose::FunctorBase<ADReal> *v_comp, *w_comp;
      if (_dim > 1)
        v_comp = &UserObject::_subproblem.getFunctor<ADReal>(
            deduceFunctorName("a_v"), tid, name(), true);
      else
        v_comp = &_zero_functor;
      if (_dim > 2)
        w_comp = &UserObject::_subproblem.getFunctor<ADReal>(
            deduceFunctorName("a_w"), tid, name(), true);
      else
        w_comp = &_zero_functor;

      _a_aux[tid] = std::make_unique<Moose::VectorCompositeFunctor<ADReal>>(
          "RC_a_coeffs",
          UserObject::_subproblem.getFunctor<ADReal>(deduceFunctorName("a_u"), tid, name(), true),
          *v_comp,
          *w_comp);
      _a_read[tid] = _a_aux[tid].get();
    }
  }
  else
    for (const auto tid : make_range(libMesh::n_threads()))
    {
      _a_read[tid] = &_a;

      // We are the fluid flow application, so we should make sure users have the ability to
      // write 'a' out to aux variables for possible transfer to other applications
      UserObject::_subproblem.addFunctor("ax", _ax, tid);
      UserObject::_subproblem.addFunctor("ay", _ay, tid);
      UserObject::_subproblem.addFunctor("az", _az, tid);
    }
}

void
INSFVRhieChowInterpolator::initialSetup()
{
  insfvSetup();

  if (_velocity_interp_method == Moose::FV::InterpMethod::Average)
    return;
  for (const auto var_num : _var_numbers)
  {
    std::vector<MooseObject *> var_objects;
    _fe_problem.theWarehouse()
        .query()
        .template condition<AttribVar>(static_cast<int>(var_num))
        .template condition<AttribResidualObject>(true)
        .template condition<AttribSysNum>(_u->sys().number())
        .queryInto(var_objects);
    for (auto * const var_object : var_objects)
    {
      // Allow FVElementalKernel that are not INSFVMomentumResidualObject for now, refs #20695
      if (!dynamic_cast<INSFVMomentumResidualObject *>(var_object) &&
          !dynamic_cast<FVElementalKernel *>(var_object))
        mooseError("Object ",
                   var_object->name(),
                   " is not a INSFVMomentumResidualObject. Make sure that all the objects applied "
                   "to the momentum equation are INSFV or derived objects.");
      else if (!dynamic_cast<INSFVMomentumResidualObject *>(var_object) &&
               dynamic_cast<FVElementalKernel *>(var_object))
        mooseWarning(
            "Elemental kernel ",
            var_object->name(),
            " is not a INSFVMomentumResidualObject. Make sure that all the objects applied "
            "to the momentum equation are INSFV or derived objects.");
    }

    if (var_objects.size() == 0 && !_a_data_provided)
      mooseError("No INSFVKernels detected for the velocity variables. If you are trying to use "
                 "auxiliary variables for advection, please specify the a_u/v/w coefficients. If "
                 "not, please specify INSFVKernels for the momentum equations.");
  }

  // Get baseline force if force-correction method is used for volumetric correction
  if (_bool_correct_vf && _volume_force_correction_method == "force-consistent")
  {
    _baseline_volume_force = 1e10;
    for (const auto & loc_elem : *_elem_range)
    {
      Real elem_value = 0.0;
      for (const auto i : make_range(_volumetric_force.size()))
        elem_value += (*_volumetric_force[i])(makeElemArg(loc_elem), determineState());

      if (std::abs(elem_value) < _baseline_volume_force)
        _baseline_volume_force = std::abs(elem_value);
      if (_baseline_volume_force == 0)
        break;
    }
    _communicator.min(_baseline_volume_force);
  }
}

void
INSFVRhieChowInterpolator::insfvSetup()
{
  _elem_range =
      std::make_unique<ConstElemRange>(_mesh.active_local_subdomain_set_elements_begin(blockIDs()),
                                       _mesh.active_local_subdomain_set_elements_end(blockIDs()));
}

void
INSFVRhieChowInterpolator::meshChanged()
{
  insfvSetup();

  // If the mesh has been modified:
  // - the boundary elements may have changed
  // - some elements may have been refined
  _elements_to_push_pull.clear();
  _a.clear();
}

void
INSFVRhieChowInterpolator::initialize()
{
  if (!needAComputation())
    return;

  // Reset map of coefficients to zero.
  // The keys should not have changed unless the mesh has changed
  // Dont reset if not in current system
  // IDEA: clear them derivatives
  if (_momentum_sys_number == _fe_problem.currentNlSysNum())
    for (auto & pair : _a)
      pair.second = 0;
  else
    for (auto & pair : _a)
    {
      auto & a_val = pair.second;
      a_val = MetaPhysicL::raw_value(a_val);
    }
}

void
INSFVRhieChowInterpolator::execute()
{
  // Either we provided the RC coefficients using aux-variable, or we are solving for another
  // system than the momentum equations are in, in a multi-system setup for example
  if (_fe_problem.currentNlSysNum() != _momentum_sys_number)
    return;

  mooseAssert(!_a_data_provided,
              "a-coefficient data should not be provided if the velocity variables are in the "
              "nonlinear system and we are running kernels that compute said a-coefficients");
  // One might think that we should do a similar assertion for
  // (_velocity_interp_method == Moose::FV::InterpMethod::RhieChow). However, even if we are not
  // using the generated a-coefficient data in that case, some kernels have been optimized to
  // add their residuals into the global system during the generation of the a-coefficient data.
  // Hence if we were to skip the kernel execution we would drop those residuals

  TIME_SECTION("execute", 1, "Computing Rhie-Chow coefficients");

  // A lot of RC data gathering leverages the automatic differentiation system, e.g. for linear
  // operators we pull out the 'a' coefficients by querying the ADReal residual derivatives
  // member at the element or neighbor dof locations. Consequently we need to enable derivative
  // computation. We do this here outside the threaded regions
  const auto saved_do_derivatives = ADReal::do_derivatives;
  ADReal::do_derivatives = true;

  PARALLEL_TRY
  {
    GatherRCDataElementThread et(_fe_problem, _momentum_sys_number, _var_numbers);
    Threads::parallel_reduce(*_elem_range, et);
  }
  PARALLEL_CATCH;

  PARALLEL_TRY
  {
    using FVRange = StoredRange<MooseMesh::const_face_info_iterator, const FaceInfo *>;
    GatherRCDataFaceThread<FVRange> fvr(
        _fe_problem, _momentum_sys_number, _var_numbers, _displaced);
    FVRange faces(_moose_mesh.ownedFaceInfoBegin(), _moose_mesh.ownedFaceInfoEnd());
    Threads::parallel_reduce(faces, fvr);
  }
  PARALLEL_CATCH;

  ADReal::do_derivatives = saved_do_derivatives;
}

void
INSFVRhieChowInterpolator::finalize()
{
  if (!needAComputation() || this->n_processors() == 1)
    return;

  // If advecting with auxiliary variables, no need to synchronize data
  // Same if not solving for the velocity variables at the moment
  if (_fe_problem.currentNlSysNum() != _momentum_sys_number)
    return;

  using Datum = std::pair<dof_id_type, VectorValue<ADReal>>;
  std::unordered_map<processor_id_type, std::vector<Datum>> push_data;
  std::unordered_map<processor_id_type, std::vector<dof_id_type>> pull_requests;
  static const VectorValue<ADReal> example;

  // Create push data
  for (const auto * const elem : _elements_to_push_pull)
  {
    const auto id = elem->id();
    const auto pid = elem->processor_id();
    auto it = _a.find(id);
    mooseAssert(it != _a.end(), "We definitely should have found something");
    push_data[pid].push_back(std::make_pair(id, it->second));
  }

  // Create pull data
  if (_pull_all_nonlocal)
  {
    for (const auto * const elem :
         as_range(_mesh.active_not_local_elements_begin(), _mesh.active_not_local_elements_end()))
      if (blockIDs().count(elem->subdomain_id()))
        pull_requests[elem->processor_id()].push_back(elem->id());
  }
  else
  {
    for (const auto * const elem : _elements_to_push_pull)
      pull_requests[elem->processor_id()].push_back(elem->id());
  }

  // First push
  {
    auto action_functor =
        [this](const processor_id_type libmesh_dbg_var(pid), const std::vector<Datum> & sent_data)
    {
      mooseAssert(pid != this->processor_id(), "We do not send messages to ourself here");
      for (const auto & pr : sent_data)
        _a[pr.first] += pr.second;
    };
    TIMPI::push_parallel_vector_data(_communicator, push_data, action_functor);
  }

  // Then pull
  {
    auto gather_functor = [this](const processor_id_type libmesh_dbg_var(pid),
                                 const std::vector<dof_id_type> & elem_ids,
                                 std::vector<VectorValue<ADReal>> & data_to_fill)
    {
      mooseAssert(pid != this->processor_id(), "We shouldn't be gathering from ourselves.");
      data_to_fill.resize(elem_ids.size());
      for (const auto i : index_range(elem_ids))
      {
        const auto id = elem_ids[i];
        auto it = _a.find(id);
        mooseAssert(it != _a.end(), "We should hold the value for this locally");
        data_to_fill[i] = it->second;
      }
    };

    auto action_functor = [this](const processor_id_type libmesh_dbg_var(pid),
                                 const std::vector<dof_id_type> & elem_ids,
                                 const std::vector<VectorValue<ADReal>> & filled_data)
    {
      mooseAssert(pid != this->processor_id(), "The request filler shouldn't have been ourselves");
      mooseAssert(elem_ids.size() == filled_data.size(), "I think these should be the same size");
      for (const auto i : index_range(elem_ids))
        _a[elem_ids[i]] = filled_data[i];
    };
    TIMPI::pull_parallel_vector_data(
        _communicator, pull_requests, gather_functor, action_functor, &example);
  }
}

void
INSFVRhieChowInterpolator::ghostADataOnBoundary(const BoundaryID boundary_id)
{
  if (!needAComputation() || this->n_processors() == 1)
    return;

  // Ghost a for the elements on the boundary
  for (auto elem_id : _moose_mesh.getBoundaryActiveSemiLocalElemIds(boundary_id))
  {
    const auto & elem = _moose_mesh.elemPtr(elem_id);
    // no need to ghost if locally owned or far from local process
    if (elem->processor_id() != this->processor_id() && elem->is_semilocal(this->processor_id()))
      // Adding to the a coefficient will make sure the final result gets communicated
      addToA(elem, 0, 0);
  }

  // Ghost a for the neighbors of the elements on the boundary
  for (auto neighbor_id : _moose_mesh.getBoundaryActiveNeighborElemIds(boundary_id))
  {
    const auto & neighbor = _moose_mesh.queryElemPtr(neighbor_id);
    // no need to ghost if locally owned or far from local process
    if (neighbor->processor_id() != this->processor_id() &&
        neighbor->is_semilocal(this->processor_id()))
      // Adding to the a coefficient will make sure the final result gets communicated
      addToA(neighbor, 0, 0);
  }
}

VectorValue<ADReal>
INSFVRhieChowInterpolator::getVelocity(const Moose::FV::InterpMethod m,
                                       const FaceInfo & fi,
                                       const Moose::StateArg & time,
                                       const THREAD_ID tid,
                                       const bool subtract_mesh_velocity) const
{
  const Elem * const elem = &fi.elem();
  const Elem * const neighbor = fi.neighborPtr();
  auto & vel = *_vel[tid];
  auto & p = *_ps[tid];
  auto * const u = _us[tid];
  MooseVariableFVReal * const v = _v ? _vs[tid] : nullptr;
  MooseVariableFVReal * const w = _w ? _ws[tid] : nullptr;
  // Check if skewness-correction is necessary
  const bool correct_skewness = velocitySkewCorrection(tid);
  auto incorporate_mesh_velocity =
      [this, tid, subtract_mesh_velocity, &time](const auto & space, auto & velocity)
  {
    if (_disps.size() && subtract_mesh_velocity)
      velocity -= _disps[tid]->dot(space, time);
  };

  if (Moose::FV::onBoundary(*this, fi))
  {
    const Elem * const boundary_elem = hasBlocks(elem->subdomain_id()) ? elem : neighbor;
    const Moose::FaceArg boundary_face{&fi,
                                       Moose::FV::LimiterType::CentralDifference,
                                       true,
                                       correct_skewness,
                                       boundary_elem,
                                       nullptr};
    auto velocity = vel(boundary_face, time);
    incorporate_mesh_velocity(boundary_face, velocity);

    // If not solving for velocity, clear derivatives
    if (_fe_problem.currentNlSysNum() != _momentum_sys_number)
      return MetaPhysicL::raw_value(velocity);
    else
      return velocity;
  }

  VectorValue<ADReal> velocity;

  Moose::FaceArg face{
      &fi, Moose::FV::LimiterType::CentralDifference, true, correct_skewness, nullptr, nullptr};
  // Create the average face velocity (not corrected using RhieChow yet)
  velocity(0) = (*u)(face, time);
  if (v)
    velocity(1) = (*v)(face, time);
  if (w)
    velocity(2) = (*w)(face, time);

  incorporate_mesh_velocity(face, velocity);

  // If not solving for velocity, clear derivatives
  if (_fe_problem.currentNlSysNum() != _momentum_sys_number)
    velocity = MetaPhysicL::raw_value(velocity);

  // Return if Rhie-Chow was not requested or if we have a porosity jump
  if (m == Moose::FV::InterpMethod::Average ||
      std::get<0>(NS::isPorosityJumpFace(epsilon(tid), fi, time)))
    return velocity;

  // Rhie-Chow coefficients are not available on initial
  if (_fe_problem.getCurrentExecuteOnFlag() == EXEC_INITIAL)
  {
    mooseDoOnce(mooseWarning("Cannot compute Rhie Chow coefficients on initial. Returning linearly "
                             "interpolated velocities"););
    return velocity;
  }
  if (!_fe_problem.shouldSolve())
  {
    mooseDoOnce(mooseWarning("Cannot compute Rhie Chow coefficients if not solving. Returning "
                             "linearly interpolated velocities"););
    return velocity;
  }

  mooseAssert(((m == Moose::FV::InterpMethod::RhieChow) &&
               (_velocity_interp_method == Moose::FV::InterpMethod::RhieChow)) ||
                  _a_data_provided,
              "The 'a' coefficients have not been generated or provided for "
              "Rhie Chow velocity interpolation.");

  mooseAssert(neighbor && this->hasBlocks(neighbor->subdomain_id()),
              "We should be on an internal face...");

  // Get pressure gradient. This is the uncorrected gradient plus a correction from cell
  // centroid values on either side of the face
  const auto correct_skewness_p = pressureSkewCorrection(tid);
  const auto & grad_p = p.adGradSln(fi, time, correct_skewness_p);

  // Get uncorrected pressure gradient. This will use the element centroid gradient if we are
  // along a boundary face
  const auto & unc_grad_p = p.uncorrectedAdGradSln(fi, time, correct_skewness_p);

  // Volumetric Correction Method #1: pressure-based correction
  // Function that allows us to mark the face for which the Rhie-Chow interpolation is
  // inconsistent Normally, we should apply a reconstructed volume correction to the Rhie-Chow
  // coefficients However, since the fluxes at the face are given by the volume force we will
  // simply mark the face add the reverse pressure interpolation for these faces In brief, this
  // function is just marking the faces where the Rhie-Chow interpolation is inconsistent
  auto vf_indicator_pressure_based =
      [this, &elem, &neighbor, &time, &fi, &correct_skewness](const Point & unit_basis_vector)
  {
    // Holders for the interpolated corrected and uncorrected volume force
    Real interp_vf;
    Real uncorrected_interp_vf;

    // Compute the corrected interpolated face value
    Moose::FaceArg face{
        &fi, Moose::FV::LimiterType::CentralDifference, true, correct_skewness, nullptr, nullptr};

    interp_vf = 0.0;
    for (const auto i : make_range(_volumetric_force.size()))
      interp_vf += (*this->_volumetric_force[i])(face, time);

    // Compute the uncorrected interpolated face value
    // For it to be consistent with the pressure gradient interpolation `uncorrectedAdGradSln`
    // the uncorrected volume force computation should follow the same Green-Gauss process

    Real elem_value = 0.0;
    Real neigh_value = 0.0;

    // Uncorrected interpolation - Step 1: loop over the faces of the element to compute
    // face-average cell value
    Real coord_multiplier;
    const auto coord_type = _fe_problem.getCoordSystem(elem->subdomain_id());
    const unsigned int rz_radial_coord =
        Moose::COORD_RZ ? _fe_problem.getAxisymmetricRadialCoord() : libMesh::invalid_uint;

    for (const auto side : make_range(elem->n_sides()))
    {
      const Elem * const loc_neighbor = elem->neighbor_ptr(side);
      const bool elem_has_fi = Moose::FV::elemHasFaceInfo(*elem, loc_neighbor);
      const FaceInfo * const fi_loc =
          _moose_mesh.faceInfo(elem_has_fi ? elem : loc_neighbor,
                               elem_has_fi ? side : loc_neighbor->which_neighbor_am_i(elem));

      Moose::FaceArg loc_face{
          fi_loc, Moose::FV::LimiterType::CentralDifference, true, correct_skewness, elem, nullptr};

      MooseMeshUtils::coordTransformFactor(
          elem->vertex_average(), coord_multiplier, coord_type, rz_radial_coord);

      Real face_volume_contribution = fi_loc->faceArea() *
                                      (neighbor->vertex_average() - elem->vertex_average()).norm() *
                                      coord_multiplier;

      for (const auto i : make_range(_volumetric_force.size()))
      {
        // Add which side (can be both, then we use a nullptr) of the face info the force is defined
        // on
        loc_face.face_side =
            this->_volumetric_force[i]->hasFaceSide(*fi_loc, true)
                ? (this->_volumetric_force[i]->hasFaceSide(*fi_loc, false) ? nullptr
                                                                           : fi_loc->elemPtr())
                : fi_loc->neighborPtr();
        elem_value += (*this->_volumetric_force[i])(loc_face, time) * face_volume_contribution *
                      (fi_loc->normal() * unit_basis_vector);
      }
    }
    elem_value = elem_value / elem->volume();

    // Uncorrected interpolation - Step 2: loop over the face of the neighbor to compute
    // face-average cell value
    for (const auto side : make_range(neighbor->n_sides()))
    {
      const Elem * const loc_elem = neighbor->neighbor_ptr(side);
      const bool elem_has_fi = Moose::FV::elemHasFaceInfo(*neighbor, loc_elem);
      const FaceInfo * const fi_loc =
          _moose_mesh.faceInfo(elem_has_fi ? neighbor : loc_elem,
                               elem_has_fi ? side : loc_elem->which_neighbor_am_i(neighbor));

      Moose::FaceArg loc_face{
          fi_loc, Moose::FV::LimiterType::CentralDifference, true, correct_skewness, elem, nullptr};

      MooseMeshUtils::coordTransformFactor(
          neighbor->vertex_average(), coord_multiplier, coord_type, rz_radial_coord);

      Real face_volume_contribution = fi_loc->faceArea() *
                                      (elem->vertex_average() - neighbor->vertex_average()).norm() *
                                      coord_multiplier;

      for (const auto i : make_range(_volumetric_force.size()))
      {
        loc_face.face_side =
            this->_volumetric_force[i]->hasFaceSide(*fi_loc, true)
                ? (this->_volumetric_force[i]->hasFaceSide(*fi_loc, false) ? nullptr
                                                                           : fi_loc->elemPtr())
                : fi_loc->neighborPtr();
        neigh_value += (*this->_volumetric_force[i])(loc_face, time) * face_volume_contribution *
                       (fi_loc->normal() * unit_basis_vector);
      }
    }
    neigh_value = neigh_value / neighbor->volume();

    // Uncorrected interpolation - Step 3: interpolate element and neighbor reconstructed values
    // to the face
    MooseMeshUtils::coordTransformFactor(
        fi.faceCentroid(), coord_multiplier, coord_type, rz_radial_coord);
    interpolate(
        Moose::FV::InterpMethod::Average, uncorrected_interp_vf, elem_value, neigh_value, fi, true);

    // Return the flag indicator on which face the volume force correction is inconsistent
    return MooseUtils::relativeFuzzyEqual(interp_vf, uncorrected_interp_vf, 1e-10) ? 0.0 : 1.0;
  };

  // Volumetric Correction Method #2: volume-based correction
  // In thery, pressure and velocity cannot be decoupled when a body force is present
  // Hence, we can de-activate the RC cofficient in faces that have a normal volume force
  // In the method we mark the faces with a non-zero volume force with recpect to the baseline
  auto vf_indicator_force_based = [this, &time, &fi, &correct_skewness](Point & face_normal)
  {
    Real value = 0.0;
    Moose::FaceArg loc_face{
        &fi, Moose::FV::LimiterType::CentralDifference, true, correct_skewness, nullptr, nullptr};

    for (const auto i : make_range(_volumetric_force.size()))
      value += (*_volumetric_force[i])(loc_face, time) * (face_normal * fi.normal());
    if ((std::abs(value) - _baseline_volume_force) > 0)
      return 1.0;
    else
      return 0.0;
  };

  const Point & elem_centroid = fi.elemCentroid();
  const Point & neighbor_centroid = fi.neighborCentroid();
  Real elem_volume = fi.elemVolume();
  Real neighbor_volume = fi.neighborVolume();

  // Now we need to perform the computations of D
  const auto elem_a = (*_a_read[tid])(makeElemArg(elem), time);

  mooseAssert(UserObject::_subproblem.getCoordSystem(elem->subdomain_id()) ==
                  UserObject::_subproblem.getCoordSystem(neighbor->subdomain_id()),
              "Coordinate systems must be the same between the two elements");

  Real coord;
  coordTransformFactor(UserObject::_subproblem, elem->subdomain_id(), elem_centroid, coord);

  elem_volume *= coord;

  VectorValue<ADReal> elem_D = 0;
  for (const auto i : make_range(_dim))
  {
    mooseAssert(elem_a(i).value() != 0, "We should not be dividing by zero");
    elem_D(i) = elem_volume / elem_a(i);
  }

  VectorValue<ADReal> face_D;

  const auto neighbor_a = (*_a_read[tid])(makeElemArg(neighbor), time);

  coordTransformFactor(UserObject::_subproblem, neighbor->subdomain_id(), neighbor_centroid, coord);
  neighbor_volume *= coord;

  VectorValue<ADReal> neighbor_D = 0;
  for (const auto i : make_range(_dim))
  {
    mooseAssert(neighbor_a(i).value() != 0, "We should not be dividing by zero");
    neighbor_D(i) = neighbor_volume / neighbor_a(i);
  }

  // We require this to ensure that the correct interpolation weights are used.
  // This will change once the traditional weights are replaced by the weights
  // that are used by the skewness-correction.
  Moose::FV::InterpMethod coeff_interp_method = correct_skewness
                                                    ? Moose::FV::InterpMethod::SkewCorrectedAverage
                                                    : Moose::FV::InterpMethod::Average;
  Moose::FV::interpolate(coeff_interp_method, face_D, elem_D, neighbor_D, fi, true);

  // evaluate face porosity, see (18) in Hanimann 2021 or (11) in Nordlund 2016
  const auto face_eps = epsilon(tid)(face, time);

  // Perform the pressure correction. We don't use skewness-correction on the pressure since
  // it only influences the averaged cell gradients which cancel out in the correction
  // below.
  for (const auto i : make_range(_dim))
  {
    // "Standard" pressure-based RC interpolation
    velocity(i) -= face_D(i) * face_eps * (grad_p(i) - unc_grad_p(i));

    if (_bool_correct_vf)
    {
      // To solve the volume force incorrect interpolation, we add back the pressure gradient to the
      // RC-inconsistent faces regarding the marking method
      Point unit_basis_vector;
      unit_basis_vector(i) = 1.0;

      // Get the value of the correction face indicator
      Real correction_indicator;
      if (_volume_force_correction_method == "force-consistent")
        correction_indicator = vf_indicator_force_based(unit_basis_vector);
      else
        correction_indicator = vf_indicator_pressure_based(unit_basis_vector);

      // Correct back the velocity
      velocity(i) += face_D(i) * face_eps * (grad_p(i) - unc_grad_p(i)) * correction_indicator;
    }
  }

  // If not solving for velocity, clear derivatives
  if (_fe_problem.currentNlSysNum() != _momentum_sys_number)
    return MetaPhysicL::raw_value(velocity);
  else
    return velocity;
}
