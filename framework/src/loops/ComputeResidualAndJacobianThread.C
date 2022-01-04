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
    _num_cached(0)
{
}

// Splitting Constructor
ComputeResidualAndJacobianThread::ComputeResidualAndJacobianThread(
    ComputeResidualAndJacobianThread & x, Threads::split split)
  : ThreadedElementLoop<ConstElemRange>(x, split), _nl(x._nl), _tags(x._tags), _num_cached(0)
{
}

ComputeResidualAndJacobianThread::~ComputeResidualAndJacobianThread() {}

void
ComputeResidualAndJacobianThread::subdomainChanged()
{
  _fe_problem.subdomainSetup(_subdomain, _tid);
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

  mooseAssert(_fe_problem.haveFV(), "Currently this is only being used with FV");

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
ComputeResidualAndJacobianThread::join(const ComputeResidualAndJacobianThread & /*y*/)
{
}
