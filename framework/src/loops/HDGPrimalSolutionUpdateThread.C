//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HDGPrimalSolutionUpdateThread.h"
#include "FEProblemBase.h"
#include "MooseObjectWarehouse.h"
#include "HDGKernel.h"

HDGPrimalSolutionUpdateThread::HDGPrimalSolutionUpdateThread(
    FEProblemBase & fe_problem, MooseObjectWarehouse<HDGKernel> & hybridized_kernels)
  : ThreadedElementLoop<ConstElemRange>(fe_problem), _hybridized_kernels(hybridized_kernels)
{
  mooseAssert(
      _hybridized_kernels.hasActiveObjects(),
      "We should not create the pre-check loop object if there are no active hybridized kernels");
}

// Splitting Constructor
HDGPrimalSolutionUpdateThread::HDGPrimalSolutionUpdateThread(HDGPrimalSolutionUpdateThread & x,
                                                             Threads::split split)
  : ThreadedElementLoop<ConstElemRange>(x, split), _hybridized_kernels(x._hybridized_kernels)
{
}

HDGPrimalSolutionUpdateThread::~HDGPrimalSolutionUpdateThread() {}

void
HDGPrimalSolutionUpdateThread::subdomainChanged()
{
  _fe_problem.subdomainSetup(_subdomain, _tid);

  // Update variable dependencies
  std::set<MooseVariableFEBase *> needed_moose_vars;
  _hybridized_kernels.updateBlockVariableDependency(_subdomain, needed_moose_vars, _tid);

  // Update material dependencies
  std::unordered_set<unsigned int> needed_mat_props;
  _hybridized_kernels.updateBlockMatPropDependency(_subdomain, needed_mat_props, _tid);

  _fe_problem.setActiveElementalMooseVariables(needed_moose_vars, _tid);
  _fe_problem.prepareMaterials(needed_mat_props, _subdomain, _tid);
}

void
HDGPrimalSolutionUpdateThread::onElement(const Elem * const elem)
{
  // Set up Sentinel class so that, even if reinitMaterials() throws in prepareElement, we
  // still remember to swap back during stack unwinding.
  SwapBackSentinel sentinel(_fe_problem, &FEProblemBase::swapBackMaterials, this->_tid);

  prepareElement(elem);

  auto & hkernels = _hybridized_kernels.getActiveBlockObjects(_subdomain, _tid);
  for (auto & hkernel : hkernels)
    hkernel->computePostLinearSolve();
}

void
HDGPrimalSolutionUpdateThread::post()
{
  clearVarsAndMaterials();
}
