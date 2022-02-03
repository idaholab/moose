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
  params.addParam<bool>(
      "standard_body_forces", false, "Whether to just apply non-interpolated body forces");
  params.addParam<bool>(
      "standard_f_data",
      false,
      "Whether to just apply non-interpolated body forces with velocity, e.g. friction");
  params.addParam<bool>("add_f_to_a", true, "Whether to add F to A");
  params.addClassDescription("Performs interpolations and reconstructions of body forces and "
                             "computes face velocities.");
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
    _b(_moose_mesh, _sub_ids),
    _b2(_moose_mesh, _sub_ids),
    _f(_moose_mesh, _sub_ids),
    _f2(_moose_mesh, _sub_ids),
    _sys(*getCheckedPointerParam<SystemBase *>("_sys")),
    _example(0),
    _standard_body_forces(getParam<bool>("standard_body_forces")),
    _standard_f_data(getParam<bool>("standard_f_data")),
    _add_f_to_a(getParam<bool>("add_f_to_a")),
    _bx(_b, 0),
    _by(_b, 1),
    _bz(_b, 2),
    _b2x(_b2, 0),
    _b2y(_b2, 1),
    _b2z(_b2, 2)
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

    UserObject::_subproblem.addFunctor("bx", _bx, tid);
    UserObject::_subproblem.addFunctor("by", _by, tid);
    UserObject::_subproblem.addFunctor("bz", _bz, tid);
    UserObject::_subproblem.addFunctor("b2x", _b2x, tid);
    UserObject::_subproblem.addFunctor("b2y", _b2y, tid);
    UserObject::_subproblem.addFunctor("b2z", _b2z, tid);
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

bool
INSFVRhieChowInterpolator::isFaceGeometricallyRelevant(const FaceInfo & fi) const
{
  if (&fi.elem() == libMesh::remote_elem)
    return false;

  bool on_us = _sub_ids.count(fi.elem().subdomain_id());

  if (fi.neighborPtr())
  {
    if (&fi.neighbor() == libMesh::remote_elem)
      return false;

    on_us = on_us || _sub_ids.count(fi.neighbor().subdomain_id());
  }

  if (!on_us)
    // Neither the element nor neighbor has a subdomain id on which we are active, so this face is
    // not relevant
    return false;

  //
  // Ok, we've established that either the element or neighbor is active on our subdomains and
  // neither of them are remote elements, so this face is still in the running to be considered
  // relevant. There is one more caveat to be considered. In the case that we are a boundary face,
  // we will generally need a two term expansion to compute our value, which will require a
  // cell-gradient evaluation. If that is the case, then all of our surrounding neighbors cannot be
  // remote. If we are not a boundary face, then at this point we're safe
  //

  if (!Moose::FV::onBoundary(_sub_ids, fi))
    return true;

  const auto & boundary_elem = (fi.neighborPtr() && _sub_ids.count(fi.neighbor().subdomain_id()))
                                   ? fi.neighbor()
                                   : fi.elem();

  for (auto * const neighbor : boundary_elem.neighbor_ptr_range())
  {
    if (!neighbor)
      continue;

    if ((neighbor == libMesh::remote_elem))
      return false;
  }

  // We made it through all the tests!
  return true;
}

void
INSFVRhieChowInterpolator::insfvSetup()
{
  _elem_range =
      std::make_unique<ConstElemRange>(_mesh.active_local_subdomain_set_elements_begin(_sub_ids),
                                       _mesh.active_local_subdomain_set_elements_end(_sub_ids));

  const auto & all_fi = _moose_mesh.allFaceInfo();
  _evaluable_fi.reserve(all_fi.size());

  const auto & eq = UserObject::_subproblem.es();
  std::vector<const DofMap *> dof_maps(eq.n_systems());
  for (const auto i : make_range(eq.n_systems()))
  {
    const auto & sys = eq.get_system(i);
    dof_maps[i] = &sys.get_dof_map();
  }

  auto is_fi_evaluable = [this, &dof_maps](const FaceInfo & fi)
  {
    if (!isFaceGeometricallyRelevant(fi))
      return false;

    auto is_elem_evaluable = [&dof_maps](const Elem & elem)
    {
      for (const auto * const dof_map : dof_maps)
        if (!dof_map->is_evaluable(elem))
          return false;

      return true;
    };

    if (!Moose::FV::onBoundary(_sub_ids, fi))
      // We're on an internal face and interpolation to this face will just entail linear
      // interpolation from neighboring cell values. We just have to check whether both elements are
      // evaluable
      return is_elem_evaluable(fi.elem()) && is_elem_evaluable(fi.neighbor());

    // Else we are on a boundary face. Two-term boundary face extrapolation will require a cell
    // value and gradient, which will require evaluations on all surrounding elements

    const auto & boundary_elem = (fi.neighborPtr() && _sub_ids.count(fi.neighbor().subdomain_id()))
                                     ? fi.neighbor()
                                     : fi.elem();

    for (auto * const neighbor : boundary_elem.neighbor_ptr_range())
    {
      if (!neighbor)
        continue;

      if (!is_elem_evaluable(*neighbor))
        return false;
    }

    return true;
  };

  for (const auto & fi : all_fi)
    if (is_fi_evaluable(fi))
      _evaluable_fi.push_back(&fi);

  _evaluable_fi.shrink_to_fit();
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
  _b.clear();
  _b2.clear();
  _f.clear();
  _f2.clear();
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
INSFVRhieChowInterpolator::finalizeAData()
{
#ifdef MOOSE_GLOBAL_AD_INDEXING
  if (this->n_processors() != 1)
  {
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
        mooseAssert(pid != this->processor_id(),
                    "The request filler shouldn't have been ourselves");
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

  if (_add_f_to_a && hasFData())
    for (auto & map_pr : _a)
    {
      auto & elem_a_value = map_pr.second;
      const auto elem_id = map_pr.first;
      const auto & value_to_add = libmesh_map_find(_f2, elem_id);
      elem_a_value += value_to_add;
    }
#else
  mooseError("INSFVRhieChowInterpolator only supported for global AD indexing.");
#endif
}

void
INSFVRhieChowInterpolator::computeFirstAndSecondOverBars(MapFunctor & foo,
                                                         MapFunctor & foo2,
                                                         const bool interpolate)
{
  foo2.reserve(_fe_problem.getEvaluableElementRange().size());

  if (!interpolate)
  {
    for (const auto & pr : foo)
      foo2[pr.first] = pr.second;

    return;
  }

  Moose::FV::interpolateReconstruct(foo2, foo, 1, false, _evaluable_fi, *this);
}

void
INSFVRhieChowInterpolator::applyFooData(const bool is_b_data)
{
  const auto s = _sys.number();
  for (auto * const elem : *_elem_range)
  {
    const auto elem_volume = _assembly.elementVolume(elem);
    for (const auto i : index_range(_var_numbers))
    {
      const auto compute_residual = [this, elem, is_b_data, elem_volume, i]()
      {
        if (is_b_data)
          // Body force term that is not a function of velocity
          // negative here because we swapped the sign in addToB and so now we need to swap it back
          return -elem_volume * libmesh_map_find(_b2, elem->id())(i);
        else
        {
          // Body force term that *is* a function of velocity, e.g. friction
          INSFVVelocityVariable * vel_comp = nullptr;
          switch (i)
          {
            case 0:
              vel_comp = _u;
              break;
            case 1:
              vel_comp = _v;
              break;
            case 2:
              vel_comp = _w;
              break;
            default:
              mooseError("Invalid component");
          }

          mooseAssert(vel_comp, "This should be non-null now");
          return libmesh_map_find(_f2, elem->id())(i) * (*vel_comp)(makeElemArg(elem));
        }
      };
      const auto residual = compute_residual();

      const auto vn = _var_numbers[i];
      const auto dof_index = elem->dof_number(s, vn, 0);

      if (_fe_problem.currentlyComputingJacobian())
        _assembly.processDerivatives(residual, dof_index, _matrix_tags);
      else
        _assembly.processResidual(residual.value(), dof_index, _vector_tags);
    }
  }

  if (_fe_problem.currentlyComputingJacobian())
    _assembly.addCachedJacobian();
  else
    _assembly.addCachedResiduals();
}

void
INSFVRhieChowInterpolator::finalizeInterpolatedData(MapFunctor & foo,
                                                    MapFunctor & foo2,
                                                    const std::set<SubdomainID> & foo_sub_ids,
                                                    const bool interpolate)
{
#ifdef MOOSE_GLOBAL_AD_INDEXING
  // First thing we do is to fill our _b map with 0's for any subdomains that do not have body force
  // kernels active
  std::set<SubdomainID> forceless_subs;
  std::set_difference(_sub_ids.begin(),
                      _sub_ids.end(),
                      foo_sub_ids.begin(),
                      foo_sub_ids.end(),
                      std::inserter(forceless_subs, forceless_subs.end()));
  for (const auto & elem : _mesh.active_local_subdomain_set_elements_ptr_range(forceless_subs))
    foo[elem->id()] = 0;

  if (this->n_processors() > 1)
  {
    // We do not have to push foo data because all that data should initially be
    // local, e.g. we only loop over active local elements for FVElementalKernels

    std::unordered_map<processor_id_type, std::vector<dof_id_type>> pull_requests;

    for (auto * const elem : _fe_problem.getEvaluableElementRange())
    {
      const auto pid = elem->processor_id();
      if (pid != this->processor_id() && _sub_ids.count(elem->subdomain_id()))
        pull_requests[pid].push_back(elem->id());
    }

    auto gather_functor = [this, &foo](const processor_id_type libmesh_dbg_var(pid),
                                       const std::vector<dof_id_type> & elem_ids,
                                       std::vector<VectorValue<ADReal>> & data_to_fill)
    {
      mooseAssert(pid != this->processor_id(), "We shouldn't be gathering from ourselves.");
      data_to_fill.resize(elem_ids.size());
      for (const auto i : index_range(elem_ids))
      {
        const auto id = elem_ids[i];
        data_to_fill[i] = libmesh_map_find(foo, id);
      }
    };

    auto action_functor = [this, &foo](const processor_id_type libmesh_dbg_var(pid),
                                       const std::vector<dof_id_type> & elem_ids,
                                       const std::vector<VectorValue<ADReal>> & filled_data)
    {
      mooseAssert(pid != this->processor_id(), "The request filler shouldn't have been ourselves");
      mooseAssert(elem_ids.size() == filled_data.size(), "I think these should be the same size");
      for (const auto i : index_range(elem_ids))
      {
        const auto id = elem_ids[i];
        foo[id] = filled_data[i];
      }
    };
    TIMPI::pull_parallel_vector_data(
        _communicator, pull_requests, gather_functor, action_functor, &_example);
  }

  // We can proceed to the overbar operations for foo. We only do the first and second overbars. The
  // third overbar is done on the fly when it is requested
  computeFirstAndSecondOverBars(foo, foo2, interpolate);
#else
  mooseError("INSFVRhieChowInterpolator only supported for global AD indexing.");
#endif
}

void
INSFVRhieChowInterpolator::finalize()
{
  if (hasBodyForces())
  {
    finalizeInterpolatedData(_b, _b2, _sub_ids_with_body_forces, !_standard_body_forces);
    // Add the b data to the residual/Jacobian
    applyFooData(true);
  }

  if (hasFData())
  {
    finalizeInterpolatedData(_f, _f2, _sub_ids_with_f_data, !_standard_f_data);
    // Add the f data to the residual/Jacobian after multiplying by the velocity variable value
    applyFooData(false);
  }

  // Must occur after we've interpolated F because we are going to add F into A
  finalizeAData();
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
  const auto b1 = hasBodyForces() ? _b(face) : VectorValue<ADReal>(0);
  const auto b3 = hasBodyForces() ? _b2(face) : VectorValue<ADReal>(0);

  // Perform the pressure correction. We don't use skewness-correction on the pressure since
  // it only influences the averaged cell gradients which cancel out in the correction
  // below.
  for (const auto i : make_range(_dim))
  {
    velocity(i) -= face_D(i) * face_eps * (grad_p(i) - unc_grad_p(i));
    if (hasBodyForces())
      velocity(i) += face_D(i) * (b1(i) - b3(i));
  }

  return velocity;
}

void
INSFVRhieChowInterpolator::hasBodyForces(const std::set<SubdomainID> & sub_ids)
{
  _sub_ids_with_body_forces.insert(sub_ids.begin(), sub_ids.end());
}

void
INSFVRhieChowInterpolator::hasFData(const std::set<SubdomainID> & sub_ids)
{
  _sub_ids_with_f_data.insert(sub_ids.begin(), sub_ids.end());
}
