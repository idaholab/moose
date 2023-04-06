//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NonlinearThread.h"
#include "NonlinearSystem.h"
#include "Problem.h"
#include "FEProblem.h"
#include "KernelBase.h"
#include "IntegratedBCBase.h"
#include "DGKernelBase.h"
#include "InterfaceKernelBase.h"
#include "Material.h"
#include "TimeKernel.h"
#include "SwapBackSentinel.h"
#include "FVTimeKernel.h"

#include "libmesh/threads.h"

NonlinearThread::NonlinearThread(FEProblemBase & fe_problem)
  : ThreadedElementLoop<ConstElemRange>(fe_problem),
    _nl(fe_problem.currentNonlinearSystem()),
    _num_cached(0),
    _integrated_bcs(_nl.getIntegratedBCWarehouse()),
    _dg_kernels(_nl.getDGKernelWarehouse()),
    _interface_kernels(_nl.getInterfaceKernelWarehouse()),
    _kernels(_nl.getKernelWarehouse()),
    _has_active_objects(_integrated_bcs.hasActiveObjects() || _dg_kernels.hasActiveObjects() ||
                        _interface_kernels.hasActiveObjects() || _kernels.hasActiveObjects() ||
                        _fe_problem.haveFV())
{
}

// Splitting Constructor
NonlinearThread::NonlinearThread(NonlinearThread & x, Threads::split split)
  : ThreadedElementLoop<ConstElemRange>(x, split),
    _nl(x._nl),
    _num_cached(0),
    _integrated_bcs(x._integrated_bcs),
    _dg_kernels(x._dg_kernels),
    _interface_kernels(x._interface_kernels),
    _kernels(x._kernels),
    _tag_kernels(x._tag_kernels),
    _has_active_objects(x._has_active_objects)
{
}

NonlinearThread::~NonlinearThread() {}

void
NonlinearThread::operator()(const ConstElemRange & range, bool bypass_threading)
{
  if (_has_active_objects)
    ThreadedElementLoop<ConstElemRange>::operator()(range, bypass_threading);
}

void
NonlinearThread::subdomainChanged()
{
  // This should come first to setup the residual objects before we do dependency determination of
  // material properties and variables
  determineResidualObjects();

  _fe_problem.subdomainSetup(_subdomain, _tid);

  // Update variable Dependencies
  std::set<MooseVariableFEBase *> needed_moose_vars;
  _tag_kernels->updateBlockVariableDependency(_subdomain, needed_moose_vars, _tid);
  _ibc_warehouse->updateBoundaryVariableDependency(needed_moose_vars, _tid);
  _dg_warehouse->updateBlockVariableDependency(_subdomain, needed_moose_vars, _tid);
  _ik_warehouse->updateBoundaryVariableDependency(needed_moose_vars, _tid);

  // Update FE variable coupleable vector tags
  std::set<TagID> needed_fe_var_vector_tags;
  _tag_kernels->updateBlockFEVariableCoupledVectorTagDependency(
      _subdomain, needed_fe_var_vector_tags, _tid);
  _ibc_warehouse->updateBlockFEVariableCoupledVectorTagDependency(
      _subdomain, needed_fe_var_vector_tags, _tid);
  _fe_problem.getMaterialWarehouse().updateBlockFEVariableCoupledVectorTagDependency(
      _subdomain, needed_fe_var_vector_tags, _tid);

  // Update material dependencies
  std::set<unsigned int> needed_mat_props;
  _tag_kernels->updateBlockMatPropDependency(_subdomain, needed_mat_props, _tid);
  _ibc_warehouse->updateBoundaryMatPropDependency(needed_mat_props, _tid);
  _dg_warehouse->updateBlockMatPropDependency(_subdomain, needed_mat_props, _tid);
  _ik_warehouse->updateBoundaryMatPropDependency(needed_mat_props, _tid);

  for (const auto fv_kernel : _fv_kernels)
  {
    const auto & fv_mv_deps = fv_kernel->getMooseVariableDependencies();
    needed_moose_vars.insert(fv_mv_deps.begin(), fv_mv_deps.end());
    const auto & fv_mp_deps = fv_kernel->getMatPropDependencies();
    needed_mat_props.insert(fv_mp_deps.begin(), fv_mp_deps.end());
  }

  _fe_problem.setActiveElementalMooseVariables(needed_moose_vars, _tid);
  _fe_problem.setActiveMaterialProperties(needed_mat_props, _tid);
  _fe_problem.setActiveFEVariableCoupleableVectorTags(needed_fe_var_vector_tags, _tid);
  _fe_problem.prepareMaterials(_subdomain, _tid);
}

void
NonlinearThread::onElement(const Elem * elem)
{
  _fe_problem.prepare(elem, _tid);
  _fe_problem.reinitElem(elem, _tid);

  // Set up Sentinel class so that, even if reinitMaterials() throws, we
  // still remember to swap back during stack unwinding.
  SwapBackSentinel sentinel(_fe_problem, &FEProblem::swapBackMaterials, _tid);

  _fe_problem.reinitMaterials(_subdomain, _tid);

  if (_tag_kernels->hasActiveBlockObjects(_subdomain, _tid))
  {
    const auto & kernels = _tag_kernels->getActiveBlockObjects(_subdomain, _tid);
    for (const auto & kernel : kernels)
      compute(*kernel);
  }

  for (auto kernel : _fv_kernels)
    compute(*kernel);
}

void
NonlinearThread::onBoundary(const Elem * elem,
                            unsigned int side,
                            BoundaryID bnd_id,
                            const Elem * lower_d_elem /*=nullptr*/)
{
  if (_ibc_warehouse->hasActiveBoundaryObjects(bnd_id, _tid))
  {
    const auto & bcs = _ibc_warehouse->getActiveBoundaryObjects(bnd_id, _tid);

    _fe_problem.reinitElemFace(elem, side, bnd_id, _tid);

    // Needed to use lower-dimensional variables on Materials
    if (lower_d_elem)
      _fe_problem.reinitLowerDElem(lower_d_elem, _tid);

    // Set up Sentinel class so that, even if reinitMaterialsFace() throws, we
    // still remember to swap back during stack unwinding.
    SwapBackSentinel sentinel(_fe_problem, &FEProblem::swapBackMaterialsFace, _tid);

    _fe_problem.reinitMaterialsFace(elem->subdomain_id(), _tid);
    _fe_problem.reinitMaterialsBoundary(bnd_id, _tid);

    for (const auto & bc : bcs)
    {
      if (bc->shouldApply())
        compute(*bc);
    }

    if (lower_d_elem)
    {
      Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
      accumulateLower();
    }
  }
}

void
NonlinearThread::onInterface(const Elem * elem, unsigned int side, BoundaryID bnd_id)
{
  if (_ik_warehouse->hasActiveBoundaryObjects(bnd_id, _tid))
  {

    // Pointer to the neighbor we are currently working on.
    const Elem * neighbor = elem->neighbor_ptr(side);

    if (neighbor->active())
    {
      _fe_problem.reinitNeighbor(elem, side, _tid);

      // Set up Sentinels so that, even if one of the reinitMaterialsXXX() calls throws, we
      // still remember to swap back during stack unwinding. Note that face, boundary, and interface
      // all operate with the same MaterialData object
      SwapBackSentinel face_sentinel(_fe_problem, &FEProblem::swapBackMaterialsFace, _tid);
      _fe_problem.reinitMaterialsFace(elem->subdomain_id(), _tid);
      _fe_problem.reinitMaterialsBoundary(bnd_id, _tid);

      SwapBackSentinel neighbor_sentinel(_fe_problem, &FEProblem::swapBackMaterialsNeighbor, _tid);
      _fe_problem.reinitMaterialsNeighbor(neighbor->subdomain_id(), _tid);

      // Has to happen after face and neighbor properties have been computed. Note that we don't use
      // a sentinel here because FEProblem::swapBackMaterialsFace is going to handle face materials,
      // boundary materials, and interface materials (e.g. it queries the boundary material data
      // with the current element and side
      _fe_problem.reinitMaterialsInterface(bnd_id, _tid);

      const auto & int_ks = _ik_warehouse->getActiveBoundaryObjects(bnd_id, _tid);
      for (const auto & interface_kernel : int_ks)
        compute(*interface_kernel);

      {
        Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
        accumulateNeighbor();
      }
    }
  }
}

void
NonlinearThread::onInternalSide(const Elem * elem, unsigned int side)
{
  if (_dg_warehouse->hasActiveBlockObjects(_subdomain, _tid))
  {
    // Pointer to the neighbor we are currently working on.
    const Elem * neighbor = elem->neighbor_ptr(side);

    _fe_problem.reinitElemNeighborAndLowerD(elem, side, _tid);

    // Set up Sentinels so that, even if one of the reinitMaterialsXXX() calls throws, we
    // still remember to swap back during stack unwinding.
    SwapBackSentinel face_sentinel(_fe_problem, &FEProblem::swapBackMaterialsFace, _tid);
    _fe_problem.reinitMaterialsFace(elem->subdomain_id(), _tid);

    SwapBackSentinel neighbor_sentinel(_fe_problem, &FEProblem::swapBackMaterialsNeighbor, _tid);
    _fe_problem.reinitMaterialsNeighbor(neighbor->subdomain_id(), _tid);

    const auto & dgks = _dg_warehouse->getActiveBlockObjects(_subdomain, _tid);
    for (const auto & dg_kernel : dgks)
      if (dg_kernel->hasBlocks(neighbor->subdomain_id()))
        compute(*dg_kernel);

    {
      Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
      accumulateNeighborLower();
    }
  }
}

void
NonlinearThread::postElement(const Elem * /*elem*/)
{
  accumulate();
}

void
NonlinearThread::post()
{
  _fe_problem.clearActiveElementalMooseVariables(_tid);
  _fe_problem.clearActiveMaterialProperties(_tid);
}
