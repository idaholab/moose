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

  // Iterate over all the elements in the range
  for (const auto & elem : range)
  {
    SubdomainID subdomain = elem->subdomain_id();
    _fe_problem.setCurrentSubdomainID(elem, _tid);
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
