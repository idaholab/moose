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

#include "MaxQpsThread.h"

#include "FEProblem.h"

// libmesh includes
#include "libmesh/threads.h"
#include LIBMESH_INCLUDE_UNORDERED_SET
LIBMESH_DEFINE_HASH_POINTERS

MaxQpsThread::MaxQpsThread(FEProblem & fe_problem) :
    _fe_problem(fe_problem),
    _max(0)
{
}

// Splitting Constructor
MaxQpsThread::MaxQpsThread(MaxQpsThread & x, Threads::split /*split*/) :
    _fe_problem(x._fe_problem),
    _max(x._max)
{
}

void
MaxQpsThread::operator() (const ConstElemRange & range)
{
  ParallelUniqueId puid;
  _tid = puid.id;

  Assembly & assembly = _fe_problem.assembly(_tid);

  for (ConstElemRange::const_iterator elem_it = range.begin() ; elem_it != range.end(); ++elem_it)
  {
    const Elem * elem = *elem_it;

    assembly.reinit(elem);

    unsigned int qps = 4; // assembly.qPoints().size();

    if(qps > _max)
      _max = qps;
  }
}

void
MaxQpsThread::join(const MaxQpsThread & y)
{
  if(y._max > _max)
    _max = y._max;
}
