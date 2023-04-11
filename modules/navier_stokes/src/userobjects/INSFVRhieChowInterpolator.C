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
#include "SubProblem.h"
#include "MooseMesh.h"
#include "SystemBase.h"
#include "NS.h"
#include "Assembly.h"
#include "INSFVVelocityVariable.h"
#include "PiecewiseByBlockLambdaFunctor.h"
#include "VectorCompositeFunctor.h"
#include "FVElementalKernel.h"
#include "NSFVUtils.h"

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
INSFVRhieChowInterpolator::validParams()
{
  auto params = GeneralUserObject::validParams();
  params += TaggingInterface::validParams();
  params += BlockRestrictable::validParams();
  ExecFlagEnum & exec_enum = params.set<ExecFlagEnum>("execute_on", true);
  exec_enum.addAvailableFlags(EXEC_PRE_KERNELS);
  exec_enum = {EXEC_PRE_KERNELS};
  params.suppressParameter<ExecFlagEnum>("execute_on");

  // Avoid uninitialized residual objects
  params.suppressParameter<bool>("force_preic");

  MooseEnum velocity_interp_method("average rc", "rc");
  params.addParam<MooseEnum>(
      "velocity_interp_method",
      velocity_interp_method,
      "The interpolation to use for the velocity. Options are "
      "'average' and 'rc' which stands for Rhie-Chow. The default is Rhie-Chow.");
  params.addRequiredParam<VariableName>(NS::pressure, "The pressure variable.");
  params.addRequiredParam<VariableName>("u", "The x-component of velocity");
  params.addParam<VariableName>("v", "The y-component of velocity");
  params.addParam<VariableName>("w", "The z-component of velocity");
  params.addClassDescription(
      "Computes the Rhie-Chow velocity based on gathered 'a' coefficient data.");
  params.addParam<MooseFunctorName>(
      "a_u",
      "For simulations in which the advecting velocities are aux variables, this parameter must be "
      "supplied. It represents the on-diagonal coefficients for the 'x' component velocity, solved "
      "via the Navier-Stokes equations.");
  params.addParam<MooseFunctorName>(
      "a_v",
      "For simulations in which the advecting velocities are aux variables, this parameter must be "
      "supplied when the mesh dimension is greater than 1. It represents the on-diagonal "
      "coefficients for the 'y' component velocity, solved "
      "via the Navier-Stokes equations.");
  params.addParam<MooseFunctorName>(
      "a_w",
      "For simulations in which the advecting velocities are aux variables, this parameter must be "
      "supplied when the mesh dimension is greater than 2. It represents the on-diagonal "
      "coefficients for the 'z' component velocity, solved "
      "via the Navier-Stokes equations.");
  params.addParam<NonlinearSystemName>("mass_momentum_system",
                                       "nl0",
                                       "The nonlinear system in which the monolithic momentum and "
                                       "continuity equations are located.");
  return params;
}

INSFVRhieChowInterpolator::INSFVRhieChowInterpolator(const InputParameters & params)
  : GeneralUserObject(params),
    TaggingInterface(this),
    BlockRestrictable(this),
    ADFunctorInterface(this),
    _moose_mesh(UserObject::_subproblem.mesh()),
    _mesh(_moose_mesh.getMesh()),
    _dim(_moose_mesh.dimension()),
    _vel(libMesh::n_threads()),
    _p(dynamic_cast<INSFVPressureVariable *>(
        &UserObject::_subproblem.getVariable(0, getParam<VariableName>(NS::pressure)))),
    _u(dynamic_cast<INSFVVelocityVariable *>(
        &UserObject::_subproblem.getVariable(0, getParam<VariableName>("u")))),
    _v(isParamValid("v") ? dynamic_cast<INSFVVelocityVariable *>(
                               &UserObject::_subproblem.getVariable(0, getParam<VariableName>("v")))
                         : nullptr),
    _w(isParamValid("w") ? dynamic_cast<INSFVVelocityVariable *>(
                               &UserObject::_subproblem.getVariable(0, getParam<VariableName>("w")))
                         : nullptr),
    _ps(libMesh::n_threads(), nullptr),
    _us(libMesh::n_threads(), nullptr),
    _vs(libMesh::n_threads(), nullptr),
    _ws(libMesh::n_threads(), nullptr),
    _sub_ids(blockRestricted() ? blockIDs() : _moose_mesh.meshSubdomains()),
    _a(_moose_mesh, _sub_ids, "a"),
    _ax(_a, 0),
    _ay(_a, 1),
    _az(_a, 2),
    _nl_sys_number(_fe_problem.nlSysNum(getParam<NonlinearSystemName>("mass_momentum_system"))),
    _sys(*getCheckedPointerParam<SystemBase *>("_sys")),
    _example(0),
    _a_data_provided(false)
{
  if (!_p)
    paramError(NS::pressure, "the pressure must be a INSFVPressureVariable.");

  auto fill_container = [this](const auto & name, auto & container)
  {
    for (const auto tid : make_range(libMesh::n_threads()))
    {
      auto * const var = static_cast<MooseVariableFVReal *>(
          &UserObject::_subproblem.getVariable(tid, getParam<VariableName>(name)));
      container[tid] = var;
    }
  };

  auto check_blocks = [this](const auto & var)
  {
    if (blockIDs() != var.blockIDs())
      mooseError("Block restriction of interpolator user object '",
                 this->name(),
                 "' (",
                 Moose::stringify(blocks()),
                 ") doesn't match the block restriction of variable '",
                 var.name(),
                 "' (",
                 Moose::stringify(var.blocks()),
                 ")");
  };

  fill_container(NS::pressure, _ps);
  check_blocks(*_p);

  if (!_u)
    paramError("u", "the u velocity must be an INSFVVelocityVariable.");
  fill_container("u", _us);
  check_blocks(*_u);
  _var_numbers.push_back(_u->number());

  if (_dim >= 2)
  {
    if (!_v)
      mooseError("In two or more dimensions, the v velocity must be supplied and it must be an "
                 "INSFVVelocityVariable.");

    fill_container("v", _vs);
    check_blocks(*_v);
    _var_numbers.push_back(_v->number());
    if (_v->faceInterpolationMethod() != _u->faceInterpolationMethod())
      mooseError("x and y velocity component face interpolation methods do not match");
  }

  if (_dim >= 3)
  {
    if (!_w)
      mooseError("In three-dimensions, the w velocity must be supplied and it must be an "
                 "INSFVVelocityVariable.");

    fill_container("w", _ws);
    check_blocks(*_w);
    _var_numbers.push_back(_w->number());
    if (_w->faceInterpolationMethod() != _u->faceInterpolationMethod())
      mooseError("x and z velocity component face interpolation methods do not match");
  }

  if (&(UserObject::_subproblem) != &(TaggingInterface::_subproblem))
    mooseError("Different subproblems in INSFVRhieChowInterpolator!");

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
  }

  const auto & velocity_interp_method = params.get<MooseEnum>("velocity_interp_method");
  if (velocity_interp_method == "average")
  {
    _velocity_interp_method = Moose::FV::InterpMethod::Average;
    if (isParamValid("a_u"))
      paramError("a_u",
                 "Rhie Chow coefficients may not be specified for average velocity interpolation");
  }
  else if (velocity_interp_method == "rc")
    _velocity_interp_method = Moose::FV::InterpMethod::RhieChow;

  if (_velocity_interp_method != Moose::FV::InterpMethod::Average)
    fillARead();
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

      // We are the fluid flow application, so we should make sure users have the ability to write
      // 'a' out to aux variables for possible transfer to other applications
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
      mooseError(
          "No INSFVKernels detected for the velocity variables. "
          "If you are trying to use auxiliary variables for advection, please specify the a_u/v/w "
          "coefficients. If not, please specify INSFVKernels for the momentum equations.");
  }
}

void
INSFVRhieChowInterpolator::insfvSetup()
{
  _elem_range =
      std::make_unique<ConstElemRange>(_mesh.active_local_subdomain_set_elements_begin(_sub_ids),
                                       _mesh.active_local_subdomain_set_elements_end(_sub_ids));
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
  // Reset map of coefficients to zero.
  // The keys should not have changed unless the mesh has changed
  for (const auto & pair : _a)
    _a[pair.first] = 0;
}

void
INSFVRhieChowInterpolator::execute()
{
  if (_a_data_provided)
    return;

  // If advecting with auxiliary variables, no need to try to run those kernels
  if (_sys.number() != _u->sys().number())
    return;

  TIME_SECTION("execute", 1, "Computing Rhie-Chow coefficients");

  // A lot of RC data gathering leverages the automatic differentiation system, e.g. for linear
  // operators we pull out the 'a' coefficients by querying the ADReal residual derivatives member
  // at the element or neighbor dof locations. Consequently we need to enable derivative
  // computation. We do this here outside the threaded regions
  const auto saved_do_derivatives = ADReal::do_derivatives;
  ADReal::do_derivatives = true;

  PARALLEL_TRY
  {
    GatherRCDataElementThread et(_fe_problem, _nl_sys_number, _var_numbers);
    Threads::parallel_reduce(*_elem_range, et);
  }
  PARALLEL_CATCH;

  PARALLEL_TRY
  {
    using FVRange = StoredRange<MooseMesh::const_face_info_iterator, const FaceInfo *>;
    GatherRCDataFaceThread<FVRange> fvr(_fe_problem, _nl_sys_number, _var_numbers);
    FVRange faces(_fe_problem.mesh().ownedFaceInfoBegin(), _fe_problem.mesh().ownedFaceInfoEnd());
    Threads::parallel_reduce(faces, fvr);
  }
  PARALLEL_CATCH;

  ADReal::do_derivatives = saved_do_derivatives;
}

void
INSFVRhieChowInterpolator::finalize()
{
  if (_a_data_provided || this->n_processors() == 1 ||
      _velocity_interp_method == Moose::FV::InterpMethod::Average)
    return;

  using Datum = std::pair<dof_id_type, VectorValue<ADReal>>;
  std::unordered_map<processor_id_type, std::vector<Datum>> push_data;
  std::unordered_map<processor_id_type, std::vector<dof_id_type>> pull_requests;
  static const VectorValue<ADReal> example;

  for (auto * const elem : _elements_to_push_pull)
  {
    const auto id = elem->id();
    const auto pid = elem->processor_id();
    auto it = _a.find(id);
    mooseAssert(it != _a.end(), "We definitely should have found something");
    push_data[pid].push_back(std::make_pair(id, it->second));
    pull_requests[pid].push_back(id);
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
      {
        const auto id = elem_ids[i];
        auto it = _a.find(id);
        mooseAssert(it != _a.end(), "We requested this so we must have it in the map");
        it->second = filled_data[i];
      }
    };
    TIMPI::pull_parallel_vector_data(
        _communicator, pull_requests, gather_functor, action_functor, &example);
  }
}

void
INSFVRhieChowInterpolator::ghostADataOnBoundary(const BoundaryID boundary_id)
{
  if (_a_data_provided || this->n_processors() == 1 ||
      _velocity_interp_method == Moose::FV::InterpMethod::Average)
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
                                       const THREAD_ID tid) const
{
  const Elem * const elem = &fi.elem();
  const Elem * const neighbor = fi.neighborPtr();
  auto & vel = *_vel[tid];
  auto & p = *_ps[tid];
  auto * const u = _us[tid];
  MooseVariableFVReal * const v = _v ? _vs[tid] : nullptr;
  MooseVariableFVReal * const w = _w ? _ws[tid] : nullptr;

  // Check if skewness-correction is necessary
  const bool correct_skewness =
      (u->faceInterpolationMethod() == Moose::FV::InterpMethod::SkewCorrectedAverage);

  if (Moose::FV::onBoundary(*this, fi))
  {
    const Elem * const boundary_elem = hasBlocks(elem->subdomain_id()) ? elem : neighbor;
    const Moose::FaceArg boundary_face{
        &fi, Moose::FV::LimiterType::CentralDifference, true, correct_skewness, boundary_elem};
    return vel(boundary_face, time);
  }

  VectorValue<ADReal> velocity;

  Moose::FaceArg face{
      &fi, Moose::FV::LimiterType::CentralDifference, true, correct_skewness, nullptr};
  // Create the average face velocity (not corrected using RhieChow yet)
  velocity(0) = (*u)(face, time);
  if (v)
    velocity(1) = (*v)(face, time);
  if (w)
    velocity(2) = (*w)(face, time);

  // Return if Rhie-Chow was not requested or if we have a porosity jump
  if (m == Moose::FV::InterpMethod::Average ||
      std::get<0>(NS::isPorosityJumpFace(epsilon(tid), fi, time)))
    return velocity;

  mooseAssert(((m == Moose::FV::InterpMethod::RhieChow) &&
               (_velocity_interp_method == Moose::FV::InterpMethod::RhieChow)) ||
                  _a_data_provided,
              "The 'a' coefficients have not been generated or provided for "
              "Rhie Chow velocity interpolation.");

  mooseAssert(neighbor && this->hasBlocks(neighbor->subdomain_id()),
              "We should be on an internal face...");

  // Get pressure gradient. This is the uncorrected gradient plus a correction from cell centroid
  // values on either side of the face
  const VectorValue<ADReal> & grad_p = p.adGradSln(fi, time);

  // Get uncorrected pressure gradient. This will use the element centroid gradient if we are
  // along a boundary face
  const VectorValue<ADReal> & unc_grad_p = p.uncorrectedAdGradSln(fi, time);

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
    velocity(i) -= face_D(i) * face_eps * (grad_p(i) - unc_grad_p(i));

  return velocity;
}

bool
INSFVRhieChowInterpolator::hasFaceSide(const FaceInfo & fi, const bool fi_elem_side) const
{
  if (fi_elem_side)
    return hasBlocks(fi.elem().subdomain_id());
  else
    return fi.neighborPtr() && hasBlocks(fi.neighbor().subdomain_id());
}
