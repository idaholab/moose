//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
#include "MooseVariable.h"
#include "SystemBase.h"

ComputeBoundaryInitialConditionThread::ComputeBoundaryInitialConditionThread(
    FEProblemBase & fe_problem)
  : ThreadedNodeLoop<ConstBndNodeRange, ConstBndNodeRange::const_iterator>(fe_problem)
{
}

ComputeBoundaryInitialConditionThread::ComputeBoundaryInitialConditionThread(
    ComputeBoundaryInitialConditionThread & x, Threads::split split)
  : ThreadedNodeLoop<ConstBndNodeRange, ConstBndNodeRange::const_iterator>(x, split)
{
}

void
ComputeBoundaryInitialConditionThread::onNode(ConstBndNodeRange::const_iterator & nd)
{
  const BndNode * bnode = *nd;

  Node * node = bnode->_node;
  BoundaryID boundary_id = bnode->_bnd_id;

  _fe_problem.assembly(_tid).reinit(node);

  const InitialConditionWarehouse & warehouse = _fe_problem.getInitialConditionWarehouse();

  if (warehouse.hasActiveBoundaryObjects(boundary_id, _tid))
  {
    const std::vector<std::shared_ptr<InitialCondition>> & ics =
        warehouse.getActiveBoundaryObjects(boundary_id, _tid);
    for (const auto & ic : ics)
    {
      if (node->processor_id() == _fe_problem.processor_id())
      {
        MooseVariable & var = ic->variable();
        var.reinitNode();
        var.computeNodalValues(); // has to call this to resize the internal array
        Real value = ic->value(*node);

        var.setNodalValue(value); // update variable data, which is referenced by others, so the
                                  // value is up-to-date

        // We are done, so update the solution vector
        {
          Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
          var.insert(var.sys().solution());
        }
      }
    }
  }
}

void
ComputeBoundaryInitialConditionThread::join(const ComputeBoundaryInitialConditionThread & /*y*/)
{
}
