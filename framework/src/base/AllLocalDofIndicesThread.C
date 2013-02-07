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

#include "AllLocalDofIndicesThread.h"

#include "FEProblem.h"

// libmesh includes
#include "libmesh/threads.h"
#include LIBMESH_INCLUDE_UNORDERED_SET
LIBMESH_DEFINE_HASH_POINTERS

AllLocalDofIndicesThread::AllLocalDofIndicesThread(System & sys,
                                                   std::vector<std::string> vars) :
    _sys(sys),
    _dof_map(sys.get_dof_map()),
    _vars(vars)
{
  _var_numbers.resize(_vars.size());
  for(unsigned int i=0; i<_vars.size(); i++)
    _var_numbers[i] = _sys.variable_number(_vars[i]);
}

// Splitting Constructor
AllLocalDofIndicesThread::AllLocalDofIndicesThread(AllLocalDofIndicesThread & x, Threads::split /*split*/) :
    _sys(x._sys),
    _dof_map(x._dof_map),
    _vars(x._vars),
    _var_numbers(x._var_numbers)
{
}

void
AllLocalDofIndicesThread::operator() (const ConstElemRange & range)
{
  ParallelUniqueId puid;
  _tid = puid.id;

  for (ConstElemRange::const_iterator elem_it = range.begin() ; elem_it != range.end(); ++elem_it)
  {
    const Elem * elem = *elem_it;

    std::vector<unsigned int> dof_indices;

    // prepare variables
    for (unsigned int i=0; i<_var_numbers.size(); i++)
    {
      _dof_map.dof_indices(elem, dof_indices, _var_numbers[i]);
      for(unsigned int j=0; j<dof_indices.size(); j++)
        _all_dof_indices.insert(dof_indices[j]);
    }
  }
}

void
AllLocalDofIndicesThread::join(const AllLocalDofIndicesThread & y)
{
  _all_dof_indices.insert(y._all_dof_indices.begin(), y._all_dof_indices.end());
}
