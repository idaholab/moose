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
#include "FEProblem.h"
#include "SwapBackSentinel.h"
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
  ThreadedFaceLoop(FEProblemBase & fe_problem, const std::set<TagID> & tags);

  ThreadedFaceLoop(ThreadedFaceLoop & x, Threads::split split);

  virtual ~ThreadedFaceLoop();

  virtual void operator()(const RangeType & range, bool bypass_threading = false);

  void join(const ThreadedFaceLoop & /*y*/){};

  virtual void onFace(const FaceInfo & fi) = 0;
  /// This is called once for each face after all face and boundary callbacks have been
  /// finished for that face.
  virtual void postFace(const FaceInfo & /*fi*/) {}
  /// This is called once after all face-looping is finished.
  virtual void post() {}

  /// This is called once for every face that is on a boundary *after* onFace
  /// is called for the face.
  virtual void onBoundary(const FaceInfo & fi, BoundaryID boundary) = 0;

  /// Called every time the current subdomain changes (i.e. the subdomain of *this* face's elem element
  /// is not the same as the subdomain of the last face's elem element).
  virtual void subdomainChanged(){};

  /// Called every time the neighbor subdomain changes (i.e. the subdomain of *this* face's neighbor element
  /// is not the same as the subdomain of the last face's neighbor element).
  virtual void neighborSubdomainChanged(){};

  /// Called if a MooseException is caught anywhere during the computation.
  virtual void caughtMooseException(MooseException &){};

protected:
  FEProblemBase & _fe_problem;
  MooseMesh & _mesh;
  const std::set<TagID> & _tags;
  THREAD_ID _tid;

  /// The subdomain for the current element
  SubdomainID _subdomain;

  /// The subdomain for the last element
  SubdomainID _old_subdomain;

  /// The subdomain for the current neighbor
  SubdomainID _neighbor_subdomain;

  /// The subdomain for the last neighbor
  SubdomainID _old_neighbor_subdomain;
};

template <typename RangeType>
ThreadedFaceLoop<RangeType>::ThreadedFaceLoop(FEProblemBase & fe_problem,
                                              const std::set<TagID> & tags)
  : _fe_problem(fe_problem), _mesh(fe_problem.mesh()), _tags(tags)
{
}

template <typename RangeType>
ThreadedFaceLoop<RangeType>::ThreadedFaceLoop(ThreadedFaceLoop & x, Threads::split /*split*/)
  : _fe_problem(x._fe_problem), _mesh(x._mesh), _tags(x._tags)
{
}

template <typename RangeType>
ThreadedFaceLoop<RangeType>::~ThreadedFaceLoop()
{
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

      _subdomain = Moose::INVALID_BLOCK_ID;
      _neighbor_subdomain = Moose::INVALID_BLOCK_ID;

      typename RangeType::const_iterator faceinfo = range.begin();
      for (faceinfo = range.begin(); faceinfo != range.end(); ++faceinfo)
      {
        const Elem & elem = faceinfo->elem();

        _old_subdomain = _subdomain;
        _subdomain = elem.subdomain_id();
        if (_subdomain != _old_subdomain)
          subdomainChanged();

        _old_neighbor_subdomain = _neighbor_subdomain;
        _neighbor_subdomain = Elem::invalid_subdomain_id;
        if (faceinfo->neighborPtr())
          _neighbor_subdomain = faceinfo->neighbor().subdomain_id();

        if (_neighbor_subdomain != _old_neighbor_subdomain)
          neighborSubdomainChanged();

        onFace(*faceinfo);

        const std::set<BoundaryID> boundary_ids = faceinfo->boundaryIDs();
        for (auto & it : boundary_ids)
          onBoundary(*faceinfo, it);

        postFace(*faceinfo);

      } // range
      post();
    }
    catch (libMesh::LogicError & e)
    {
      throw MooseException("We caught a libMesh error");
    }
  }
  catch (MooseException & e)
  {
    caughtMooseException(e);
  }
}

/**
 * Base class for assembly-like calculations.
 */
template <typename RangeType>
class ComputeFVFluxThread : public ThreadedFaceLoop<RangeType>
{
public:
  ComputeFVFluxThread(FEProblemBase & fe_problem, const std::set<TagID> & tags);

  ComputeFVFluxThread(ComputeFVFluxThread & x, Threads::split split);

  virtual ~ComputeFVFluxThread();

  virtual void onFace(const FaceInfo & fi) override;
  virtual void postFace(const FaceInfo & fi) override;
  virtual void post() override;

  virtual void onBoundary(const FaceInfo & fi, BoundaryID boundary) override;

  virtual void subdomainChanged() override;

private:
  OnScopeExit reinitVariables(const FaceInfo & fi);

  std::set<MooseVariableFieldBase *> _needed_fv_vars;
  const bool _do_jacobian;
  unsigned int _num_cached = 0;

  using ThreadedFaceLoop<RangeType>::_fe_problem;
  using ThreadedFaceLoop<RangeType>::_mesh;
  using ThreadedFaceLoop<RangeType>::_tid;
  using ThreadedFaceLoop<RangeType>::_tags;
  using ThreadedFaceLoop<RangeType>::_subdomain;
};

template <typename RangeType>
ComputeFVFluxThread<RangeType>::ComputeFVFluxThread(FEProblemBase & fe_problem,
                                                    const std::set<TagID> & tags)
  : ThreadedFaceLoop<RangeType>(fe_problem, tags),
    _do_jacobian(fe_problem.currentlyComputingJacobian())
{
}

template <typename RangeType>
ComputeFVFluxThread<RangeType>::ComputeFVFluxThread(ComputeFVFluxThread & x, Threads::split split)
  : ThreadedFaceLoop<RangeType>(x, split),
    _needed_fv_vars(x._needed_fv_vars),
    _do_jacobian(x._do_jacobian)
{
}

template <typename RangeType>
ComputeFVFluxThread<RangeType>::~ComputeFVFluxThread()
{
}

template <typename RangeType>
OnScopeExit
ComputeFVFluxThread<RangeType>::reinitVariables(const FaceInfo & fi)
{
  // TODO: this skips necessary FE reinit.  In addition to this call, we need
  // to conditionally do some FE-specific reinit here if we have any active FE
  // variables.  However, we still want to keep/do FV-style quadrature.
  // Figure out how to do all this some day.
  _fe_problem.assembly(_tid).reinitFVFace(fi);

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

  for (auto var : _needed_fv_vars)
    var->computeFaceValues(fi);

  _fe_problem.reinitMaterialsFace(fi.elem().subdomain_id(), _tid);

  if (!fi.isBoundary())
    _fe_problem.reinitMaterialsNeighbor(fi.neighbor().subdomain_id(), _tid);
  else
  {
    // Note that this won't work right if any material properties try to
    // retrieve neighbor element geometric or other information from assembly
    // or elsewhere.  They will either get nullptrs, or possibly see elem
    // information instead of neighbor info.
    // TODO: see if we can guard against some of these problems somehow.
    _fe_problem.reinitMaterialsNeighborGhost(fi.elem().subdomain_id(), _tid);
  }

  // this is the swap-back object - don't forget to catch it into local var
  std::function<void()> fn = [this, &fi] {
    _fe_problem.swapBackMaterialsFace(_tid);
    if (!fi.isBoundary())
      _fe_problem.swapBackMaterialsNeighbor(_tid);
    else
      _fe_problem.swapBackMaterialsNeighborGhost(_tid);
  };
  return OnScopeExit(fn);
}

template <typename RangeType>
void
ComputeFVFluxThread<RangeType>::onFace(const FaceInfo & fi)
{
  std::vector<FVFluxKernel *> kernels;
  auto q = _fe_problem.theWarehouse()
               .query()
               .template condition<AttribSystem>("FVFluxKernel")
               .template condition<AttribThread>(_tid)
               .template condition<AttribVectorTags>(_tags)
               .template condition<AttribSubdomains>(_subdomain)
               .queryInto(kernels);

  if (kernels.size() == 0)
    return;

  auto swap_back_sentinel = reinitVariables(fi);

  for (const auto k : kernels)
    if (_do_jacobian)
      k->computeJacobian(fi);
    else
      k->computeResidual(fi);
}

template <typename RangeType>
void
ComputeFVFluxThread<RangeType>::onBoundary(const FaceInfo & fi, BoundaryID bnd_id)
{
  std::vector<FVFluxBC *> bcs;
  _fe_problem.theWarehouse()
      .query()
      .template condition<AttribSystem>("FVFluxBC")
      .template condition<AttribThread>(_tid)
      .template condition<AttribVectorTags>(_tags)
      .template condition<AttribBoundaries>(bnd_id)
      .queryInto(bcs);
  if (bcs.size() == 0)
    return;

  auto swap_back_sentinel = reinitVariables(fi);

  for (const auto & bc : bcs)
    if (_do_jacobian)
      bc->computeJacobian(fi);
    else
      bc->computeResidual(fi);
}

template <typename RangeType>
void
ComputeFVFluxThread<RangeType>::postFace(const FaceInfo & /*fi*/)
{
  _num_cached++;
  if (_do_jacobian)
  {
    // TODO: do we need both calls - or just the neighbor one? - confirm this
    _fe_problem.cacheJacobian(_tid);
    _fe_problem.cacheJacobianNeighbor(_tid);

    if (_num_cached % 20 == 0)
    {
      Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
      _fe_problem.addCachedJacobian(_tid);
    }
  }
  else
  {
    // TODO: do we need both calls - or just the neighbor one? - confirm this
    _fe_problem.cacheResidual(_tid);
    _fe_problem.cacheResidualNeighbor(_tid);

    if (_num_cached % 20 == 0)
    {
      Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
      _fe_problem.addCachedResidual(_tid);
    }
  }
}

template <typename RangeType>
void
ComputeFVFluxThread<RangeType>::post()
{
  // make sure we add any remaining cached residuals/jacobians to add/record
  Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
  if (_do_jacobian)
    _fe_problem.addCachedJacobian(_tid);
  else
    _fe_problem.addCachedResidual(_tid);

  _fe_problem.clearActiveElementalMooseVariables(_tid);
  _fe_problem.clearActiveMaterialProperties(_tid);
}

template <typename RangeType>
void
ComputeFVFluxThread<RangeType>::subdomainChanged()
{
  // With a subdomain change, the set of needed material properties and
  // variables may have changed, so we recompute them here.

  _needed_fv_vars.clear();
  std::set<unsigned int> needed_mat_props;

  // TODO: do this for other relevant objects - like FV BCs, FV source term
  // kernels, etc. - but we don't need to add them for other types of objects
  // like FE or DG kernels because those kernels don't run in this loop. Do we
  // really want to integrate fv source kernels into this loop?
  std::vector<FVFluxKernel *> kernels;
  _fe_problem.theWarehouse()
      .query()
      .template condition<AttribSystem>("FVFluxKernel")
      .template condition<AttribSubdomains>(_subdomain)
      .template condition<AttribThread>(_tid)
      .template condition<AttribVectorTags>(_tags)
      .queryInto(kernels);

  std::set<MooseVariableFEBase *> needed_fe_vars;
  for (auto k : kernels)
  {
    // TODO: we need a better way to do this - especially when FE objects begin to
    // couple to FV vars.  This code shoud be refactored out into one place
    // where it is easy for users to say initialize all materials and
    // variables needed by these objects for me.
    const auto & deps = k->getMooseVariableDependencies();
    for (auto var : deps)
      if (var->isFV())
        _needed_fv_vars.insert(var);
      else
        needed_fe_vars.insert(var);
    const auto & mdeps = k->getMatPropDependencies();
    needed_mat_props.insert(mdeps.begin(), mdeps.end());
  }

  _fe_problem.setActiveElementalMooseVariables(needed_fe_vars, _tid);
  _fe_problem.setActiveMaterialProperties(needed_mat_props, _tid);
  _fe_problem.prepareMaterials(_subdomain, _tid);
}
