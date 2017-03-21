/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "ComputeJacobianThread.h"
#include "NonlinearSystem.h"
#include "FEProblem.h"
#include "TimeDerivative.h"
#include "IntegratedBC.h"
#include "DGKernel.h"
#include "InterfaceKernel.h"
#include "KernelWarehouse.h"
#include "NonlocalKernel.h"
#include "SwapBackSentinel.h"
#include "NonlocalIntegratedBC.h"

// libmesh includes
#include "libmesh/threads.h"

ComputeJacobianThread::ComputeJacobianThread(FEProblemBase & fe_problem,
                                             SparseMatrix<Number> & jacobian,
                                             Moose::KernelType kernel_type)
  : ThreadedElementLoop<ConstElemRange>(fe_problem),
    _jacobian(jacobian),
    _nl(fe_problem.getNonlinearSystemBase()),
    _num_cached(0),
    _integrated_bcs(_nl.getIntegratedBCWarehouse()),
    _dg_kernels(_nl.getDGKernelWarehouse()),
    _interface_kernels(_nl.getInterfaceKernelWarehouse()),
    _kernels(_nl.getKernelWarehouse()),
    _kernel_type(kernel_type)
{
}

// Splitting Constructor
ComputeJacobianThread::ComputeJacobianThread(ComputeJacobianThread & x, Threads::split split)
  : ThreadedElementLoop<ConstElemRange>(x, split),
    _jacobian(x._jacobian),
    _nl(x._nl),
    _num_cached(x._num_cached),
    _integrated_bcs(x._integrated_bcs),
    _dg_kernels(x._dg_kernels),
    _interface_kernels(x._interface_kernels),
    _kernels(x._kernels),
    _kernel_type(x._kernel_type)
{
}

ComputeJacobianThread::~ComputeJacobianThread() {}

void
ComputeJacobianThread::computeJacobian()
{
  const MooseObjectWarehouse<KernelBase> * warehouse;
  switch (_kernel_type)
  {
    case Moose::KT_ALL:
      warehouse = &_nl.getKernelWarehouse();
      break;

    case Moose::KT_TIME:
      warehouse = &_nl.getTimeKernelWarehouse();
      break;

    case Moose::KT_NONTIME:
      warehouse = &_nl.getNonTimeKernelWarehouse();
      break;

    case Moose::KT_EIGEN:
      warehouse = &_nl.getEigenKernelWarehouse();
      break;

    case Moose::KT_NONEIGEN:
      warehouse = &_nl.getNonEigenKernelWarehouse();
      break;

    default:
      mooseError("Unknown kernel type \n");
  }

  if (warehouse->hasActiveBlockObjects(_subdomain, _tid))
  {
    const std::vector<std::shared_ptr<KernelBase>> & kernels =
        warehouse->getActiveBlockObjects(_subdomain, _tid);
    for (const auto & kernel : kernels)
      if (kernel->isImplicit())
      {
        kernel->subProblem().prepareShapes(kernel->variable().number(), _tid);
        kernel->computeJacobian();
        /// done only when nonlocal kernels exist in the system
        if (_fe_problem.checkNonlocalCouplingRequirement())
        {
          std::shared_ptr<NonlocalKernel> nonlocal_kernel =
              std::dynamic_pointer_cast<NonlocalKernel>(kernel);
          if (nonlocal_kernel)
            kernel->computeNonlocalJacobian();
        }
      }
  }
}

void
ComputeJacobianThread::computeFaceJacobian(BoundaryID bnd_id)
{
  const std::vector<std::shared_ptr<IntegratedBC>> & bcs =
      _integrated_bcs.getActiveBoundaryObjects(bnd_id, _tid);
  for (const auto & bc : bcs)
    if (bc->shouldApply() && bc->isImplicit())
    {
      bc->subProblem().prepareFaceShapes(bc->variable().number(), _tid);
      bc->computeJacobian();
      /// done only when nonlocal integrated_bcs exist in the system
      if (_fe_problem.checkNonlocalCouplingRequirement())
      {
        std::shared_ptr<NonlocalIntegratedBC> nonlocal_integrated_bc =
            std::dynamic_pointer_cast<NonlocalIntegratedBC>(bc);
        if (nonlocal_integrated_bc)
          bc->computeNonlocalJacobian();
      }
    }
}

void
ComputeJacobianThread::computeInternalFaceJacobian(const Elem * neighbor)
{
  // No need to call hasActiveObjects, this is done in the calling method (see onInternalSide)
  const std::vector<std::shared_ptr<DGKernel>> & dgks =
      _dg_kernels.getActiveBlockObjects(_subdomain, _tid);
  for (const auto & dg : dgks)
    if (dg->isImplicit())
    {
      dg->subProblem().prepareFaceShapes(dg->variable().number(), _tid);
      dg->subProblem().prepareNeighborShapes(dg->variable().number(), _tid);
      if (dg->hasBlocks(neighbor->subdomain_id()))
        dg->computeJacobian();
    }
}

void
ComputeJacobianThread::computeInternalInterFaceJacobian(BoundaryID bnd_id)
{
  // No need to call hasActiveObjects, this is done in the calling method (see onInterface)
  const std::vector<std::shared_ptr<InterfaceKernel>> & intks =
      _interface_kernels.getActiveBoundaryObjects(bnd_id, _tid);
  for (const auto & intk : intks)
    if (intk->isImplicit())
    {
      intk->subProblem().prepareFaceShapes(intk->variable().number(), _tid);
      intk->subProblem().prepareNeighborShapes(intk->neighborVariable().number(), _tid);
      intk->computeJacobian();
    }
}

void
ComputeJacobianThread::subdomainChanged()
{
  _fe_problem.subdomainSetup(_subdomain, _tid);

  // Update variable Dependencies
  std::set<MooseVariable *> needed_moose_vars;
  _kernels.updateBlockVariableDependency(_subdomain, needed_moose_vars, _tid);
  _integrated_bcs.updateBoundaryVariableDependency(needed_moose_vars, _tid);
  _dg_kernels.updateBlockVariableDependency(_subdomain, needed_moose_vars, _tid);
  _interface_kernels.updateBoundaryVariableDependency(needed_moose_vars, _tid);

  // Update material dependencies
  std::set<unsigned int> needed_mat_props;
  _kernels.updateBlockMatPropDependency(_subdomain, needed_mat_props, _tid);
  _integrated_bcs.updateBoundaryMatPropDependency(needed_mat_props, _tid);
  _dg_kernels.updateBlockMatPropDependency(_subdomain, needed_mat_props, _tid);
  _interface_kernels.updateBoundaryMatPropDependency(needed_mat_props, _tid);

  _fe_problem.setActiveElementalMooseVariables(needed_moose_vars, _tid);
  _fe_problem.setActiveMaterialProperties(needed_mat_props, _tid);
  _fe_problem.prepareMaterials(_subdomain, _tid);
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
ComputeJacobianThread::onBoundary(const Elem * elem, unsigned int side, BoundaryID bnd_id)
{
  if (_integrated_bcs.hasActiveBoundaryObjects(bnd_id, _tid))
  {
    _fe_problem.reinitElemFace(elem, side, bnd_id, _tid);

    // Set up Sentinel class so that, even if reinitMaterials() throws, we
    // still remember to swap back during stack unwinding.
    SwapBackSentinel sentinel(_fe_problem, &FEProblem::swapBackMaterialsFace, _tid);

    _fe_problem.reinitMaterialsFace(elem->subdomain_id(), _tid);
    _fe_problem.reinitMaterialsBoundary(bnd_id, _tid);

    // Set the active boundary id so that BoundaryRestrictable::_boundary_id is correct
    _fe_problem.setCurrentBoundaryID(bnd_id);

    computeFaceJacobian(bnd_id);

    // Set the active boundary to invalid
    _fe_problem.setCurrentBoundaryID(Moose::INVALID_BOUNDARY_ID);
  }
}

void
ComputeJacobianThread::onInternalSide(const Elem * elem, unsigned int side)
{
  if (_dg_kernels.hasActiveBlockObjects(_subdomain, _tid))
  {
    // Pointer to the neighbor we are currently working on.
    const Elem * neighbor = elem->neighbor(side);

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

      computeInternalFaceJacobian(neighbor);

      {
        Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
        _fe_problem.addJacobianNeighbor(_jacobian, _tid);
      }
    }
  }
}

void
ComputeJacobianThread::onInterface(const Elem * elem, unsigned int side, BoundaryID bnd_id)
{
  if (_interface_kernels.hasActiveBoundaryObjects(bnd_id, _tid))
  {
    // Pointer to the neighbor we are currently working on.
    const Elem * neighbor = elem->neighbor(side);

    if (neighbor->active())
    {
      _fe_problem.reinitNeighbor(elem, side, _tid);

      // Set up Sentinels so that, even if one of the reinitMaterialsXXX() calls throws, we
      // still remember to swap back during stack unwinding.
      SwapBackSentinel face_sentinel(_fe_problem, &FEProblem::swapBackMaterialsFace, _tid);
      _fe_problem.reinitMaterialsFace(elem->subdomain_id(), _tid);

      SwapBackSentinel neighbor_sentinel(_fe_problem, &FEProblem::swapBackMaterialsNeighbor, _tid);
      _fe_problem.reinitMaterialsNeighbor(neighbor->subdomain_id(), _tid);

      computeInternalInterFaceJacobian(bnd_id);

      {
        Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
        _fe_problem.addJacobianNeighbor(_jacobian, _tid);
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
    _fe_problem.addCachedJacobian(_jacobian, _tid);
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
