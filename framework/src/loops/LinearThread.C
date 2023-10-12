//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearThread.h"
#include "LinearSystem.h"
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
#include "ComputeJacobianThread.h"

#include "libmesh/threads.h"

LinearThread::LinearThread(FEProblemBase & fe_problem)
  : ThreadedElementLoop<ConstElemRange>(fe_problem),
    _linear_system(fe_problem.currentLinearSystem()),
    _has_active_objects(false)
{
}

// Splitting Constructor
LinearThread::LinearThread(LinearThread & x, Threads::split split)
  : ThreadedElementLoop<ConstElemRange>(x, split),
    _linear_system(x._linear_system),
    _has_active_objects(x._has_active_objects)
{
}

LinearThread::~LinearThread() {}

void
LinearThread::operator()(const ConstElemRange & range, bool bypass_threading)
{
  if (_has_active_objects)
    ThreadedElementLoop<ConstElemRange>::operator()(range, bypass_threading);
}

void
LinearThread::subdomainChanged()
{
  // This should come first to setup the residual objects before we do dependency determination of
  // material properties and variables
  determineObjectWarehouses();

  _fe_problem.subdomainSetup(_subdomain, _tid);

  // // Update variable Dependencies
  // std::set<MooseVariableFEBase *> needed_moose_vars;
  // _tag_kernels->updateBlockVariableDependency(_subdomain, needed_moose_vars, _tid);
  // _ibc_warehouse->updateBoundaryVariableDependency(needed_moose_vars, _tid);
  // _dg_warehouse->updateBlockVariableDependency(_subdomain, needed_moose_vars, _tid);
  // _ik_warehouse->updateBoundaryVariableDependency(needed_moose_vars, _tid);

  // // Update FE variable coupleable vector tags
  // std::set<TagID> needed_fe_var_vector_tags;
  // _tag_kernels->updateBlockFEVariableCoupledVectorTagDependency(
  //     _subdomain, needed_fe_var_vector_tags, _tid);
  // _ibc_warehouse->updateBlockFEVariableCoupledVectorTagDependency(
  //     _subdomain, needed_fe_var_vector_tags, _tid);
  // _fe_problem.getMaterialWarehouse().updateBlockFEVariableCoupledVectorTagDependency(
  //     _subdomain, needed_fe_var_vector_tags, _tid);

  // // Update material dependencies
  // std::set<unsigned int> needed_mat_props;
  // _tag_kernels->updateBlockMatPropDependency(_subdomain, needed_mat_props, _tid);
  // _ibc_warehouse->updateBoundaryMatPropDependency(needed_mat_props, _tid);
  // _dg_warehouse->updateBlockMatPropDependency(_subdomain, needed_mat_props, _tid);
  // _ik_warehouse->updateBoundaryMatPropDependency(needed_mat_props, _tid);

  // if (_fe_problem.haveFV())
  // {
  //   // Re-query the finite volume elemental kernels
  //   _fv_kernels.clear();
  //   _fe_problem.theWarehouse()
  //       .query()
  //       .template condition<AttribSysNum>(_nl.number())
  //       .template condition<AttribSystem>("FVElementalKernel")
  //       .template condition<AttribSubdomains>(_subdomain)
  //       .template condition<AttribThread>(_tid)
  //       .queryInto(_fv_kernels);
  //   for (const auto fv_kernel : _fv_kernels)
  //   {
  //     const auto & fv_mv_deps = fv_kernel->getMooseVariableDependencies();
  //     needed_moose_vars.insert(fv_mv_deps.begin(), fv_mv_deps.end());
  //     const auto & fv_mp_deps = fv_kernel->getMatPropDependencies();
  //     needed_mat_props.insert(fv_mp_deps.begin(), fv_mp_deps.end());
  //   }
  // }

  // _fe_problem.setActiveElementalMooseVariables(needed_moose_vars, _tid);
  // _fe_problem.setActiveMaterialProperties(needed_mat_props, _tid);
  // _fe_problem.setActiveFEVariableCoupleableVectorTags(needed_fe_var_vector_tags, _tid);
  // _fe_problem.prepareMaterials(_subdomain, _tid);
}

void
LinearThread::onElement(const Elem * elem)
{
  // _fe_problem.prepare(elem, _tid);
  // _fe_problem.reinitElem(elem, _tid);

  // // Set up Sentinel class so that, even if reinitMaterials() throws, we
  // // still remember to swap back during stack unwinding.
  // SwapBackSentinel sentinel(_fe_problem, &FEProblem::swapBackMaterials, _tid);

  // _fe_problem.reinitMaterials(_subdomain, _tid);

  // if (dynamic_cast<ComputeJacobianThread *>(this))
  //   if (_nl.getScalarVariables(_tid).size() > 0)
  //     _fe_problem.reinitOffDiagScalars(_tid);

  // computeOnElement();
}

void
LinearThread::computeOnElement()
{
  // if (_tag_kernels->hasActiveBlockObjects(_subdomain, _tid))
  // {
  //   const auto & kernels = _tag_kernels->getActiveBlockObjects(_subdomain, _tid);
  //   for (const auto & kernel : kernels)
  //     compute(*kernel);
  // }

  // if (_fe_problem.haveFV())
  //   for (auto kernel : _fv_kernels)
  //     compute(*kernel);
}

void
LinearThread::onBoundary(const Elem * elem,
                         unsigned int side,
                         BoundaryID bnd_id,
                         const Elem * lower_d_elem /*=nullptr*/)
{
  // if (_ibc_warehouse->hasActiveBoundaryObjects(bnd_id, _tid))
  // {
  //   _fe_problem.reinitElemFace(elem, side, bnd_id, _tid);

  //   // Needed to use lower-dimensional variables on Materials
  //   if (lower_d_elem)
  //     _fe_problem.reinitLowerDElem(lower_d_elem, _tid);

  //   // Set up Sentinel class so that, even if reinitMaterialsFace() throws, we
  //   // still remember to swap back during stack unwinding.
  //   SwapBackSentinel sentinel(_fe_problem, &FEProblem::swapBackMaterialsFace, _tid);

  //   _fe_problem.reinitMaterialsFace(elem->subdomain_id(), _tid);
  //   _fe_problem.reinitMaterialsBoundary(bnd_id, _tid);

  //   computeOnBoundary(bnd_id, lower_d_elem);

  //   if (lower_d_elem)
  //   {
  //     Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
  //     accumulateLower();
  //   }
  // }
}

void
LinearThread::computeOnBoundary(BoundaryID bnd_id, const Elem * /*lower_d_elem*/)
{
  // const auto & bcs = _ibc_warehouse->getActiveBoundaryObjects(bnd_id, _tid);
  // for (const auto & bc : bcs)
  //   if (bc->shouldApply())
  //     compute(*bc);
}

void
LinearThread::onInterface(const Elem * elem, unsigned int side, BoundaryID bnd_id)
{
  // if (_ik_warehouse->hasActiveBoundaryObjects(bnd_id, _tid))
  // {

  //   // Pointer to the neighbor we are currently working on.
  //   const Elem * neighbor = elem->neighbor_ptr(side);

  //   if (neighbor->active())
  //   {
  //     _fe_problem.reinitNeighbor(elem, side, _tid);

  //     // Set up Sentinels so that, even if one of the reinitMaterialsXXX() calls throws, we
  //     // still remember to swap back during stack unwinding. Note that face, boundary, and interface
  //     // all operate with the same MaterialData object
  //     SwapBackSentinel face_sentinel(_fe_problem, &FEProblem::swapBackMaterialsFace, _tid);
  //     _fe_problem.reinitMaterialsFace(elem->subdomain_id(), _tid);
  //     _fe_problem.reinitMaterialsBoundary(bnd_id, _tid);

  //     SwapBackSentinel neighbor_sentinel(_fe_problem, &FEProblem::swapBackMaterialsNeighbor,
  //     _tid); _fe_problem.reinitMaterialsNeighbor(neighbor->subdomain_id(), _tid);

  //     // Has to happen after face and neighbor properties have been computed. Note that we don't use
  //     // a sentinel here because FEProblem::swapBackMaterialsFace is going to handle face materials,
  //     // boundary materials, and interface materials (e.g. it queries the boundary material data
  //     // with the current element and side
  //     _fe_problem.reinitMaterialsInterface(bnd_id, _tid);

  //     computeOnInterface(bnd_id);

  //     {
  //       Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
  //       accumulateNeighbor();
  //     }
  //   }
  // }
}

void
LinearThread::computeOnInterface(BoundaryID bnd_id)
{
  // const auto & int_ks = _ik_warehouse->getActiveBoundaryObjects(bnd_id, _tid);
  // for (const auto & interface_kernel : int_ks)
  //   compute(*interface_kernel);
}

void
LinearThread::onInternalSide(const Elem * elem, unsigned int side)
{
  // if (_dg_warehouse->hasActiveBlockObjects(_subdomain, _tid))
  // {
  //   // Pointer to the neighbor we are currently working on.
  //   const Elem * neighbor = elem->neighbor_ptr(side);

  //   _fe_problem.reinitElemNeighborAndLowerD(elem, side, _tid);

  //   // Set up Sentinels so that, even if one of the reinitMaterialsXXX() calls throws, we
  //   // still remember to swap back during stack unwinding.
  //   SwapBackSentinel face_sentinel(_fe_problem, &FEProblem::swapBackMaterialsFace, _tid);
  //   _fe_problem.reinitMaterialsFace(elem->subdomain_id(), _tid);

  //   SwapBackSentinel neighbor_sentinel(_fe_problem, &FEProblem::swapBackMaterialsNeighbor, _tid);
  //   _fe_problem.reinitMaterialsNeighbor(neighbor->subdomain_id(), _tid);

  //   computeOnInternalFace(neighbor);

  //   {
  //     Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
  //     accumulateNeighborLower();
  //   }
  // }
}

void
LinearThread::computeOnInternalFace(const Elem * neighbor)
{
  // const auto & dgks = _dg_warehouse->getActiveBlockObjects(_subdomain, _tid);
  // for (const auto & dg_kernel : dgks)
  //   if (dg_kernel->hasBlocks(neighbor->subdomain_id()))
  //     compute(*dg_kernel, neighbor);
}

void
LinearThread::compute(FVElementalKernel & kernel)
{
  compute(static_cast<ResidualObject &>(kernel));
}

void
LinearThread::postElement(const Elem * /*elem*/)
{
  // accumulate();
}

void
LinearThread::post()
{
  // _fe_problem.clearActiveElementalMooseVariables(_tid);
  // _fe_problem.clearActiveMaterialProperties(_tid);
}
