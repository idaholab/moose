//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ParallelUniqueId.h"
#include "MooseMesh.h"
#include "MooseTypes.h"
#include "MooseException.h"
#include "MooseVariableFieldBase.h"
#include "FVFluxKernel.h"
#include "FVFluxBC.h"
#include "FVInterfaceKernel.h"
#include "FEProblem.h"
#include "SwapBackSentinel.h"
#include "MaterialBase.h"
#include "libmesh/libmesh_exceptions.h"
#include "libmesh/elem.h"

#include <set>

class MooseVariableFVBase;

/// This works like the Sentinel helper classes in MOOSE, except it is more
/// flexible and concise to use.  You just initialize it with a lambda and can
/// return it from within another function scope.  So a function that has other
/// cleanup functions associated with it can just wrap those cleanup funcs in
/// an OnScopeExit object and return it.  The caller of the function needing
/// cleanup simply catches its return value and voila - the cleanup function
/// are called at the end of the calling functions scope. Like this:
///
///   OnScopeExit doSomethingSpecial()
///   {
///     ...
///     return OnScopeExit([]{cleanupSomethingSpecial();});
///   }
///   void cleanupSomethingSpecial() {...}
///
///   void main()
///   {
///     auto cleaner_uper = doSomethingSpecial();
///   }
struct OnScopeExit
{
  std::function<void()> _f;
  OnScopeExit(std::function<void()> f) noexcept : _f(std::move(f)) {}
  OnScopeExit(OnScopeExit && other) : _f(std::move(other._f)) {}
  OnScopeExit & operator=(OnScopeExit && other)
  {
    _f = std::move(other._f);
    return *this;
  }
  ~OnScopeExit()
  {
    if (_f)
      _f();
  }
};

/**
 * This loops over a set of mesh faces (i.e. FaceInfo objects).  Callback
 * routines are provided for visiting each face, for visiting boundary faces,
 * for sudomain changes, and pre/post many of these events.
 */
template <typename RangeType>
class ThreadedFaceLoop
{
public:
  ThreadedFaceLoop(FEProblemBase & fe_problem,
                   const unsigned int nl_system_num,
                   const std::set<TagID> & tags);

  ThreadedFaceLoop(ThreadedFaceLoop & x, Threads::split split);

  virtual ~ThreadedFaceLoop();

  virtual void operator()(const RangeType & range, bool bypass_threading = false);

  void join(const ThreadedFaceLoop & y);

  virtual void onFace(const FaceInfo & fi) = 0;
  /// This is called once for each face after all face and boundary callbacks have been
  /// finished for that face.
  virtual void postFace(const FaceInfo & /*fi*/) {}
  /// This is called once before all face-looping
  virtual void pre() {}
  /// This is called once after all face-looping is finished.
  virtual void post() {}

  /// This is called once for every face that is on a boundary *after* onFace
  /// is called for the face.
  virtual void onBoundary(const FaceInfo & fi, BoundaryID boundary) = 0;

  /// Called every time the current subdomain changes (i.e. the subdomain of *this* face's elem element
  /// is not the same as the subdomain of the last face's elem element).
  virtual void subdomainChanged() { _fe_problem.subdomainSetup(_subdomain, _tid); }

  /// Called every time the neighbor subdomain changes (i.e. the subdomain of *this* face's neighbor element
  /// is not the same as the subdomain of the last face's neighbor element).
  virtual void neighborSubdomainChanged()
  {
    _fe_problem.neighborSubdomainSetup(_neighbor_subdomain, _tid);
  }

  /// Called if a MooseException is caught anywhere during the computation.
  virtual void caughtMooseException(MooseException &) {}

protected:
  /// Print list of object types executed and in which order
  virtual void printGeneralExecutionInformation() const {}

  /// Print ordering of objects executed on each block
  virtual void printBlockExecutionInformation() const {}

  /// Print ordering of objects exected on each boundary
  virtual void printBoundaryExecutionInformation(const BoundaryID /* bnd_id */) const {}

  /// Reset lists of blocks and boundaries for which execution printing has been done
  void resetExecutionPrinting()
  {
    _blocks_exec_printed.clear();
    _boundaries_exec_printed.clear();
  }

  FEProblemBase & _fe_problem;
  MooseMesh & _mesh;
  const std::set<TagID> & _tags;
  THREAD_ID _tid;
  const unsigned int _nl_system_num;

  /// The subdomain for the current element
  SubdomainID _subdomain;

  /// The subdomain for the last element
  SubdomainID _old_subdomain;

  /// The subdomain for the current neighbor
  SubdomainID _neighbor_subdomain;

  /// The subdomain for the last neighbor
  SubdomainID _old_neighbor_subdomain;

  /// Set to keep track of blocks for which we have printed the execution pattern
  mutable std::set<std::pair<const SubdomainID, const SubdomainID>> _blocks_exec_printed;

  /// Set to keep track of boundaries for which we have printed the execution pattern
  mutable std::set<BoundaryID> _boundaries_exec_printed;

  /// Holds caught runtime error messages
  std::string _error_message;

private:
  /// Whether this is the zeroth threaded copy of this body
  const bool _zeroth_copy;

  /// The value of \p Moose::_throw_on_error at the time of construction. This data member only has
  /// meaning and will only be read if this is the thread 0 copy of the class
  const bool _incoming_throw_on_error;
};

template <typename RangeType>
ThreadedFaceLoop<RangeType>::ThreadedFaceLoop(FEProblemBase & fe_problem,
                                              const unsigned int nl_system_num,
                                              const std::set<TagID> & tags)
  : _fe_problem(fe_problem),
    _mesh(fe_problem.mesh()),
    _tags(tags),
    _nl_system_num(nl_system_num),
    _zeroth_copy(true),
    _incoming_throw_on_error(Moose::_throw_on_error)
{
  Moose::_throw_on_error = true;
}

template <typename RangeType>
ThreadedFaceLoop<RangeType>::ThreadedFaceLoop(ThreadedFaceLoop & x, Threads::split /*split*/)
  : _fe_problem(x._fe_problem),
    _mesh(x._mesh),
    _tags(x._tags),
    _nl_system_num(x._nl_system_num),
    _zeroth_copy(false),
    _incoming_throw_on_error(false)
{
}

template <typename RangeType>
ThreadedFaceLoop<RangeType>::~ThreadedFaceLoop()
{
  if (_zeroth_copy)
  {
    Moose::_throw_on_error = _incoming_throw_on_error;

    if (!_error_message.empty())
      mooseError(_error_message);
  }
}

template <typename RangeType>
void
ThreadedFaceLoop<RangeType>::join(const ThreadedFaceLoop<RangeType> & y)
{
  if (_error_message.empty() && !y._error_message.empty())
    _error_message = y._error_message;
}

// TODO: ensure the vector<faceinfo> data structure needs to be built such
// that for all sides on an interface between two subdomains, the elements of
// the same subdomain are used consistently for all the "elem" (i.e. not
// "neighbor") parameters in order to avoid jumping back and forth along the
// boundary between using one or the other subdomains' FV kernels
// unpredictably.
template <typename RangeType>
void
ThreadedFaceLoop<RangeType>::operator()(const RangeType & range, bool bypass_threading)
{
  // TODO: make this query fv flux kernel specific or somehow integrate the
  // fv source kernels into this loop. Also this check will need to increase
  // in generality if/when other systems and objects besides FV stuff get
  // added to this loop.
  std::vector<FVKernel *> kernels;
  _fe_problem.theWarehouse()
      .query()
      .template condition<AttribSysNum>(_nl_system_num)
      .template condition<AttribSystem>("FVFluxKernel")
      .queryInto(kernels);
  if (kernels.size() == 0)
    return;

  try
  {
    try
    {
      ParallelUniqueId puid;
      _tid = bypass_threading ? 0 : puid.id;

      pre();
      printGeneralExecutionInformation();

      _subdomain = Moose::INVALID_BLOCK_ID;
      _neighbor_subdomain = Moose::INVALID_BLOCK_ID;

      typename RangeType::const_iterator faceinfo = range.begin();
      for (faceinfo = range.begin(); faceinfo != range.end(); ++faceinfo)
      {
        const Elem & elem = (*faceinfo)->elem();

        _fe_problem.setCurrentSubdomainID(&elem, _tid);

        _old_subdomain = _subdomain;
        _subdomain = elem.subdomain_id();
        if (_subdomain != _old_subdomain)
        {
          subdomainChanged();
          printBlockExecutionInformation();
        }

        _old_neighbor_subdomain = _neighbor_subdomain;
        if (const Elem * const neighbor = (*faceinfo)->neighborPtr())
        {
          _fe_problem.setNeighborSubdomainID(neighbor, _tid);
          _neighbor_subdomain = neighbor->subdomain_id();
        }
        else
          _neighbor_subdomain = Moose::INVALID_BLOCK_ID;

        if (_neighbor_subdomain != _old_neighbor_subdomain)
        {
          neighborSubdomainChanged();
          // This is going to cause a lot more printing
          printBlockExecutionInformation();
        }

        onFace(**faceinfo);
        // Cache data now because onBoundary may clear it. E.g. there was a nasty bug for two
        // variable FV systems where if one variable was executing an FVFluxKernel on a boundary
        // while the other was executing an FVFluxBC, the FVFluxKernel data would get lost because
        // onBoundary would clear the residual/Jacobian data before it was cached
        postFace(**faceinfo);

        const std::set<BoundaryID> boundary_ids = (*faceinfo)->boundaryIDs();
        for (auto & it : boundary_ids)
        {
          printBoundaryExecutionInformation(it);
          onBoundary(**faceinfo, it);
        }

        postFace(**faceinfo);

      } // range
      post();

      // Clear execution printing sets to start printing on every block and boundary again
      resetExecutionPrinting();
    }
    catch (libMesh::LogicError & e)
    {
      mooseException("We caught a libMesh error: ", e.what());
    }
    catch (MetaPhysicL::LogicError & e)
    {
      moose::translateMetaPhysicLError(e);
    }
  }
  catch (MooseException & e)
  {
    caughtMooseException(e);
  }
  catch (std::runtime_error & e)
  {
    _error_message = e.what();
  }
}

/**
 * Base class for assembly-like calculations.
 */
template <typename RangeType, typename AttributeTagType>
class ComputeFVFluxThread : public ThreadedFaceLoop<RangeType>
{
public:
  ComputeFVFluxThread(FEProblemBase & fe_problem,
                      const unsigned int nl_system_num,
                      const std::set<TagID> & tags);

  ComputeFVFluxThread(ComputeFVFluxThread & x, Threads::split split);

  virtual ~ComputeFVFluxThread();

  virtual void onFace(const FaceInfo & fi) override;
  virtual void pre() override;
  virtual void post() override;

  virtual void onBoundary(const FaceInfo & fi, BoundaryID boundary) override;

  virtual void subdomainChanged() override;
  virtual void neighborSubdomainChanged() override;

protected:
  /**
   * call either computeResidual, computeJacobian, or computeResidualAndJacobian on the provided
   * residual object depending on what derived class of this class is instantiated
   */
  virtual void compute(FVFaceResidualObject & ro, const FaceInfo & fi) = 0;

  /**
   * call either residualSetup or jacobianSetup depending on what derived class of this class is
   * instantiated
   */
  virtual void setup(SetupInterface & obj) = 0;

  /**
   * call either addCachedJacobian or addCachedResidual or both  depending on what derived class of
   * this class is instantiated
   */
  virtual void addCached() = 0;

  unsigned int _num_cached = 0;

  using ThreadedFaceLoop<RangeType>::_fe_problem;
  using ThreadedFaceLoop<RangeType>::_mesh;
  using ThreadedFaceLoop<RangeType>::_tid;
  using ThreadedFaceLoop<RangeType>::_tags;
  using ThreadedFaceLoop<RangeType>::_nl_system_num;
  using ThreadedFaceLoop<RangeType>::_subdomain;
  using ThreadedFaceLoop<RangeType>::_neighbor_subdomain;
  using ThreadedFaceLoop<RangeType>::_blocks_exec_printed;
  using ThreadedFaceLoop<RangeType>::_boundaries_exec_printed;

private:
  void reinitVariables(const FaceInfo & fi);
  void checkPropDeps(const std::vector<std::shared_ptr<MaterialBase>> & mats) const;
  void finalizeContainers();
  static void emptyDifferenceTest(const std::set<unsigned int> & requested,
                                  const std::set<unsigned int> & supplied,
                                  std::set<unsigned int> & difference);

  /// Print list of object types executed and in which order
  virtual void printGeneralExecutionInformation() const override;

  /// Print ordering of objects executed on each block
  virtual void printBlockExecutionInformation() const override;

  /// Print ordering of objects exected on each boundary
  virtual void printBoundaryExecutionInformation(const BoundaryID bnd_id) const override;

  /// Utility to get the subdomain names from the ids
  std::pair<SubdomainName, SubdomainName> getBlockNames() const;

  /// Variables
  std::set<MooseVariableFieldBase *> _fv_vars;
  std::set<MooseVariableFieldBase *> _elem_sub_fv_vars;
  std::set<MooseVariableFieldBase *> _neigh_sub_fv_vars;

  /// FVFluxKernels
  std::set<FVFluxKernel *> _fv_flux_kernels;
  std::set<FVFluxKernel *> _elem_sub_fv_flux_kernels;
  std::set<FVFluxKernel *> _neigh_sub_fv_flux_kernels;

  /// Element face materials
  std::vector<std::shared_ptr<MaterialBase>> _elem_face_mats;
  std::vector<std::shared_ptr<MaterialBase>> _elem_sub_elem_face_mats;
  std::vector<std::shared_ptr<MaterialBase>> _neigh_sub_elem_face_mats;

  // Neighbor face materials
  std::vector<std::shared_ptr<MaterialBase>> _neigh_face_mats;
  std::vector<std::shared_ptr<MaterialBase>> _elem_sub_neigh_face_mats;
  std::vector<std::shared_ptr<MaterialBase>> _neigh_sub_neigh_face_mats;

  const bool _scaling_jacobian;
  const bool _scaling_residual;
};

template <typename RangeType, typename AttributeTagType>
ComputeFVFluxThread<RangeType, AttributeTagType>::ComputeFVFluxThread(FEProblemBase & fe_problem,
                                                                      unsigned int nl_system_num,
                                                                      const std::set<TagID> & tags)
  : ThreadedFaceLoop<RangeType>(fe_problem, nl_system_num, tags),
    _scaling_jacobian(fe_problem.computingScalingJacobian()),
    _scaling_residual(fe_problem.computingScalingResidual())
{
}

template <typename RangeType, typename AttributeTagType>
ComputeFVFluxThread<RangeType, AttributeTagType>::ComputeFVFluxThread(ComputeFVFluxThread & x,
                                                                      Threads::split split)
  : ThreadedFaceLoop<RangeType>(x, split),
    _fv_vars(x._fv_vars),
    _scaling_jacobian(x._scaling_jacobian),
    _scaling_residual(x._scaling_residual)
{
}

template <typename RangeType, typename AttributeTagType>
ComputeFVFluxThread<RangeType, AttributeTagType>::~ComputeFVFluxThread()
{
}

template <typename RangeType, typename AttributeTagType>
void
ComputeFVFluxThread<RangeType, AttributeTagType>::reinitVariables(const FaceInfo & fi)
{
  // TODO: this skips necessary FE reinit.  In addition to this call, we need
  // to conditionally do some FE-specific reinit here if we have any active FE
  // variables.  However, we still want to keep/do FV-style quadrature.
  // Figure out how to do all this some day.
  _fe_problem.reinitFVFace(_tid, fi);

  // TODO: for FE variables, this is handled via setting needed vars through
  // fe problem API which passes the value on to the system class.  Then
  // reinit is called on fe problem which forwards its calls to the system
  // function and then fe problem also calls displaced problem reinit.  All
  // the call forwarding seems silly, but it does allow the displaced problem
  // to be easily kept in sync.  However, the displaced problem has different
  // pointers for its own face info objects, etc, we can't just pass in fe
  // problem's face info down to the sub-problem -- centroids are different,
  // volumes are different, etc.  To support displaced meshes correctly, we
  // need to be able to reinit the subproblems variables using its equivalent
  // face info object.  How?  See https://github.com/idaholab/moose/issues/15064

  for (auto var : _fv_vars)
    var->computeFaceValues(fi);

  _fe_problem.resizeMaterialData(Moose::MaterialDataType::FACE_MATERIAL_DATA, /*nqp=*/1, _tid);

  for (std::shared_ptr<MaterialBase> mat : _elem_face_mats)
  {
    mat->setFaceInfo(fi);
    mat->computeProperties();
  }

  if (fi.neighborPtr())
  {
    _fe_problem.resizeMaterialData(
        Moose::MaterialDataType::NEIGHBOR_MATERIAL_DATA, /*nqp=*/1, _tid);

    for (std::shared_ptr<MaterialBase> mat : _neigh_face_mats)
    {
      mat->setFaceInfo(fi);
      mat->computeProperties();
    }
  }
}

template <typename RangeType, typename AttributeTagType>
void
ComputeFVFluxThread<RangeType, AttributeTagType>::onFace(const FaceInfo & fi)
{
  reinitVariables(fi);

  for (auto * const k : _fv_flux_kernels)
    compute(*k, fi);
}

template <typename RangeType, typename AttributeTagType>
void
ComputeFVFluxThread<RangeType, AttributeTagType>::onBoundary(const FaceInfo & fi, BoundaryID bnd_id)
{
  if (_scaling_jacobian || _scaling_residual)
    return;

  std::vector<FVFluxBC *> bcs;
  _fe_problem.theWarehouse()
      .query()
      .template condition<AttribSysNum>(_nl_system_num)
      .template condition<AttribSystem>("FVFluxBC")
      .template condition<AttribThread>(_tid)
      .template condition<AttributeTagType>(_tags)
      .template condition<AttribBoundaries>(bnd_id)
      .queryInto(bcs);

  for (auto * const bc : bcs)
    compute(*bc, fi);

  std::vector<FVInterfaceKernel *> iks;
  _fe_problem.theWarehouse()
      .query()
      .template condition<AttribSysNum>(_nl_system_num)
      .template condition<AttribSystem>("FVInterfaceKernel")
      .template condition<AttribThread>(_tid)
      .template condition<AttributeTagType>(_tags)
      .template condition<AttribBoundaries>(bnd_id)
      .queryInto(iks);

  for (auto * const ik : iks)
    compute(*ik, fi);
}

template <typename RangeType, typename AttributeTagType>
void
ComputeFVFluxThread<RangeType, AttributeTagType>::post()
{
  // make sure we add any remaining cached residuals/jacobians to add/record
  Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
  addCached();

  _fe_problem.clearActiveElementalMooseVariables(_tid);
  _fe_problem.clearActiveMaterialProperties(_tid);
}

template <typename RangeType, typename AttributeTagType>
void
ComputeFVFluxThread<RangeType, AttributeTagType>::emptyDifferenceTest(
    const std::set<unsigned int> & requested,
    const std::set<unsigned int> & supplied,
    std::set<unsigned int> & difference)
{
  std::set_difference(requested.begin(),
                      requested.end(),
                      supplied.begin(),
                      supplied.end(),
                      std::inserter(difference, difference.begin()));

  mooseAssert(
      difference.empty(),
      "All of the material properties we depend on should already be supplied/computed. Do your FV "
      "objects depend on a material property that is computed in a material with coupled FE "
      "variables? If so, that property needs to be moved to a material without FE coupling.");
}

template <typename RangeType, typename AttributeTagType>
void
ComputeFVFluxThread<RangeType, AttributeTagType>::checkPropDeps(
    const std::vector<std::shared_ptr<MaterialBase>> & libmesh_dbg_var(mats)) const
{
#ifndef NDEBUG
  std::set<unsigned int> props_diff;
  std::set<unsigned int> supplied_props;
  std::set<unsigned int> fv_kernel_requested_props;

  for (auto * kernel : _fv_flux_kernels)
  {
    const auto & mp_deps = kernel->getMatPropDependencies();
    fv_kernel_requested_props.insert(mp_deps.begin(), mp_deps.end());
  }

  std::set<std::string> same_matprop_name;
  for (std::shared_ptr<MaterialBase> mat : mats)
  {
    for (const auto prop_id : mat->getSuppliedPropIDs())
    {
      auto pr = supplied_props.insert(prop_id);
      if (!pr.second)
      {
        const auto & prop_ids = MaterialPropertyStorage::propIDs();
        auto same_matprop_name_it =
            std::find_if(prop_ids.begin(),
                         prop_ids.end(),
                         [prop_id](const std::pair<std::string, unsigned int> & map_pr)
                         { return map_pr.second == prop_id; });
        same_matprop_name.insert(same_matprop_name_it->first);
      }
    }

    const auto & mp_deps = mat->getMatPropDependencies();
    emptyDifferenceTest(mp_deps, supplied_props, props_diff);
  }

  // Print a warning if block restricted materials are used
  auto same_matprop_name_str = MooseUtils::join(same_matprop_name, " ");

  if (same_matprop_name.size() > 0)
    mooseDoOnce(
        mooseWarning("Multiple objects supply properties of name ",
                     same_matprop_name_str,
                     ".\nDo you have different block-restricted physics *and* different "
                     "block-restricted \nmaterials "
                     "on either side of an interface that define the same "
                     "property name? \nUnfortunately that is not supported in FV because we have "
                     "to allow ghosting \nof material properties for block-restricted physics."));

  emptyDifferenceTest(fv_kernel_requested_props, supplied_props, props_diff);
#endif
}

template <typename RangeType, typename AttributeTagType>
void
ComputeFVFluxThread<RangeType, AttributeTagType>::finalizeContainers()
{
  //
  // Finalize our variables
  //
  std::set_union(_elem_sub_fv_vars.begin(),
                 _elem_sub_fv_vars.end(),
                 _neigh_sub_fv_vars.begin(),
                 _neigh_sub_fv_vars.end(),
                 std::inserter(_fv_vars, _fv_vars.begin()));

  //
  // Finalize our kernels
  //
  const bool same_kernels = _elem_sub_fv_flux_kernels == _neigh_sub_fv_flux_kernels;
  if (same_kernels)
    _fv_flux_kernels = _elem_sub_fv_flux_kernels;
  else
    std::set_union(_elem_sub_fv_flux_kernels.begin(),
                   _elem_sub_fv_flux_kernels.end(),
                   _neigh_sub_fv_flux_kernels.begin(),
                   _neigh_sub_fv_flux_kernels.end(),
                   std::inserter(_fv_flux_kernels, _fv_flux_kernels.begin()));
  const bool need_ghosting = !same_kernels;

  //
  // Finalize our element face materials
  //
  _elem_face_mats = _elem_sub_elem_face_mats;

  if (need_ghosting)
    // Add any element face materials from the neighboring subdomain that do not exist on the
    // element subdomain
    for (std::shared_ptr<MaterialBase> neigh_sub_elem_face_mat : _neigh_sub_elem_face_mats)
      if (std::find(_elem_sub_elem_face_mats.begin(),
                    _elem_sub_elem_face_mats.end(),
                    neigh_sub_elem_face_mat) == _elem_sub_elem_face_mats.end())
        _elem_face_mats.push_back(neigh_sub_elem_face_mat);

  //
  // Finalize our neighbor face materials
  //
  _neigh_face_mats = _neigh_sub_neigh_face_mats;

  if (need_ghosting)
    // Add any neighbor face materials from the element subdomain that do not exist on the
    // neighbor subdomain
    for (std::shared_ptr<MaterialBase> elem_sub_neigh_face_mat : _elem_sub_neigh_face_mats)
      if (std::find(_neigh_sub_neigh_face_mats.begin(),
                    _neigh_sub_neigh_face_mats.end(),
                    elem_sub_neigh_face_mat) == _neigh_sub_neigh_face_mats.end())
        _neigh_face_mats.push_back(elem_sub_neigh_face_mat);

  //
  // Check satisfaction of material property dependencies
  //
  checkPropDeps(_elem_face_mats);
  checkPropDeps(_neigh_face_mats);
}

template <typename RangeType, typename AttributeTagType>
void
ComputeFVFluxThread<RangeType, AttributeTagType>::subdomainChanged()
{
  ThreadedFaceLoop<RangeType>::subdomainChanged();

  // Clear variables
  _fv_vars.clear();
  _elem_sub_fv_vars.clear();

  // Clear kernels
  _fv_flux_kernels.clear();
  _elem_sub_fv_flux_kernels.clear();

  // Clear element face materials
  _elem_face_mats.clear();
  _elem_sub_elem_face_mats.clear();

  // Clear neighbor face materials
  _neigh_face_mats.clear();
  _elem_sub_neigh_face_mats.clear();

  // TODO: do this for other relevant objects - like FV BCs, FV source term
  // kernels, etc. - but we don't need to add them for other types of objects
  // like FE or DG kernels because those kernels don't run in this loop. Do we
  // really want to integrate fv source kernels into this loop?
  std::vector<FVFluxKernel *> kernels;
  _fe_problem.theWarehouse()
      .query()
      .template condition<AttribSysNum>(_nl_system_num)
      .template condition<AttribSystem>("FVFluxKernel")
      .template condition<AttribSubdomains>(_subdomain)
      .template condition<AttribThread>(_tid)
      .template condition<AttributeTagType>(_tags)
      .queryInto(kernels);

  _elem_sub_fv_flux_kernels = std::set<FVFluxKernel *>(kernels.begin(), kernels.end());

  for (auto * k : _elem_sub_fv_flux_kernels)
  {
    // TODO: we need a better way to do this - especially when FE objects begin to
    // couple to FV vars.  This code shoud be refactored out into one place
    // where it is easy for users to say initialize all materials and
    // variables needed by these objects for me.
    const auto & deps = k->getMooseVariableDependencies();
    for (auto var : deps)
    {
      mooseAssert(var->isFV(),
                  "We do not currently support coupling of FE variables into FV objects");
      _elem_sub_fv_vars.insert(var);
    }
  }

  _fe_problem.getFVMatsAndDependencies(
      _subdomain, _elem_sub_elem_face_mats, _elem_sub_neigh_face_mats, _elem_sub_fv_vars, _tid);

  finalizeContainers();
}

template <typename RangeType, typename AttributeTagType>
void
ComputeFVFluxThread<RangeType, AttributeTagType>::neighborSubdomainChanged()
{
  ThreadedFaceLoop<RangeType>::neighborSubdomainChanged();

  // Clear variables
  _fv_vars.clear();
  _neigh_sub_fv_vars.clear();

  // Clear kernels
  _fv_flux_kernels.clear();
  _neigh_sub_fv_flux_kernels.clear();

  // Clear element face materials
  _elem_face_mats.clear();
  _neigh_sub_elem_face_mats.clear();

  // Clear neighbor face materials
  _neigh_face_mats.clear();
  _neigh_sub_neigh_face_mats.clear();

  // TODO: do this for other relevant objects - like FV BCs, FV source term
  // kernels, etc. - but we don't need to add them for other types of objects
  // like FE or DG kernels because those kernels don't run in this loop. Do we
  // really want to integrate fv source kernels into this loop?
  std::vector<FVFluxKernel *> kernels;
  _fe_problem.theWarehouse()
      .query()
      .template condition<AttribSysNum>(_nl_system_num)
      .template condition<AttribSystem>("FVFluxKernel")
      .template condition<AttribSubdomains>(_neighbor_subdomain)
      .template condition<AttribThread>(_tid)
      .template condition<AttributeTagType>(_tags)
      .queryInto(kernels);

  _neigh_sub_fv_flux_kernels = std::set<FVFluxKernel *>(kernels.begin(), kernels.end());

  for (auto * k : _neigh_sub_fv_flux_kernels)
  {
    // TODO: we need a better way to do this - especially when FE objects begin to
    // couple to FV vars.  This code shoud be refactored out into one place
    // where it is easy for users to say initialize all materials and
    // variables needed by these objects for me.
    const auto & deps = k->getMooseVariableDependencies();
    for (auto var : deps)
    {
      mooseAssert(var->isFV(),
                  "We do not currently support coupling of FE variables into FV objects");
      _neigh_sub_fv_vars.insert(var);
    }
  }

  _fe_problem.getFVMatsAndDependencies(_neighbor_subdomain,
                                       _neigh_sub_elem_face_mats,
                                       _neigh_sub_neigh_face_mats,
                                       _neigh_sub_fv_vars,
                                       _tid);

  finalizeContainers();
}

template <typename RangeType, typename AttributeTagType>
void
ComputeFVFluxThread<RangeType, AttributeTagType>::pre()
{
  std::vector<FVFluxBC *> bcs;
  _fe_problem.theWarehouse()
      .query()
      .template condition<AttribSysNum>(_nl_system_num)
      .template condition<AttribSystem>("FVFluxBC")
      .template condition<AttribThread>(_tid)
      .template condition<AttributeTagType>(_tags)
      .queryInto(bcs);

  std::vector<FVInterfaceKernel *> iks;
  _fe_problem.theWarehouse()
      .query()
      .template condition<AttribSysNum>(_nl_system_num)
      .template condition<AttribSystem>("FVInterfaceKernel")
      .template condition<AttribThread>(_tid)
      .template condition<AttributeTagType>(_tags)
      .queryInto(iks);

  std::vector<FVFluxKernel *> kernels;
  _fe_problem.theWarehouse()
      .query()
      .template condition<AttribSysNum>(_nl_system_num)
      .template condition<AttribSystem>("FVFluxKernel")
      .template condition<AttribThread>(_tid)
      .template condition<AttributeTagType>(_tags)
      .queryInto(kernels);

  for (auto * const bc : bcs)
    setup(*bc);
  for (auto * const ik : iks)
    setup(*ik);
  for (auto * const kernel : kernels)
    setup(*kernel);

  // Clear variables
  _fv_vars.clear();
  _elem_sub_fv_vars.clear();
  _neigh_sub_fv_vars.clear();

  // Clear kernels
  _fv_flux_kernels.clear();
  _elem_sub_fv_flux_kernels.clear();
  _neigh_sub_fv_flux_kernels.clear();

  // Clear element face materials
  _elem_face_mats.clear();
  _elem_sub_elem_face_mats.clear();
  _neigh_sub_elem_face_mats.clear();

  // Clear neighbor face materials
  _neigh_face_mats.clear();
  _elem_sub_neigh_face_mats.clear();
  _neigh_sub_neigh_face_mats.clear();
}

template <typename RangeType>
class ComputeFVFluxResidualThread : public ComputeFVFluxThread<RangeType, AttribVectorTags>
{
public:
  ComputeFVFluxResidualThread(FEProblemBase & fe_problem,
                              const unsigned int nl_system_num,
                              const std::set<TagID> & tags);

  ComputeFVFluxResidualThread(ComputeFVFluxResidualThread & x, Threads::split split);

protected:
  using ComputeFVFluxThread<RangeType, AttribVectorTags>::_fe_problem;
  using ComputeFVFluxThread<RangeType, AttribVectorTags>::_tid;
  using ComputeFVFluxThread<RangeType, AttribVectorTags>::_nl_system_num;
  using ComputeFVFluxThread<RangeType, AttribVectorTags>::_num_cached;

  void postFace(const FaceInfo & fi) override;
  void compute(FVFaceResidualObject & ro, const FaceInfo & fi) override { ro.computeResidual(fi); }
  void setup(SetupInterface & obj) override { obj.residualSetup(); }
  void addCached() override { _fe_problem.addCachedResidual(_tid); }
};

template <typename RangeType>
ComputeFVFluxResidualThread<RangeType>::ComputeFVFluxResidualThread(
    FEProblemBase & fe_problem, const unsigned int nl_system_num, const std::set<TagID> & tags)
  : ComputeFVFluxThread<RangeType, AttribVectorTags>(fe_problem, nl_system_num, tags)
{
}

template <typename RangeType>
ComputeFVFluxResidualThread<RangeType>::ComputeFVFluxResidualThread(ComputeFVFluxResidualThread & x,
                                                                    Threads::split split)
  : ComputeFVFluxThread<RangeType, AttribVectorTags>(x, split)
{
}

template <typename RangeType>
void
ComputeFVFluxResidualThread<RangeType>::postFace(const FaceInfo & /*fi*/)
{
  _num_cached++;
  // TODO: do we need both calls - or just the neighbor one? - confirm this
  _fe_problem.cacheResidual(_tid);
  _fe_problem.cacheResidualNeighbor(_tid);

  _fe_problem.addCachedResidual(_tid);
}

template <typename RangeType>
class ComputeFVFluxJacobianThread : public ComputeFVFluxThread<RangeType, AttribMatrixTags>
{
public:
  ComputeFVFluxJacobianThread(FEProblemBase & fe_problem,
                              const unsigned int nl_system_num,
                              const std::set<TagID> & tags);

  ComputeFVFluxJacobianThread(ComputeFVFluxJacobianThread & x, Threads::split split);

protected:
  using ComputeFVFluxThread<RangeType, AttribMatrixTags>::_fe_problem;
  using ComputeFVFluxThread<RangeType, AttribMatrixTags>::_tid;
  using ComputeFVFluxThread<RangeType, AttribMatrixTags>::_num_cached;

  void postFace(const FaceInfo & fi) override;
  void compute(FVFaceResidualObject & ro, const FaceInfo & fi) override { ro.computeJacobian(fi); }
  void setup(SetupInterface & obj) override { obj.jacobianSetup(); }
  void addCached() override { _fe_problem.addCachedJacobian(_tid); }
};

template <typename RangeType>
ComputeFVFluxJacobianThread<RangeType>::ComputeFVFluxJacobianThread(
    FEProblemBase & fe_problem, const unsigned int nl_system_num, const std::set<TagID> & tags)
  : ComputeFVFluxThread<RangeType, AttribMatrixTags>(fe_problem, nl_system_num, tags)
{
}

template <typename RangeType>
ComputeFVFluxJacobianThread<RangeType>::ComputeFVFluxJacobianThread(ComputeFVFluxJacobianThread & x,
                                                                    Threads::split split)
  : ComputeFVFluxThread<RangeType, AttribMatrixTags>(x, split)
{
}

template <typename RangeType>
void
ComputeFVFluxJacobianThread<RangeType>::postFace(const FaceInfo & /*fi*/)
{
  _num_cached++;
  // TODO: do we need both calls - or just the neighbor one? - confirm this
  _fe_problem.cacheJacobian(_tid);
  _fe_problem.cacheJacobianNeighbor(_tid);

  if (_num_cached % 20 == 0)
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    _fe_problem.addCachedJacobian(_tid);
  }
}

template <typename RangeType>
class ComputeFVFluxRJThread : public ComputeFVFluxThread<RangeType, AttribVectorTags>
{
public:
  ComputeFVFluxRJThread(FEProblemBase & fe_problem,
                        const unsigned int nl_system_num,
                        const std::set<TagID> & vector_tags,
                        const std::set<TagID> & /*matrix_tags*/);

  ComputeFVFluxRJThread(ComputeFVFluxRJThread & x, Threads::split split);

protected:
  using ComputeFVFluxThread<RangeType, AttribVectorTags>::_fe_problem;
  using ComputeFVFluxThread<RangeType, AttribVectorTags>::_tid;
  using ComputeFVFluxThread<RangeType, AttribVectorTags>::_num_cached;

  void postFace(const FaceInfo & fi) override;
  void compute(FVFaceResidualObject & ro, const FaceInfo & fi) override
  {
    ro.computeResidualAndJacobian(fi);
  }
  void setup(SetupInterface & obj) override { obj.residualSetup(); }
  void addCached() override
  {
    _fe_problem.addCachedResidual(_tid);
    _fe_problem.addCachedJacobian(_tid);
  }
};

template <typename RangeType>
ComputeFVFluxRJThread<RangeType>::ComputeFVFluxRJThread(FEProblemBase & fe_problem,
                                                        const unsigned int nl_system_num,
                                                        const std::set<TagID> & vector_tags,
                                                        const std::set<TagID> & /*matrix_tags*/)
  : ComputeFVFluxThread<RangeType, AttribVectorTags>(fe_problem, nl_system_num, vector_tags)
{
}

template <typename RangeType>
ComputeFVFluxRJThread<RangeType>::ComputeFVFluxRJThread(ComputeFVFluxRJThread & x,
                                                        Threads::split split)
  : ComputeFVFluxThread<RangeType, AttribVectorTags>(x, split)
{
}

template <typename RangeType>
void
ComputeFVFluxRJThread<RangeType>::postFace(const FaceInfo & /*fi*/)
{
  _num_cached++;
  // TODO: do we need both calls - or just the neighbor one? - confirm this
  _fe_problem.cacheResidual(_tid);
  _fe_problem.cacheResidualNeighbor(_tid);
  _fe_problem.cacheJacobian(_tid);
  _fe_problem.cacheJacobianNeighbor(_tid);

  if (_num_cached % 20 == 0)
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    _fe_problem.addCachedResidual(_tid);
    _fe_problem.addCachedJacobian(_tid);
  }
}

template <typename RangeType, typename AttributeTagType>
void
ComputeFVFluxThread<RangeType, AttributeTagType>::printGeneralExecutionInformation() const
{
  if (!_fe_problem.shouldPrintExecution(_tid))
    return;
  auto & console = _fe_problem.console();
  auto execute_on = _fe_problem.getCurrentExecuteOnFlag();
  console << "[DBG] Beginning finite volume flux objects loop on " << execute_on << std::endl;
  mooseDoOnce(console << "[DBG] Loop on faces (FaceInfo), objects ordered on each face: "
                      << std::endl;
              console << "[DBG] - (finite volume) flux kernels" << std::endl;
              console << "[DBG] - (finite volume) flux boundary conditions" << std::endl;
              console << "[DBG] - (finite volume) interface kernels" << std::endl;);
}

template <typename RangeType, typename AttributeTagType>
void
ComputeFVFluxThread<RangeType, AttributeTagType>::printBlockExecutionInformation() const
{
  if (!_fe_problem.shouldPrintExecution(_tid) || !_fv_flux_kernels.size())
    return;

  // Print the location of the execution
  const auto block_pair = std::make_pair(_subdomain, _neighbor_subdomain);
  const auto block_pair_names = this->getBlockNames();
  if (_blocks_exec_printed.count(block_pair))
    return;
  auto & console = _fe_problem.console();
  console << "[DBG] Flux kernels on block " << block_pair_names.first;
  if (_neighbor_subdomain != Moose::INVALID_BLOCK_ID)
    console << " and neighbor " << block_pair_names.second << std::endl;
  else
    console << " with no neighbor block" << std::endl;

  // Print the list of objects
  std::vector<MooseObject *> fv_flux_kernels;
  for (const auto & fv_kernel : _fv_flux_kernels)
    fv_flux_kernels.push_back(dynamic_cast<MooseObject *>(fv_kernel));
  console << ConsoleUtils::formatString(ConsoleUtils::mooseObjectVectorToString(fv_flux_kernels),
                                        "[DBG]")
          << std::endl;
  _blocks_exec_printed.insert(block_pair);
}

template <typename RangeType, typename AttributeTagType>
void
ComputeFVFluxThread<RangeType, AttributeTagType>::printBoundaryExecutionInformation(
    const BoundaryID bnd_id) const
{
  if (!_fe_problem.shouldPrintExecution(_tid))
    return;
  if (_boundaries_exec_printed.count(bnd_id))
    return;
  std::vector<MooseObject *> bcs;
  _fe_problem.theWarehouse()
      .query()
      .template condition<AttribSystem>("FVFluxBC")
      .template condition<AttribThread>(_tid)
      .template condition<AttributeTagType>(_tags)
      .template condition<AttribBoundaries>(bnd_id)
      .queryInto(bcs);

  std::vector<MooseObject *> iks;
  _fe_problem.theWarehouse()
      .query()
      .template condition<AttribSystem>("FVInterfaceKernel")
      .template condition<AttribThread>(_tid)
      .template condition<AttributeTagType>(_tags)
      .template condition<AttribBoundaries>(bnd_id)
      .queryInto(iks);

  const auto block_pair_names = this->getBlockNames();
  if (bcs.size())
  {
    auto & console = _fe_problem.console();
    console << "[DBG] FVBCs on boundary " << bnd_id << " between subdomain "
            << block_pair_names.first;
    if (_neighbor_subdomain != Moose::INVALID_BLOCK_ID)
      console << " and neighbor " << block_pair_names.second << std::endl;
    else
      console << " and the exterior of the mesh " << std::endl;
    const std::string fv_bcs = ConsoleUtils::mooseObjectVectorToString(bcs);
    console << ConsoleUtils::formatString(fv_bcs, "[DBG]") << std::endl;
  }
  if (iks.size())
  {
    auto & console = _fe_problem.console();
    console << "[DBG] FVIKs on boundary " << bnd_id << " between subdomain "
            << block_pair_names.first;
    if (_neighbor_subdomain != Moose::INVALID_BLOCK_ID)
      console << " and neighbor " << block_pair_names.second << std::endl;
    else
      console << " and the exterior of the mesh " << std::endl;
    const std::string fv_iks = ConsoleUtils::mooseObjectVectorToString(iks);
    console << ConsoleUtils::formatString(fv_iks, "[DBG]") << std::endl;
  }
  _boundaries_exec_printed.insert(bnd_id);
}

template <typename RangeType, typename AttributeTagType>
std::pair<SubdomainName, SubdomainName>
ComputeFVFluxThread<RangeType, AttributeTagType>::getBlockNames() const
{
  auto block_names = std::make_pair(_mesh.getSubdomainName(_subdomain),
                                    _mesh.getSubdomainName(_neighbor_subdomain));
  if (block_names.first == "")
    block_names.first = Moose::stringify(_subdomain);
  if (block_names.second == "")
    block_names.second = Moose::stringify(_neighbor_subdomain);
  return block_names;
}
