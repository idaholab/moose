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
#include "Reconstructions.h"
#include "Assembly.h"
#include "INSFVVelocityVariable.h"
#include "INSFVPressureVariable.h"
#include "PiecewiseByBlockLambdaFunctor.h"

#include "libmesh/mesh_base.h"
#include "libmesh/elem_range.h"
#include "libmesh/parallel_algebra.h"
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
  params.addRequiredParam<VariableName>(NS::pressure, "The pressure variable.");
  params.addRequiredParam<VariableName>("u", "The x-component of velocity");
  params.addParam<VariableName>("v", "The y-component of velocity");
  params.addParam<VariableName>("w", "The z-component of velocity");
  params.addClassDescription(
      "Computes the Rhie-Chow velocity based on gathered 'a' coefficient data.");
  return params;
}

INSFVRhieChowInterpolator::INSFVRhieChowInterpolator(const InputParameters & params)
  : GeneralUserObject(params),
    TaggingInterface(this),
    BlockRestrictable(this),
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
    _sys(*getCheckedPointerParam<SystemBase *>("_sys")),
    _example(0)
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
                 "' doesn't match the block restriction of variable '",
                 var.name(),
                 "'");
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
  }

  if (_dim >= 3)
  {
    if (!_w)
      mooseError("In three-dimensions, the w velocity must be supplied and it must be an "
                 "INSFVVelocityVariable.");

    fill_container("w", _ws);
    check_blocks(*_w);
    _var_numbers.push_back(_w->number());
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
}

void
INSFVRhieChowInterpolator::initialSetup()
{
  for (const auto var_num : _var_numbers)
  {
    std::vector<MooseObject *> var_objects;
    _fe_problem.theWarehouse()
        .query()
        .template condition<AttribVar>(static_cast<int>(var_num))
        .template condition<AttribResidualObject>(true)
        .queryInto(var_objects);
    for (auto * const var_object : var_objects)
      if (!dynamic_cast<INSFVMomentumResidualObject *>(var_object))
        mooseError("Object ",
                   var_object->name(),
                   " is not a INSFVMomentumResidualObject. Make sure that all the objects applied "
                   "to the momentum equation are INSFV or derived objects.");
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
INSFVRhieChowInterpolator::residualSetup()
{
  if (!_initial_setup_done)
    insfvSetup();

  _initial_setup_done = true;
}

void
INSFVRhieChowInterpolator::meshChanged()
{
  insfvSetup();
}

void
INSFVRhieChowInterpolator::initialize()
{
  _elements_to_push_pull.clear();
  _a.clear();
}

void
INSFVRhieChowInterpolator::execute()
{
  // A lot of RC data gathering leverages the automatic differentiation system, e.g. for linear
  // operators we pull out the 'a' coefficents by querying the ADReal residual derivatives member at
  // the element or neighbor dof locations. Consequently we need to enable derivative computation.
  // We do this here outside the threaded regions
  const auto saved_do_derivatives = ADReal::do_derivatives;
  ADReal::do_derivatives = true;

  PARALLEL_TRY
  {
    GatherRCDataElementThread et(_fe_problem, _var_numbers);
    Threads::parallel_reduce(*_elem_range, et);
  }
  PARALLEL_CATCH;

  PARALLEL_TRY
  {
    using FVRange = StoredRange<std::vector<const FaceInfo *>::const_iterator, const FaceInfo *>;
    GatherRCDataFaceThread<FVRange> fvr(_fe_problem, _var_numbers);
    FVRange faces(_fe_problem.mesh().faceInfo().begin(), _fe_problem.mesh().faceInfo().end());
    Threads::parallel_reduce(faces, fvr);
  }
  PARALLEL_CATCH;

  ADReal::do_derivatives = saved_do_derivatives;
}

void
INSFVRhieChowInterpolator::finalize()
{
#ifdef MOOSE_GLOBAL_AD_INDEXING
  if (this->n_processors() == 1)
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
#else
  mooseError("INSFVRhieChowInterpolator only supported for global AD indexing.");
#endif
}

VectorValue<ADReal>
INSFVRhieChowInterpolator::getVelocity(const Moose::FV::InterpMethod m,
                                       const FaceInfo & fi,
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
    const auto sub_id =
        hasBlocks(elem->subdomain_id()) ? elem->subdomain_id() : neighbor->subdomain_id();
    const Moose::SingleSidedFaceArg boundary_face{&fi,
                                                  Moose::FV::LimiterType::CentralDifference,
                                                  true,
                                                  correct_skewness,
                                                  correct_skewness,
                                                  sub_id};
    return vel(boundary_face);
  }

  VectorValue<ADReal> velocity;

  // Create the average face velocity (not corrected using RhieChow yet)
  velocity(0) = u->getInternalFaceValue(fi, correct_skewness);
  if (v)
    velocity(1) = v->getInternalFaceValue(fi, correct_skewness);
  if (w)
    velocity(2) = w->getInternalFaceValue(fi, correct_skewness);

  // Return if Rhie-Chow was not requested
  if (m == Moose::FV::InterpMethod::Average)
    return velocity;

  mooseAssert(neighbor && this->hasBlocks(neighbor->subdomain_id()),
              "We should be on an internal face...");

  // Get pressure gradient. This is the uncorrected gradient plus a correction from cell centroid
  // values on either side of the face
  const VectorValue<ADReal> & grad_p = p.adGradSln(fi);

  // Get uncorrected pressure gradient. This will use the element centroid gradient if we are
  // along a boundary face
  const VectorValue<ADReal> & unc_grad_p = p.uncorrectedAdGradSln(fi);

  const Point & elem_centroid = fi.elemCentroid();
  const Point & neighbor_centroid = fi.neighborCentroid();
  Real elem_volume = fi.elemVolume();
  Real neighbor_volume = fi.neighborVolume();

  // Now we need to perform the computations of D
  const VectorValue<ADReal> & elem_a = libmesh_map_find(_a, elem->id());

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

  const VectorValue<ADReal> & neighbor_a = libmesh_map_find(_a, neighbor->id());

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

  const auto face = Moose::FV::makeCDFace(
      fi, Moose::FV::faceArgSubdomains(*this, fi), correct_skewness, correct_skewness);

  // evaluate face porosity, see (18) in Hanimann 2021 or (11) in Nordlund 2016
  const auto face_eps = epsilon(tid)(face);

  // Perform the pressure correction. We don't use skewness-correction on the pressure since
  // it only influences the averaged cell gradients which cancel out in the correction
  // below.
  for (const auto i : make_range(_dim))
    velocity(i) -= face_D(i) * face_eps * (grad_p(i) - unc_grad_p(i));

  return velocity;
}
