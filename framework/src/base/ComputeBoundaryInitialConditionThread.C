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

#include "ComputeBoundaryInitialConditionThread.h"

#include "Assembly.h"
#include "InitialCondition.h"
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
