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
#include "MooseVariableFVBase.h"
#include "FVKernel.h"
#include "FEProblem.h"
#include "SwapBackSentinel.h"
#include "libmesh/libmesh_exceptions.h"
#include "libmesh/elem.h"

#include <set>

class FVBoundaryCondition;
class MooseVariableFVBase;

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
  virtual void postFace(const FaceInfo & fi) {}
  virtual void post() {}

  virtual void onBoundary(const FaceInfo & fi, BoundaryID boundary) = 0;

  /**
   * Called every time the current subdomain changes (i.e. the subdomain of _this_ element
   * is not the same as the subdomain of the last element).  Beware of over-using this!
   * You might think that you can do some expensive stuff in here and get away with it...
   * but there are applications that have TONS of subdomains....
   */
  virtual void subdomainChanged(){};

  /**
   * Called every time the neighbor subdomain changes (i.e. the subdomain of _this_ neighbor
   * is not the same as the subdomain of the last neighbor).  Beware of over-using this!
   * You might think that you can do some expensive stuff in here and get away with it...
   * but there are applications that have TONS of subdomains....
   */
  virtual void neighborSubdomainChanged(){};

  /**
   * Called if a MooseException is caught anywhere during the computation.
   * The single input parameter taken is a MooseException object.
   */
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
      .template condition<AttribSystem>("FVKernels")
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
        const Elem & elem = faceinfo->leftElem();
        unsigned int side = faceinfo->leftSideID();

        _old_subdomain = _subdomain;
        _subdomain = elem.subdomain_id();
        if (_subdomain != _old_subdomain)
          subdomainChanged();

        _old_neighbor_subdomain = _neighbor_subdomain;
        _neighbor_subdomain = faceinfo->rightElem().subdomain_id();
        if (_neighbor_subdomain != _old_neighbor_subdomain)
          neighborSubdomainChanged();

        // get elem's face residual contribution to it's neighbor
        onFace(*faceinfo);
        postFace(*faceinfo);

        // boundary faces only border one element and so only contribute to
        // one element's residual
        std::vector<BoundaryID> boundary_ids = _mesh.getBoundaryIDs(&elem, side);
        for (auto it = boundary_ids.begin(); it != boundary_ids.end(); ++it)
          onBoundary(*faceinfo, *it);
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

  std::set<MooseVariableFVBase *> _needed_fv_vars;
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
  // TODO: (VITAL) we need to figure out how to reinit/prepare assembly
  // appropriately for residual/jacobian calcs without doing all the other
  // heavy stuff FEProblemBase::prepare does.  Maybe we just need direct
  // assembly.prepare() and assembly.prepareNonlocal() calls? Also there might
  // be some stuff that happens in assembly::reinit/reinitFE that we need, but
  // most of that is (obviously) FE specific.  Also - we need to fall back to
  // reiniting everything like normal if there is any FV-FE variable coupling.
  _fe_problem.prepare(&fi.leftElem(), _tid);

  // TODO: this triggers a bunch of FE-specific stuff to occur that we might
  // not need if only FV variables are active.  Some of the stuff triggered by
  // this call, however is necessary - particularly for reiniting materials.
  // Figure out a way to only do the minimum required here if we only have FV
  // variables.
  _fe_problem.reinitNeighbor(&fi.leftElem(), fi.leftSideID(), _tid);

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
  // face info object.  How?

  _fe_problem.reinitMaterialsFace(fi.leftElem().subdomain_id(), _tid);
  _fe_problem.reinitMaterialsNeighbor(fi.rightElem().subdomain_id(), _tid);

  for (auto var : _needed_fv_vars)
    var->computeFaceValues(fi);

  // this is the swap-back object - don't forget to catch it into local var
  std::function<void()> fn = [this] {
    _fe_problem.swapBackMaterialsFace(_tid);
    _fe_problem.swapBackMaterialsNeighbor(_tid);
  };
  return OnScopeExit(fn);
}

template <typename RangeType>
void
ComputeFVFluxThread<RangeType>::post()
{
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
ComputeFVFluxThread<RangeType>::postFace(const FaceInfo & fi)
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
ComputeFVFluxThread<RangeType>::onFace(const FaceInfo & fi)
{
  std::vector<FVFluxKernelBase *> kernels;
  auto q = _fe_problem.theWarehouse()
               .query()
               .template condition<AttribInterfaces>(Interfaces::FVFluxKernel)
               .template condition<AttribSubdomains>(_subdomain)
               .template condition<AttribVectorTags>(_tags)
               .template condition<AttribThread>(_tid)
               .template condition<AttribSystem>("FVKernels")
               .template condition<AttribIsADJac>(_do_jacobian)
               .queryInto(kernels);

  if (kernels.size() == 0)
    return;

  auto swap_back_sentinel = reinitVariables(fi);

  // Set up Sentinels so that, even if one of the reinitMaterialsXXX() calls throws, we
  // still remember to swap back during stack unwinding.
  SwapBackSentinel face_sentinel(_fe_problem, &FEProblem::swapBackMaterialsFace, _tid);
  _fe_problem.reinitMaterialsFace(fi.leftElem().subdomain_id(), _tid);

  // this reinits the materials for the neighbor element on the face (although it may not look like
  // it)
  SwapBackSentinel neighbor_sentinel(_fe_problem, &FEProblem::swapBackMaterialsNeighbor, _tid);
  _fe_problem.reinitMaterialsNeighbor(fi.rightElem().subdomain_id(), _tid);

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
  std::vector<FVBoundaryCondition *> bcs;
  _fe_problem.theWarehouse()
      .query()
      .template condition<AttribSystem>("FVBC")
      .template condition<AttribBoundaries>(bnd_id)
      .template condition<AttribThread>(_tid)
      .template condition<AttribVectorTags>(_tags)
      .queryInto(bcs);
  if (bcs.size() == 0)
    return;

  auto swap_back_sentinel = reinitVariables(fi);

  SwapBackSentinel sentinel(_fe_problem, &FEProblem::swapBackMaterialsFace, _tid);

  _fe_problem.reinitMaterialsFace(fi.leftElem().subdomain_id(), _tid);
  _fe_problem.reinitMaterialsBoundary(bnd_id, _tid);

  for (const auto & bc : bcs)
    /*TODO: implement FV BCs - so we can bc->computeResidual(); here*/;
  // TODO: don't forget to add the if(_do_jacobian) logic too!
}

template <typename RangeType>
void
ComputeFVFluxThread<RangeType>::subdomainChanged()
{
  _needed_fv_vars.clear();
  std::set<unsigned int> needed_mat_props;

  // TODO: do this for other relevant objects - like FV BCs, FV source term
  // kernels, etc. - but we don't need to add them for other types of objects
  // like FE or DG kernels because those kernels don't run in this loop. Do we
  // really want to integrate fv source kernels into this loop?
  std::vector<FVFluxKernelBase *> kernels;
  _fe_problem.theWarehouse()
      .query()
      .template condition<AttribSystem>("FVKernels")
      .template condition<AttribSubdomains>(_subdomain)
      .template condition<AttribInterfaces>(Interfaces::FVFluxKernel)
      .template condition<AttribThread>(_tid)
      .template condition<AttribVectorTags>(_tags)
      .queryInto(kernels);

  std::set<MooseVariableFEBase *> needed_fe_vars;
  MooseVariableFVBase * fvptr = nullptr;
  for (auto k : kernels)
  {
    const auto & deps = k->getMooseVariableDependencies();
    for (auto var : deps)
      if ((fvptr = dynamic_cast<MooseVariableFVBase *>(var)))
        _needed_fv_vars.insert(fvptr);
      else
        needed_fe_vars.insert(var);
    const auto & mdeps = k->getMatPropDependencies();
    needed_mat_props.insert(mdeps.begin(), mdeps.end());
  }

  _fe_problem.setActiveElementalMooseVariables(needed_fe_vars, _tid);
  _fe_problem.setActiveMaterialProperties(needed_mat_props, _tid);
  _fe_problem.prepareMaterials(_subdomain, _tid);
}
