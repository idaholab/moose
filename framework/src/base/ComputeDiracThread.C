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

//Moose Includes
#include "ParallelUniqueId.h"
#include "DiracKernel.h"
#include "Problem.h"
#include "NonlinearSystem.h"
#include "MooseVariable.h"
#include "DiracKernel.h"
#include "Assembly.h"

// libmesh includes
#include "libmesh/threads.h"

ComputeDiracThread::ComputeDiracThread(FEProblem & feproblem,
                                       NonlinearSystem & system,
                                       SparseMatrix<Number> * jacobian) :
    ThreadedElementLoop<DistElemRange>(feproblem, system),
    _jacobian(jacobian),
    _sys(system),
    _dirac_kernels(_sys.getDiracKernelWarehouse())
{}

// Splitting Constructor
ComputeDiracThread::ComputeDiracThread(ComputeDiracThread & x, Threads::split split) :
    ThreadedElementLoop<DistElemRange>(x, split),
    _jacobian(x._jacobian),
    _sys(x._sys),
    _dirac_kernels(x._dirac_kernels)
{
}

ComputeDiracThread::~ComputeDiracThread()
{
}

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
  _fe_problem.setActiveElementalMooseVariables(needed_moose_vars, _tid);
}


void
ComputeDiracThread::onElement(const Elem * elem)
{
  const bool has_dirac_kernels_on_elem = _fe_problem.reinitDirac(elem, _tid);
  if (!has_dirac_kernels_on_elem)
    return;

  std::set<MooseVariable *> needed_moose_vars;
  const std::vector<MooseSharedPointer<DiracKernel> > & dkernels = _dirac_kernels.getActiveObjects(_tid);

  // Only call reinitMaterials() if one or more DiracKernels has
  // actually called getMaterialProperty().  Loop over all the
  // DiracKernels and check whether this is the case.
  bool need_reinit_materials = false;
  {
    for (const auto & dirac_kernel : dkernels)
    {
      // If any of the DiracKernels have had getMaterialProperty()
      // called, we need to reinit Materials.
      if (dirac_kernel->getMaterialPropertyCalled())
      {
        need_reinit_materials = true;
        break;
      }
    }
  }

  if (need_reinit_materials)
    _fe_problem.reinitMaterialsDirac(_subdomain, _tid);

  for (const auto & dirac_kernel : dkernels)
  {
    if (dirac_kernel->hasPointsOnElem(elem))
    {
      if (_jacobian == NULL)
        dirac_kernel->computeResidual();
      else
      {
        // Get a list of coupled variables from the SubProblem
        std::vector<std::pair<MooseVariable *, MooseVariable *> > & coupling_entries =
          dirac_kernel->subProblem().assembly(_tid).couplingEntries();

        // Loop over the list of coupled variable pairs
        {
          std::vector<std::pair<MooseVariable *, MooseVariable *> >::iterator
            var_pair_iter = coupling_entries.begin(),
            var_pair_end = coupling_entries.end();

          for (; var_pair_iter != var_pair_end; ++var_pair_iter)
          {
            MooseVariable * ivariable = var_pair_iter->first;
            MooseVariable * jvariable = var_pair_iter->second;

            // A variant of the check that is in
            // ComputeFullJacobianThread::computeJacobian().  We
            // only want to call computeOffDiagJacobian() if both
            // variables are active on this subdomain, and the
            // off-diagonal variable actually has dofs.
            if (dirac_kernel->variable().number() == ivariable->number()
                && ivariable->activeOnSubdomain(_subdomain)
                && jvariable->activeOnSubdomain(_subdomain)
                && (jvariable->numberOfDofs() > 0))
            {
              dirac_kernel->subProblem().prepareShapes(jvariable->number(), _tid);
              dirac_kernel->computeOffDiagJacobian(jvariable->number());
            }
          }
        }
      }
    }
  }

  if (need_reinit_materials)
    _fe_problem.swapBackMaterialsDirac(_tid);

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
}

void
ComputeDiracThread::join(const ComputeDiracThread & /*y*/)
{
}
