//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeResidualAndJacobianThread.h"
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

ComputeResidualAndJacobianThread::ComputeResidualAndJacobianThread(
    FEProblemBase & fe_problem,
    const std::set<TagID> & vector_tags,
    const std::set<TagID> & /*matrix_tags*/)
  : ThreadedElementLoop<ConstElemRange>(fe_problem),
    _nl(fe_problem.getNonlinearSystemBase()),
    _tags(vector_tags),
    _num_cached(0),
    _integrated_bcs(_nl.getIntegratedBCWarehouse()),
    _kernels(_nl.getKernelWarehouse())
{
}

// Splitting Constructor
ComputeResidualAndJacobianThread::ComputeResidualAndJacobianThread(
    ComputeResidualAndJacobianThread & x, Threads::split split)
  : ThreadedElementLoop<ConstElemRange>(x, split),
    _nl(x._nl),
    _tags(x._tags),
    _num_cached(0),
    _integrated_bcs(x._integrated_bcs),
    _kernels(x._kernels)
{
}

ComputeResidualAndJacobianThread::~ComputeResidualAndJacobianThread() {}

void
ComputeResidualAndJacobianThread::subdomainChanged()
{
  _fe_problem.subdomainSetup(_subdomain, _tid);

  // Update variable Dependencies
  std::set<MooseVariableFEBase *> needed_moose_vars;
  _kernels.updateBlockVariableDependency(_subdomain, needed_moose_vars, _tid);
  _integrated_bcs.updateBoundaryVariableDependency(needed_moose_vars, _tid);

  // Update material dependencies
  std::set<unsigned int> needed_mat_props;
  _kernels.updateBlockMatPropDependency(_subdomain, needed_mat_props, _tid);
  _integrated_bcs.updateBoundaryMatPropDependency(needed_mat_props, _tid);

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
  _fe_problem.prepareMaterials(_subdomain, _tid);
}

void
ComputeResidualAndJacobianThread::onElement(const Elem * elem)
{
  _fe_problem.prepare(elem, _tid);
  _fe_problem.reinitElem(elem, _tid);

  // Set up Sentinel class so that, even if reinitMaterials() throws, we
  // still remember to swap back during stack unwinding.
  SwapBackSentinel sentinel(_fe_problem, &FEProblem::swapBackMaterials, _tid);

  _fe_problem.reinitMaterials(_subdomain, _tid);

  if (_kernels.hasActiveBlockObjects(_subdomain, _tid))
  {
    const auto & kernels = _kernels.getActiveBlockObjects(_subdomain, _tid);
    for (const auto & kernel : kernels)
      kernel->computeResidualAndJacobian();
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

    for (auto * const kernel : kernels)
      kernel->computeResidualAndJacobian();
  }
}

void
ComputeResidualAndJacobianThread::postElement(const Elem * /*elem*/)
{
  _fe_problem.cacheResidual(_tid);
  _fe_problem.cacheJacobian(_tid);
  _num_cached++;

  if (_num_cached % 20 == 0)
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    _fe_problem.addCachedResidual(_tid);
    _fe_problem.addCachedJacobian(_tid);
  }
}

void
ComputeResidualAndJacobianThread::onBoundary(const Elem * elem,
                                             unsigned int side,
                                             BoundaryID bnd_id,
                                             const Elem *)
{
  if (_integrated_bcs.hasActiveBoundaryObjects(bnd_id, _tid))
  {
    const auto & bcs = _integrated_bcs.getActiveBoundaryObjects(bnd_id, _tid);

    _fe_problem.reinitElemFace(elem, side, bnd_id, _tid);

    // Set up Sentinel class so that, even if reinitMaterialsFace() throws, we
    // still remember to swap back during stack unwinding.
    SwapBackSentinel sentinel(_fe_problem, &FEProblem::swapBackMaterialsFace, _tid);

    _fe_problem.reinitMaterialsFace(elem->subdomain_id(), _tid);
    _fe_problem.reinitMaterialsBoundary(bnd_id, _tid);

    for (const auto & bc : bcs)
    {
      if (bc->shouldApply())
        bc->computeResidualAndJacobian();
    }
  }
}

void
ComputeResidualAndJacobianThread::join(const ComputeResidualAndJacobianThread & /*y*/)
{
}
