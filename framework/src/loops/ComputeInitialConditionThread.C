//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeInitialConditionThread.h"
#include "FEProblem.h"
#include "DisplacedProblem.h"
#include "Assembly.h"
#include "InitialCondition.h"

ComputeInitialConditionThread::ComputeInitialConditionThread(FEProblemBase & fe_problem)
  : _fe_problem(fe_problem)
{
}

ComputeInitialConditionThread::ComputeInitialConditionThread(ComputeInitialConditionThread & x,
                                                             Threads::split /*split*/)
  : _fe_problem(x._fe_problem)
{
}

void
ComputeInitialConditionThread::operator()(const ConstElemRange & range)
{
  ParallelUniqueId puid;
  _tid = puid.id;

  const InitialConditionWarehouse & warehouse = _fe_problem.getInitialConditionWarehouse();
  printGeneralExecutionInformation();

  // Iterate over all the elements in the range
  for (const auto & elem : range)
  {
    const unsigned int n_nodes = elem->n_nodes();

    // we need to execute objects that are for all subdomains covered by this
    // elements' nodes.
    std::set<SubdomainID> block_ids;
    for (unsigned int n = 0; n < n_nodes; n++)
    {
      auto node = elem->node_ptr(n);
      const auto & ids = _fe_problem.mesh().getNodeBlockIds(*node);
      block_ids.insert(ids.begin(), ids.end());
    }

    // we need to remember the order the variables originally are provided in
    // since the ics dependencies are resolved to handle the inter-variable
    // dependencies correctly.
    std::vector<MooseVariableFEBase *> order;

    // group all initial condition objects by variable. so we can compute all
    // its dof values at once and copy into solution vector once.  This is
    // necessary because we have to collect extra off-block ic objects from
    // nodes shared between subdomains for cases where the off-block ic "wins"
    // on the interface.  The grouping is required because we need to have all
    // the dof values for the element determined together so we can compute
    // the correct qp values, etc. for the variable.
    std::map<MooseVariableFEBase *, std::vector<std::shared_ptr<InitialConditionBase>>> groups;
    for (auto id : block_ids)
      if (warehouse.hasActiveBlockObjects(id, _tid))
        for (auto ic : warehouse.getActiveBlockObjects(id, _tid))
        {
          if ((id != elem->subdomain_id()) && !ic->variable().isNodal())
            continue;
          order.push_back(&(ic->variable()));
          groups[&(ic->variable())].push_back(ic);
        }

    _fe_problem.setCurrentSubdomainID(elem, _tid);
    _fe_problem.prepare(elem, _tid);
    _fe_problem.reinitElem(elem, _tid);

    for (auto var : order)
    {
      DenseVector<Real> Ue;
      auto & vec = groups[var];

      // because of all the off-node shenanigans/grouping above, per-variable
      // objects could possible have their order jumbled - so re-sort just in
      // case.
      try
      {
        DependencyResolverInterface::sort<std::shared_ptr<InitialConditionBase>>(vec);
      }
      catch (CyclicDependencyException<std::shared_ptr<InitialConditionBase>> & e)
      {
        DependencyResolverInterface::cyclicDependencyError<std::shared_ptr<InitialConditionBase>>(
            e, "Cyclic dependency detected in object ordering");
      }

      for (auto ic : vec)
        ic->compute();
      vec.clear();

      // Now that all dofs are set for this variable, solemnize the solution.
      var->insert(var->sys().solution());
    }
  }
}

void
ComputeInitialConditionThread::join(const ComputeInitialConditionThread & /*y*/)
{
}

void
ComputeInitialConditionThread::printGeneralExecutionInformation() const
{
  const auto & ic_wh = _fe_problem.getInitialConditionWarehouse();
  if (_fe_problem.shouldPrintExecution(_tid) && ic_wh.hasActiveObjects())
  {
    const auto & console = _fe_problem.console();
    const auto & execute_on = _fe_problem.getCurrentExecuteOnFlag();
    console << "[DBG] Executing initial conditions on elements on " << execute_on << std::endl;
    console << "[DBG] Unordered list:" << std::endl;
    console << ic_wh.activeObjectsToFormattedString() << std::endl;
    console << "[DBG] The order of execution is defined by dependency resolution on every element"
            << std::endl;
  }
}
