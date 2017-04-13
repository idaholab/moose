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

#include "ComputeDiracThread.h"

// Moose Includes
#include "ParallelUniqueId.h"
#include "DiracKernel.h"
#include "Problem.h"
#include "NonlinearSystem.h"
#include "MooseVariable.h"
#include "DiracKernel.h"
#include "Assembly.h"

// libmesh includes
#include "libmesh/threads.h"

ComputeDiracThread::ComputeDiracThread(FEProblemBase & feproblem, SparseMatrix<Number> * jacobian)
  : ThreadedElementLoop<DistElemRange>(feproblem),
    _jacobian(jacobian),
    _nl(feproblem.getNonlinearSystemBase()),
    _dirac_kernels(_nl.getDiracKernelWarehouse())
{
}

// Splitting Constructor
ComputeDiracThread::ComputeDiracThread(ComputeDiracThread & x, Threads::split split)
  : ThreadedElementLoop<DistElemRange>(x, split),
    _jacobian(x._jacobian),
    _nl(x._nl),
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
  std::set<MooseVariable *> needed_moose_vars;
  _dirac_kernels.updateVariableDependency(needed_moose_vars, _tid);

  // Update material dependencies
  std::set<unsigned int> needed_mat_props;
  _dirac_kernels.updateMatPropDependency(needed_mat_props, _tid);

  _fe_problem.setActiveElementalMooseVariables(needed_moose_vars, _tid);
  _fe_problem.setActiveMaterialProperties(needed_mat_props, _tid);
}

void
ComputeDiracThread::onElement(const Elem * elem)
{
  const bool has_dirac_kernels_on_elem = _fe_problem.reinitDirac(elem, _tid);
  if (!has_dirac_kernels_on_elem)
    return;

  std::set<MooseVariable *> needed_moose_vars;
  const std::vector<std::shared_ptr<DiracKernel>> & dkernels =
      _dirac_kernels.getActiveObjects(_tid);

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
    else if (_jacobian == NULL)
    {
      dirac_kernel->computeResidual();
      continue;
    }

    // Get a list of coupled variables from the SubProblem
    std::vector<std::pair<MooseVariable *, MooseVariable *>> & coupling_entries =
        dirac_kernel->subProblem().assembly(_tid).couplingEntries();

    // Loop over the list of coupled variable pairs
    for (auto & it : coupling_entries)
    {
      MooseVariable * ivariable = it.first;
      MooseVariable * jvariable = it.second;

      // A variant of the check that is in
      // ComputeFullJacobianThread::computeJacobian().  We
      // only want to call computeOffDiagJacobian() if both
      // variables are active on this subdomain, and the
      // off-diagonal variable actually has dofs.
      if (dirac_kernel->variable().number() == ivariable->number() &&
          ivariable->activeOnSubdomain(_subdomain) && jvariable->activeOnSubdomain(_subdomain) &&
          (jvariable->numberOfDofs() > 0))
      {
        dirac_kernel->subProblem().prepareShapes(jvariable->number(), _tid);
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
  if (_jacobian == NULL)
    _fe_problem.addResidual(_tid);
  else
    _fe_problem.addJacobian(*_jacobian, _tid);
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
