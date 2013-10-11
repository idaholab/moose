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
// libmesh includes
#include "libmesh/threads.h"

ComputeInitialConditionThread::ComputeInitialConditionThread(FEProblem & fe_problem) :
    _fe_problem(fe_problem)
{
}

ComputeInitialConditionThread::ComputeInitialConditionThread(ComputeInitialConditionThread & x, Threads::split /*split*/) :
    _fe_problem(x._fe_problem)
{
}

void
ComputeInitialConditionThread::operator() (const ConstElemRange & range)
{
  ParallelUniqueId puid;
  _tid = puid.id;

  // Iterate over all the elements in the range
  for (ConstElemRange::const_iterator elem_it=range.begin(); elem_it != range.end(); ++elem_it)
  {
    const Elem* elem = *elem_it;

    SubdomainID subdomain = elem->subdomain_id();

    _fe_problem._ics[_tid].updateActiveICs(subdomain);
    _fe_problem.prepare(elem, _tid);

    const std::vector<InitialCondition *> & ics = _fe_problem._ics[_tid].active();
    for (std::vector<InitialCondition *>::const_iterator it = ics.begin(); it != ics.end(); ++it)
    {
      InitialCondition * ic = (*it);
      ic->compute();
    }
  }
}

void
ComputeInitialConditionThread::join(const ComputeInitialConditionThread & /*y*/)
{
}
