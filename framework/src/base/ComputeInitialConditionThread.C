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

#include "ComputeInitialConditionThread.h"
#include "FEProblem.h"
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

  // Iterate over all the elements in the range
  for (const auto & elem : range)
  {
    SubdomainID subdomain = elem->subdomain_id();

    _fe_problem.prepare(elem, _tid);

    if (warehouse.hasActiveBlockObjects(subdomain, _tid))
    {
      const auto & ics = warehouse.getActiveBlockObjects(subdomain, _tid);
      for (const auto & ic : ics)
        ic->compute();
    }
  }
}

void
ComputeInitialConditionThread::join(const ComputeInitialConditionThread & /*y*/)
{
}
