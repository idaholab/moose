//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeJacobianThread.h"

#include "DGKernelBase.h"
#include "FEProblem.h"
#include "IntegratedBCBase.h"
#include "InterfaceKernelBase.h"
#include "MooseVariableFE.h"
#include "NonlinearSystem.h"
#include "SwapBackSentinel.h"
#include "TimeDerivative.h"
#include "FVElementalKernel.h"
#include "MaterialBase.h"

#include "libmesh/threads.h"

ComputeJacobianThread::ComputeJacobianThread(FEProblemBase & fe_problem,
                                             const std::set<TagID> & tags)
  : ThreadedElementLoop<ConstElemRange>(fe_problem),
    _nl(fe_problem.currentNonlinearSystem()),
    _num_cached(0),
    _integrated_bcs(_nl.getIntegratedBCWarehouse()),
    _dg_kernels(_nl.getDGKernelWarehouse()),
    _interface_kernels(_nl.getInterfaceKernelWarehouse()),
    _kernels(_nl.getKernelWarehouse()),
    _tags(tags)
{
}

// Splitting Constructor
ComputeJacobianThread::ComputeJacobianThread(ComputeJacobianThread & x, Threads::split split)
  : ThreadedElementLoop<ConstElemRange>(x, split),
    _nl(x._nl),
    _num_cached(x._num_cached),
    _integrated_bcs(x._integrated_bcs),
    _dg_kernels(x._dg_kernels),
    _interface_kernels(x._interface_kernels),
    _kernels(x._kernels),
    _warehouse(x._warehouse),
    _tags(x._tags)
{
}

ComputeJacobianThread::~ComputeJacobianThread() {}

void
ComputeJacobianThread::computeJacobian()
{
  if (_warehouse->hasActiveBlockObjects(_subdomain, _tid))
  {
    auto & kernels = _warehouse->getActiveBlockObjects(_subdomain, _tid);
    for (const auto & kernel : kernels)
      if (kernel->isImplicit())
      {
        kernel->prepareShapes(kernel->variable().number());
        kernel->computeJacobian();
        if (_fe_problem.checkNonlocalCouplingRequirement() &&
            !_fe_problem.computingScalingJacobian())
          mooseError(
              "Nonlocal kernels only supported for non-diagonal coupling. Please specify an SMP "
              "preconditioner, with appropriate row-column coupling or specify full = true.");
      }
  }

  if (_fe_problem.haveFV())
    for (auto kernel : _fv_kernels)
      if (kernel->isImplicit())
        kernel->computeJacobian();
}

void
ComputeJacobianThread::computeFaceJacobian(BoundaryID bnd_id, const Elem *)
{
  const auto & bcs = _ibc_warehouse->getActiveBoundaryObjects(bnd_id, _tid);
  for (const auto & bc : bcs)
    if (bc->shouldApply() && bc->isImplicit())
    {
      bc->prepareShapes(bc->variable().number());
      bc->computeJacobian();
      if (_fe_problem.checkNonlocalCouplingRequirement() && !_fe_problem.computingScalingJacobian())
        mooseError("Nonlocal boundary conditions only supported for non-diagonal coupling. Please "
                   "specify an SMP preconditioner, with appropriate row-column coupling or specify "
                   "full = true.");
    }
}

void
ComputeJacobianThread::computeInternalFaceJacobian(const Elem * neighbor)
{
  // No need to call hasActiveObjects, this is done in the calling method (see onInternalSide)
  const auto & dgks = _dg_warehouse->getActiveBlockObjects(_subdomain, _tid);
  for (const auto & dg : dgks)
    if (dg->isImplicit())
    {
      dg->prepareShapes(dg->variable().number());
      dg->prepareNeighborShapes(dg->variable().number());
      if (dg->hasBlocks(neighbor->subdomain_id()))
        dg->computeJacobian();
    }
}

void
ComputeJacobianThread::computeInternalInterFaceJacobian(BoundaryID bnd_id)
{
  // No need to call hasActiveObjects, this is done in the calling method (see onInterface)
  const auto & intks = _ik_warehouse->getActiveBoundaryObjects(bnd_id, _tid);
  for (const auto & intk : intks)
    if (intk->isImplicit())
    {
      intk->prepareShapes(intk->variable().number());
      intk->prepareNeighborShapes(intk->neighborVariable().number());
      intk->computeJacobian();
    }
}

void
ComputeJacobianThread::subdomainChanged()
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
  _fe_problem.getMaterialWarehouse().updateBlockFEVariableCoupledVectorTagDependency(
      _subdomain, needed_fe_var_vector_tags, _tid);

  // Update material dependencies
  std::set<unsigned int> needed_mat_props;
  _kernels.updateBlockMatPropDependency(_subdomain, needed_mat_props, _tid);
  _integrated_bcs.updateBoundaryMatPropDependency(needed_mat_props, _tid);
  _dg_kernels.updateBlockMatPropDependency(_subdomain, needed_mat_props, _tid);
  _interface_kernels.updateBoundaryMatPropDependency(needed_mat_props, _tid);

  if (_fe_problem.haveFV())
  {
    _fv_kernels.clear();
    _fe_problem.theWarehouse()
        .query()
        .template condition<AttribSystem>("FVElementalKernel")
        .template condition<AttribSubdomains>(_subdomain)
        .template condition<AttribThread>(_tid)
        .queryInto(_fv_kernels);
    for (const auto fv_kernel : _fv_kernels)
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
  if (!_tags.size() || _tags.size() == _fe_problem.numMatrixTags())
  {
    _warehouse = &_kernels;
    _dg_warehouse = &_dg_kernels;
    _ibc_warehouse = &_integrated_bcs;
    _ik_warehouse = &_interface_kernels;
  }
  // If we have one tag only,
  // We call tag based storage
  else if (_tags.size() == 1)
  {
    _warehouse = &(_kernels.getMatrixTagObjectWarehouse(*(_tags.begin()), _tid));
    _dg_warehouse = &(_dg_kernels.getMatrixTagObjectWarehouse(*(_tags.begin()), _tid));
    _ibc_warehouse = &(_integrated_bcs.getMatrixTagObjectWarehouse(*(_tags.begin()), _tid));
    _ik_warehouse = &(_interface_kernels.getMatrixTagObjectWarehouse(*(_tags.begin()), _tid));
  }
  // This one may be expensive, and hopefully we do not use it so often
  else
  {
    _warehouse = &(_kernels.getMatrixTagsObjectWarehouse(_tags, _tid));
    _dg_warehouse = &(_dg_kernels.getMatrixTagsObjectWarehouse(_tags, _tid));
    _ibc_warehouse = &(_integrated_bcs.getMatrixTagsObjectWarehouse(_tags, _tid));
    _ik_warehouse = &(_interface_kernels.getMatrixTagsObjectWarehouse(_tags, _tid));
  }
}

void
ComputeJacobianThread::onElement(const Elem * elem)
{
  _fe_problem.prepare(elem, _tid);

  _fe_problem.reinitElem(elem, _tid);

  // Set up Sentinel class so that, even if reinitMaterials() throws, we
  // still remember to swap back during stack unwinding.
  SwapBackSentinel sentinel(_fe_problem, &FEProblem::swapBackMaterials, _tid);
  _fe_problem.reinitMaterials(_subdomain, _tid);

  if (_nl.getScalarVariables(_tid).size() > 0)
    _fe_problem.reinitOffDiagScalars(_tid);

  computeJacobian();
}

void
ComputeJacobianThread::onBoundary(const Elem * elem,
                                  unsigned int side,
                                  BoundaryID bnd_id,
                                  const Elem * lower_d_elem /*=nullptr*/)
{
  if (_ibc_warehouse->hasActiveBoundaryObjects(bnd_id, _tid))
  {
    _fe_problem.reinitElemFace(elem, side, bnd_id, _tid);

    // Reinitialize lower-dimensional variables for use in boundary Materials
    if (lower_d_elem)
      _fe_problem.reinitLowerDElem(lower_d_elem, _tid);

    // Set up Sentinel class so that, even if reinitMaterials() throws, we
    // still remember to swap back during stack unwinding.
    SwapBackSentinel sentinel(_fe_problem, &FEProblem::swapBackMaterialsFace, _tid);

    _fe_problem.reinitMaterialsFace(elem->subdomain_id(), _tid);
    _fe_problem.reinitMaterialsBoundary(bnd_id, _tid);

    computeFaceJacobian(bnd_id, lower_d_elem);

    if (lower_d_elem)
    {
      Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
      _fe_problem.addJacobianLowerD(_tid);
    }
  }
}

void
ComputeJacobianThread::onInternalSide(const Elem * elem, unsigned int side)
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

    computeInternalFaceJacobian(neighbor);

    {
      Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
      _fe_problem.addJacobianNeighborLowerD(_tid);
    }
  }
}

void
ComputeJacobianThread::onInterface(const Elem * elem, unsigned int side, BoundaryID bnd_id)
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

      computeInternalInterFaceJacobian(bnd_id);

      {
        Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
        _fe_problem.addJacobianNeighbor(_tid);
      }
    }
  }
}

void
ComputeJacobianThread::postElement(const Elem * /*elem*/)
{
  _fe_problem.cacheJacobian(_tid);
  _num_cached++;

  if (_num_cached % 20 == 0)
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    _fe_problem.addCachedJacobian(_tid);
  }
}

void
ComputeJacobianThread::post()
{
  _fe_problem.clearActiveElementalMooseVariables(_tid);
  _fe_problem.clearActiveMaterialProperties(_tid);
}

void
ComputeJacobianThread::join(const ComputeJacobianThread & /*y*/)
{
}

void
ComputeJacobianThread::printGeneralExecutionInformation() const
{
  if (_fe_problem.shouldPrintExecution())
  {
    auto console = _fe_problem.console();
    auto execute_on = _fe_problem.getCurrentExecuteOnFlag();
    console << "[DBG] Beginning elemental loop to compute Jacobian on " << execute_on << std::endl;
    mooseDoOnce(
    console << "[DBG] Execution order on each element:" << std::endl;
    console << "[DBG] - kernels on element quadrature points" << std::endl;
    console << "[DBG] - finite volume elemental kernels on element" << std::endl;
    console << "[DBG] - integrated boundary conditions on element side quadrature points"
        << std::endl;
    console << "[DBG] - DG kernels on element side quadrature points" << std::endl;
    console << "[DBG] - interface kernels on element side quadrature points" << std::endl;
    );
  }
}

void
ComputeJacobianThread::printBlockExecutionInformation()
{
  // Number of objects executing is approximated by size of warehouses
  int num_objects = _kernels.size() + _fv_kernels.size() + _integrated_bcs.size() +
                    _dg_kernels.size() + _interface_kernels.size();
  auto console = _fe_problem.console();
  if (_fe_problem.shouldPrintExecution() && num_objects > 0)
  {
    if (_blocks_exec_printed.count(_subdomain))
      return;
    console << "[DBG] Ordering of Jacobian Objects on block " << _subdomain << std::endl;
    if (_kernels.hasActiveObjects())
    {
      console << "[DBG] Ordering of kernels:" << std::endl;
      console << "[DBG] " << _kernels.activeObjectsToString() << std::endl;
    }
    if (_fv_kernels.size())
    {
      console << "[DBG] Ordering of FV elemental kernels:" << std::endl;
      std::string fvkernels =
          std::accumulate(_fv_kernels.begin() + 1,
                          _fv_kernels.end(),
                          _fv_kernels[0]->name(),
                          [](const std::string & str_out, FVElementalKernel * kernel)
                          { return str_out + " " + kernel->name(); });
      console << "[DBG] " << fvkernels << std::endl;
    }
    if (_dg_kernels.hasActiveObjects())
    {
      console << "[DBG] Ordering of DG kernels:" << std::endl;
      console << "[DBG] " << _dg_kernels.activeObjectsToString() << std::endl;
    }
    if (_integrated_bcs.hasActiveObjects())
    {
      console << "[DBG] Ordering of boundary conditions:" << std::endl;
      console << "[DBG] " << _integrated_bcs.activeObjectsToString() << std::endl;
    }
    if (_interface_kernels.hasActiveObjects())
    {
      console << "[DBG] Ordering of interface kernels:" << std::endl;
      console << "[DBG] " << _interface_kernels.activeObjectsToString() << std::endl;
    }
    _blocks_exec_printed.insert(_subdomain);
  }
  else if (_fe_problem.shouldPrintExecution() && num_objects == 0 &&
           _blocks_exec_printed.count(_subdomain))
    console << "[DBG] No Objects contributing to Jacobian on block " << _subdomain << std::endl;
}
