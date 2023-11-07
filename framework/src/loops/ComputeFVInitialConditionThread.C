//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeFVInitialConditionThread.h"
#include "FEProblem.h"
#include "FVInitialConditionTempl.h"

ComputeFVInitialConditionThread::ComputeFVInitialConditionThread(FEProblemBase & fe_problem)
  : _fe_problem(fe_problem)
{
}

ComputeFVInitialConditionThread::ComputeFVInitialConditionThread(
    ComputeFVInitialConditionThread & x, Threads::split /*split*/)
  : _fe_problem(x._fe_problem)
{
}

void
ComputeFVInitialConditionThread::operator()(const ElemInfoRange & range)
{
  ParallelUniqueId puid;
  _tid = puid.id;

  const FVInitialConditionWarehouse & warehouse = _fe_problem.getFVInitialConditionWarehouse();
  printGeneralExecutionInformation();

  // Iterate over all the elements in the range
  for (const auto & elem_info : range)
    if (warehouse.hasActiveBlockObjects(elem_info->subdomain_id(), _tid))
      for (auto ic : warehouse.getActiveBlockObjects(elem_info->subdomain_id(), _tid))
        ic->computeElement(*elem_info);
}

void
ComputeFVInitialConditionThread::join(const ComputeFVInitialConditionThread & /*y*/)
{
}

void
ComputeFVInitialConditionThread::printGeneralExecutionInformation() const
{
  const auto & ic_wh = _fe_problem.getFVInitialConditionWarehouse();
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
