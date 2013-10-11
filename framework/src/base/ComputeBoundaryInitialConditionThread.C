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

ComputeBoundaryInitialConditionThread::ComputeBoundaryInitialConditionThread(SubProblem & subproblem, SystemBase & sys, NumericVector<Number> & solution) :
    _subproblem(subproblem),
    _sys(sys),
    _solution(solution)
{
}

ComputeBoundaryInitialConditionThread::ComputeBoundaryInitialConditionThread(ComputeBoundaryInitialConditionThread & x, Threads::split /*split*/) :
    _subproblem(x._subproblem),
    _sys(x._sys),
    _solution(x._solution)
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

    _subproblem.assembly(_tid).reinit(node);

    // Loop over all the variables in the system
    const std::vector<MooseVariable *> & vars = _sys.getVariables(_tid);
    for (std::vector<MooseVariable *>::const_iterator it = vars.begin(); it != vars.end(); ++it)
    {
      MooseVariable & var = *(*it);

      InitialCondition * ic = _sys._ics[_tid].getBoundaryInitialCondition(var.name(), boundary_id);
      if (ic == NULL)
        continue;               // no initial condition -> skip this variable

      if (node->processor_id() == libMesh::processor_id())
      {
        var.reinitNode();
        var.computeNodalValues();                   // has to call this to resize the internal array
        Real value = ic->value(*node);

        var.setNodalValue(value);                  // update variable data, which is referenced by others, so the value is up-to-date
        // We are done, so update the solution vector
        {
          Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
          var.insert(_solution);
        }
      }
    }
  }
}

void
ComputeBoundaryInitialConditionThread::join(const ComputeBoundaryInitialConditionThread & /*y*/)
{
}
