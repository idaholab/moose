//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeDiracThread.h"

// Moose Includes
#include "ParallelUniqueId.h"
#include "DiracKernel.h"
#include "Problem.h"
#include "NonlinearSystem.h"
#include "MooseVariableFE.h"
#include "Assembly.h"
#include "ThreadedElementLoop.h"

#include "libmesh/threads.h"

ComputeDiracThread::ComputeDiracThread(FEProblemBase & feproblem,
                                       const std::set<TagID> & tags,
                                       bool is_jacobian)
  : ThreadedElementLoop<DistElemRange>(feproblem),
    _is_jacobian(is_jacobian),
    _nl(feproblem.currentNonlinearSystem()),
    _tags(tags),
    _dirac_kernels(_nl.getDiracKernelWarehouse())
{
}

// Splitting Constructor
ComputeDiracThread::ComputeDiracThread(ComputeDiracThread & x, Threads::split split)
  : ThreadedElementLoop<DistElemRange>(x, split),
    _is_jacobian(x._is_jacobian),
    _nl(x._nl),
    _tags(x._tags),
    _dirac_kernels(x._dirac_kernels)
{
}

ComputeDiracThread::~ComputeDiracThread() {}

void
ComputeDiracThread::pre()
{
  // Force TID=0 because we run this object _NON THREADED_
  // Take this out if we ever get Dirac's working with threads!
  _tid = 0;
}

void
ComputeDiracThread::subdomainChanged()
{
  _fe_problem.subdomainSetup(_subdomain, _tid);

  std::set<MooseVariableFEBase *> needed_moose_vars;
  _dirac_kernels.updateVariableDependency(needed_moose_vars, _tid);

  // Update material dependencies
  std::set<unsigned int> needed_mat_props;
  _dirac_kernels.updateMatPropDependency(needed_mat_props, _tid);

  _fe_problem.setActiveElementalMooseVariables(needed_moose_vars, _tid);
  _fe_problem.setActiveMaterialProperties(needed_mat_props, _tid);

  // If users pass a empty vector or a full size of vector,
  // we take all kernels
  if (!_tags.size() || _tags.size() == _fe_problem.numMatrixTags())
    _dirac_warehouse = &_dirac_kernels;
  // If we have one tag only,  We call tag based storage
  else if (_tags.size() == 1)
    _dirac_warehouse = _is_jacobian
                           ? &(_dirac_kernels.getMatrixTagObjectWarehouse(*(_tags.begin()), _tid))
                           : &(_dirac_kernels.getVectorTagObjectWarehouse(*(_tags.begin()), _tid));
  // This one may be expensive, and hopefully we do not use it so often
  else
    _dirac_warehouse = _is_jacobian ? &(_dirac_kernels.getMatrixTagsObjectWarehouse(_tags, _tid))
                                    : &(_dirac_kernels.getVectorTagsObjectWarehouse(_tags, _tid));
}

void
ComputeDiracThread::onElement(const Elem * elem)
{
  const bool has_dirac_kernels_on_elem = _fe_problem.reinitDirac(elem, _tid);
  if (!has_dirac_kernels_on_elem)
    return;

  std::set<MooseVariableFEBase *> needed_moose_vars;
  const auto & dkernels = _dirac_warehouse->getActiveObjects(_tid);

  // Only call reinitMaterials() if one or more DiracKernels has
  // actually called getMaterialProperty().  Loop over all the
  // DiracKernels and check whether this is the case.
  for (const auto & dirac_kernel : dkernels)
  {
    // If any of the DiracKernels have had getMaterialProperty()
    // called, we need to reinit Materials.
    if (dirac_kernel->getMaterialPropertyCalled())
    {
      _fe_problem.reinitMaterials(_subdomain, _tid, /*swap_stateful=*/false);
      break;
    }
  }

  for (const auto & dirac_kernel : dkernels)
  {
    if (!dirac_kernel->hasPointsOnElem(elem))
      continue;
    else if (!_is_jacobian)
    {
      dirac_kernel->computeResidual();
      continue;
    }

    // Get a list of coupled variables from the SubProblem
    const auto & coupling_entries =
        dirac_kernel->subProblem().assembly(_tid, _nl.number()).couplingEntries();

    // Loop over the list of coupled variable pairs
    for (const auto & it : coupling_entries)
    {
      const MooseVariableFEBase * const ivariable = it.first;
      const MooseVariableFEBase * const jvariable = it.second;

      // A variant of the check that is in
      // ComputeFullJacobianThread::computeJacobian().  We
      // only want to call computeOffDiagJacobian() if both
      // variables are active on this subdomain, and the
      // off-diagonal variable actually has dofs.
      if (dirac_kernel->variable().number() == ivariable->number() &&
          ivariable->activeOnSubdomain(_subdomain) && jvariable->activeOnSubdomain(_subdomain) &&
          (jvariable->numberOfDofs() > 0))
      {
        dirac_kernel->prepareShapes(jvariable->number());
        dirac_kernel->computeOffDiagJacobian(jvariable->number());
      }
    }
  }

  // Note that we do not call swapBackMaterials() here as they were
  // never swapped in the first place.  This avoids messing up
  // stored values of stateful material properties.
}

void
ComputeDiracThread::postElement(const Elem * /*elem*/)
{
  Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
  if (!_is_jacobian)
    _fe_problem.addResidual(_tid);
  else
    _fe_problem.addJacobian(_tid);
}

void
ComputeDiracThread::post()
{
  _fe_problem.clearActiveElementalMooseVariables(_tid);
  _fe_problem.clearActiveMaterialProperties(_tid);
}

void
ComputeDiracThread::join(const ComputeDiracThread & /*y*/)
{
}

void
ComputeDiracThread::printGeneralExecutionInformation() const
{
  if (!_fe_problem.shouldPrintExecution(_tid))
    return;
  const auto & console = _fe_problem.console();
  console << "[DBG] Executing Dirac Kernels on " << _fe_problem.getCurrentExecuteOnFlag().name()
          << std::endl;
}

void
ComputeDiracThread::printBlockExecutionInformation() const
{
  if (!_fe_problem.shouldPrintExecution(_tid) || _blocks_exec_printed.count(_subdomain) ||
      !_dirac_warehouse->hasActiveBlockObjects(_subdomain, _tid))
    return;

  const auto & dkernels = _dirac_warehouse->getActiveBlockObjects(_subdomain, _tid);
  const auto & console = _fe_problem.console();
  console << "[DBG] Ordering of DiracKernels on subdomain " << _subdomain << std::endl;
  printExecutionOrdering<DiracKernelBase>(dkernels, false);
  _blocks_exec_printed.insert(_subdomain);
}
