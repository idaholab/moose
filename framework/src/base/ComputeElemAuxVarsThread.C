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

#include "ComputeElemAuxVarsThread.h"
#include "AuxiliarySystem.h"
#include "AuxKernel.h"
#include "SwapBackSentinel.h"
#include "FEProblem.h"

// libmesh includes
#include "libmesh/threads.h"

ComputeElemAuxVarsThread::ComputeElemAuxVarsThread(FEProblemBase & problem,
                                                   const MooseObjectWarehouse<AuxKernel> & storage,
                                                   bool need_materials)
  : ThreadedElementLoop<ConstElemRange>(problem),
    _aux_sys(problem.getAuxiliarySystem()),
    _aux_kernels(storage),
    _need_materials(need_materials)
{
}

// Splitting Constructor
ComputeElemAuxVarsThread::ComputeElemAuxVarsThread(ComputeElemAuxVarsThread & x,
                                                   Threads::split /*split*/)
  : ThreadedElementLoop<ConstElemRange>(x._fe_problem),
    _aux_sys(x._aux_sys),
    _aux_kernels(x._aux_kernels),
    _need_materials(x._need_materials)
{
}

ComputeElemAuxVarsThread::~ComputeElemAuxVarsThread() {}

void
ComputeElemAuxVarsThread::subdomainChanged()
{
  // prepare variables
  for (const auto & it : _aux_sys._elem_vars[_tid])
  {
    MooseVariable * var = it.second;
    var->prepareAux();
  }

  std::set<MooseVariable *> needed_moose_vars;
  std::set<unsigned int> needed_mat_props;

  if (_aux_kernels.hasActiveBlockObjects(_subdomain, _tid))
  {
    const std::vector<std::shared_ptr<AuxKernel>> & kernels =
        _aux_kernels.getActiveBlockObjects(_subdomain, _tid);
    for (const auto & aux : kernels)
    {
      aux->subdomainSetup();
      const std::set<MooseVariable *> & mv_deps = aux->getMooseVariableDependencies();
      const std::set<unsigned int> & mp_deps = aux->getMatPropDependencies();
      needed_moose_vars.insert(mv_deps.begin(), mv_deps.end());
      needed_mat_props.insert(mp_deps.begin(), mp_deps.end());
    }
  }

  _fe_problem.setActiveElementalMooseVariables(needed_moose_vars, _tid);
  _fe_problem.setActiveMaterialProperties(needed_mat_props, _tid);
  _fe_problem.prepareMaterials(_subdomain, _tid);
}

void
ComputeElemAuxVarsThread::onElement(const Elem * elem)
{
  if (_aux_kernels.hasActiveBlockObjects(_subdomain, _tid))
  {
    const std::vector<std::shared_ptr<AuxKernel>> & kernels =
        _aux_kernels.getActiveBlockObjects(_subdomain, _tid);
    _fe_problem.prepare(elem, _tid);
    _fe_problem.reinitElem(elem, _tid);

    // Set up the sentinel so that, even if reinitMaterials() throws, we
    // still remember to swap back.
    SwapBackSentinel sentinel(_fe_problem, &FEProblem::swapBackMaterials, _tid, _need_materials);

    if (_need_materials)
      _fe_problem.reinitMaterials(elem->subdomain_id(), _tid);

    for (const auto & aux : kernels)
      aux->compute();

    // update the solution vector
    {
      Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
      for (const auto & it : _aux_sys._elem_vars[_tid])
      {
        MooseVariable * var = it.second;
        var->insert(_aux_sys.solution());
      }
    }
  }
}

void
ComputeElemAuxVarsThread::post()
{
  _fe_problem.clearActiveElementalMooseVariables(_tid);
  _fe_problem.clearActiveMaterialProperties(_tid);
}

void
ComputeElemAuxVarsThread::join(const ComputeElemAuxVarsThread & /*y*/)
{
}
