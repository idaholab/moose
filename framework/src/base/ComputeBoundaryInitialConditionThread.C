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
#include "SystemBase.h"
#include "InitialCondition.h"
// libmesh includes
#include "libmesh/threads.h"
#include "libmesh/fe_interface.h"

ComputeBoundaryInitialConditionThread::ComputeBoundaryInitialConditionThread(FEProblem & fe_problem) :
    _fe_problem(fe_problem)
{
}

ComputeBoundaryInitialConditionThread::ComputeBoundaryInitialConditionThread(ComputeBoundaryInitialConditionThread & x, Threads::split /*split*/) :
    _fe_problem(x._fe_problem)
{
}

void
ComputeBoundaryInitialConditionThread::operator() (const ConstBndNodeRange & range)
{
  ParallelUniqueId puid;
  _tid = puid.id;

  for (ConstBndNodeRange::const_iterator nd = range.begin() ; nd != range.end(); ++nd)
  {
    const BndNode * bnode = *nd;

    Node * node = bnode->_node;
    BoundaryID boundary_id = bnode->_bnd_id;

    _fe_problem.assembly(_tid).reinit(node);

    const std::vector<InitialCondition *> & ics = _fe_problem._ics[_tid].activeBoundary(boundary_id);
    for (std::vector<InitialCondition *>::const_iterator it = ics.begin(); it != ics.end(); ++it)
    {
      InitialCondition * ic = (*it);
      if (node->processor_id() == libMesh::processor_id())
      {
        MooseVariable & var = ic->variable();
        var.reinitNode();
        var.computeNodalValues();                   // has to call this to resize the internal array
        Real value = ic->value(*node);

        var.setNodalValue(value);                  // update variable data, which is referenced by others, so the value is up-to-date
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
