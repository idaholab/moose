//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeResidualThread.h"
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

ComputeResidualThread::ComputeResidualThread(FEProblemBase & fe_problem,
                                             const std::set<TagID> & tags)
  : ThreadedElementLoop<ConstElemRange>(fe_problem),
    _nl(fe_problem.getNonlinearSystemBase()),
    _tags(tags),
    _num_cached(0),
    _integrated_bcs(_nl.getIntegratedBCWarehouse()),
    _dg_kernels(_nl.getDGKernelWarehouse()),
    _interface_kernels(_nl.getInterfaceKernelWarehouse()),
    _kernels(_nl.getKernelWarehouse())
{
}

// Splitting Constructor
ComputeResidualThread::ComputeResidualThread(ComputeResidualThread & x, Threads::split split)
  : ThreadedElementLoop<ConstElemRange>(x, split),
    _nl(x._nl),
    _tags(x._tags),
    _num_cached(0),
    _integrated_bcs(x._integrated_bcs),
    _dg_kernels(x._dg_kernels),
    _interface_kernels(x._interface_kernels),
    _kernels(x._kernels),
    _tag_kernels(x._tag_kernels)
{
}

ComputeResidualThread::~ComputeResidualThread() {}

void
ComputeResidualThread::subdomainChanged()
{
  _fe_problem.subdomainSetup(_subdomain, _tid);

  // Update variable Dependencies
  std::set<MooseVariableFEBase *> needed_moose_vars;
  _kernels.updateBlockVariableDependency(_subdomain, needed_moose_vars, _tid);
  _integrated_bcs.updateBoundaryVariableDependency(needed_moose_vars, _tid);
  _dg_kernels.updateBlockVariableDependency(_subdomain, needed_moose_vars, _tid);
  _interface_kernels.updateBoundaryVariableDependency(needed_moose_vars, _tid);

  // Update FE variable coupleable vector tags
  std::set<TagID> needed_fe_var_vector_tags;
  _kernels.updateBlockFEVariableCoupledVectorTagDependency(
      _subdomain, needed_fe_var_vector_tags, _tid);

  // Update material dependencies
  std::set<unsigned int> needed_mat_props;
  _kernels.updateBlockMatPropDependency(_subdomain, needed_mat_props, _tid);
  _integrated_bcs.updateBoundaryMatPropDependency(needed_mat_props, _tid);
  _dg_kernels.updateBlockMatPropDependency(_subdomain, needed_mat_props, _tid);
  _interface_kernels.updateBoundaryMatPropDependency(needed_mat_props, _tid);

  if (_fe_problem.haveFV())
  {
    std::vector<FVElementalKernel *> fv_kernels;
    _fe_problem.theWarehouse()
        .query()
        .template condition<AttribSystem>("FVElementalKernel")
        .template condition<AttribSubdomains>(_subdomain)
        .template condition<AttribThread>(_tid)
        .template condition<AttribVectorTags>(_tags)
        .queryInto(fv_kernels);
    for (const auto fv_kernel : fv_kernels)
    {
      const auto & fv_mv_deps = fv_kernel->getMooseVariableDependencies();
      needed_moose_vars.insert(fv_mv_deps.begin(), fv_mv_deps.end());
      const auto & fv_mp_deps = fv_kernel->getMatPropDependencies();
      needed_mat_props.insert(fv_mp_deps.begin(), fv_mp_deps.end());
    }
  }

  _fe_problem.setActiveElementalMooseVariables(needed_moose_vars, _tid);
  _fe_problem.setActiveMaterialProperties(needed_mat_props, _tid);
  _fe_problem.setActiveFEVariableCoupleableVectorTags(needed_fe_var_vector_tags, _tid);
  _fe_problem.prepareMaterials(_subdomain, _tid);

  // If users pass a empty vector or a full size of vector,
  // we take all kernels
  if (!_tags.size() || _tags.size() == _fe_problem.numVectorTags(Moose::VECTOR_TAG_RESIDUAL))
  {
    _tag_kernels = &_kernels;
    _dg_warehouse = &_dg_kernels;
    _ibc_warehouse = &_integrated_bcs;
    _ik_warehouse = &_interface_kernels;
  }
  // If we have one tag only,
  // We call tag based storage
  else if (_tags.size() == 1)
  {
    _tag_kernels = &(_kernels.getVectorTagObjectWarehouse(*(_tags.begin()), _tid));
    _dg_warehouse = &(_dg_kernels.getVectorTagObjectWarehouse(*(_tags.begin()), _tid));
    _ibc_warehouse = &(_integrated_bcs.getVectorTagObjectWarehouse(*(_tags.begin()), _tid));
    _ik_warehouse = &(_interface_kernels.getVectorTagObjectWarehouse(*(_tags.begin()), _tid));
  }
  // This one may be expensive
  else
  {
    _tag_kernels = &(_kernels.getVectorTagsObjectWarehouse(_tags, _tid));
    _dg_warehouse = &(_dg_kernels.getVectorTagsObjectWarehouse(_tags, _tid));
    _ibc_warehouse = &(_integrated_bcs.getVectorTagsObjectWarehouse(_tags, _tid));
    _ik_warehouse = &(_interface_kernels.getVectorTagsObjectWarehouse(_tags, _tid));
  }
}

void
ComputeResidualThread::onElement(const Elem * elem)
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
      kernel->computeResidual();
  }

  // TODO: use a querycache here and then we probably won't need the if-guard
  // anymore.
  if (_fe_problem.haveFV())
  {
    std::vector<FVElementalKernel *> kernels;
    _fe_problem.theWarehouse()
        .query()
        .template condition<AttribSystem>("FVElementalKernel")
        .template condition<AttribSubdomains>(_subdomain)
        .template condition<AttribThread>(_tid)
        .template condition<AttribVectorTags>(_tags)
        .queryInto(kernels);

    for (auto kernel : kernels)
      kernel->computeResidual();
  }
}

void
ComputeResidualThread::onBoundary(const Elem * elem,
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
        bc->computeResidual();
    }
  }
}

void
ComputeResidualThread::onInterface(const Elem * elem, unsigned int side, BoundaryID bnd_id)
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
        interface_kernel->computeResidual();

      {
        Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
        _fe_problem.addResidualNeighbor(_tid);
      }
    }
  }
}

void
ComputeResidualThread::onInternalSide(const Elem * elem, unsigned int side)
{
  if (_dg_warehouse->hasActiveBlockObjects(_subdomain, _tid))
  {
    // Pointer to the neighbor we are currently working on.
    const Elem * neighbor = elem->neighbor_ptr(side);

    // Get the global id of the element and the neighbor
    const dof_id_type elem_id = elem->id(), neighbor_id = neighbor->id();

    if ((neighbor->active() && (neighbor->level() == elem->level()) && (elem_id < neighbor_id)) ||
        (neighbor->level() < elem->level()))
    {
      _fe_problem.reinitNeighbor(elem, side, _tid);

      // Set up Sentinels so that, even if one of the reinitMaterialsXXX() calls throws, we
      // still remember to swap back during stack unwinding.
      SwapBackSentinel face_sentinel(_fe_problem, &FEProblem::swapBackMaterialsFace, _tid);
      _fe_problem.reinitMaterialsFace(elem->subdomain_id(), _tid);

      SwapBackSentinel neighbor_sentinel(_fe_problem, &FEProblem::swapBackMaterialsNeighbor, _tid);
      _fe_problem.reinitMaterialsNeighbor(neighbor->subdomain_id(), _tid);

      const auto & dgks = _dg_warehouse->getActiveBlockObjects(_subdomain, _tid);
      for (const auto & dg_kernel : dgks)
        if (dg_kernel->hasBlocks(neighbor->subdomain_id()))
          dg_kernel->computeResidual();

      {
        Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
        _fe_problem.addResidualNeighbor(_tid);
      }
    }
  }
}

void
ComputeResidualThread::postElement(const Elem * /*elem*/)
{
  _fe_problem.cacheResidual(_tid);
  _num_cached++;

  if (_num_cached % 20 == 0)
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    _fe_problem.addCachedResidual(_tid);
  }
}

void
ComputeResidualThread::post()
{
  _fe_problem.clearActiveElementalMooseVariables(_tid);
  _fe_problem.clearActiveMaterialProperties(_tid);
}

void
ComputeResidualThread::join(const ComputeResidualThread & /*y*/)
{
}
