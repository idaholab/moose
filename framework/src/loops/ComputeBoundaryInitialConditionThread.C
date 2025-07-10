//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeBoundaryInitialConditionThread.h"

// MOOSE includes
#include "Assembly.h"
#include "InitialCondition.h"
#include "MooseVariableFE.h"
#include "SystemBase.h"

ComputeBoundaryInitialConditionThread::ComputeBoundaryInitialConditionThread(
    FEProblemBase & fe_problem)
  : ThreadedNodeLoop<ConstBndNodeRange, ConstBndNodeRange::const_iterator>(fe_problem),
    _target_var_names({}),
    _target_var_usage(TargetVarUsageForIC::SKIP_LIST)
{
}

ComputeBoundaryInitialConditionThread::ComputeBoundaryInitialConditionThread(
    ComputeBoundaryInitialConditionThread & x, Threads::split split)
  : ThreadedNodeLoop<ConstBndNodeRange, ConstBndNodeRange::const_iterator>(x, split),
    _target_var_names(x._target_var_names),
    _target_var_usage(x._target_var_usage)
{
}

ComputeBoundaryInitialConditionThread::ComputeBoundaryInitialConditionThread(
    FEProblemBase & fe_problem,
    const std::set<VariableName> & target_var_names,
    const TargetVarUsageForIC target_var_usage)
  : ThreadedNodeLoop<ConstBndNodeRange, ConstBndNodeRange::const_iterator>(fe_problem),
    _target_var_names(target_var_names),
    _target_var_usage(target_var_usage)
{
}

void
ComputeBoundaryInitialConditionThread::onNode(ConstBndNodeRange::const_iterator & nd)
{
  const BndNode * bnode = *nd;

  Node * node = bnode->_node;
  BoundaryID boundary_id = bnode->_bnd_id;

  for (const auto nl_sys_num : make_range(_fe_problem.numNonlinearSystems()))
    _fe_problem.assembly(_tid, nl_sys_num).reinit(node);

  const InitialConditionWarehouse & warehouse = _fe_problem.getInitialConditionWarehouse();

  if (warehouse.hasActiveBoundaryObjects(boundary_id, _tid))
  {
    const auto & ics = warehouse.getActiveBoundaryObjects(boundary_id, _tid);
    for (const auto & ic : ics)
    {

      // Skip or include initial conditions based on target variable usage
      const auto & var_name = ic->variable().name();
      if ((_target_var_usage == TargetVarUsageForIC::SKIP_LIST &&
           _target_var_names.count(var_name)) ||
          (_target_var_usage == TargetVarUsageForIC::ONLY_LIST &&
           !_target_var_names.count(var_name)))
        continue;

      if (node->processor_id() == _fe_problem.processor_id())
        ic->computeNodal(*node);
    }
  }
}

void
ComputeBoundaryInitialConditionThread::join(const ComputeBoundaryInitialConditionThread & /*y*/)
{
}
