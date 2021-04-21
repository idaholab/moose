//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AllLocalDofIndicesThread.h"

#include "FEProblem.h"
#include "ParallelUniqueId.h"
#include "SystemBase.h"
#include "AuxiliarySystem.h"
#include "NonlinearSystem.h"

#include "libmesh/dof_map.h"
#include "libmesh/threads.h"
#include "libmesh/system.h"

#include "timpi/communicator.h"

#include LIBMESH_INCLUDE_UNORDERED_SET
LIBMESH_DEFINE_HASH_POINTERS

AllLocalDofIndicesThread::AllLocalDofIndicesThread(FEProblemBase & problem,
                                                   System & sys,
                                                   std::vector<std::string> vars,
                                                   bool include_semilocal)
  : ParallelObject(sys.comm()),
    _problem(problem),
    _sys(sys),
    _dof_map(sys.get_dof_map()),
    _vars(vars),
    _include_semilocal(include_semilocal)
{
  // get system base to discern between variables and array variables
  SystemBase * system_base = dynamic_cast<SystemBase *>(&_problem.getAuxiliarySystem());
  if (!system_base->hasVariable(vars[0]))
    system_base = dynamic_cast<SystemBase *>(&_problem.getNonlinearSystem());

  // get variable numbers
  _var_numbers.resize(_vars.size());
  _var_components.resize(_vars.size());
  _is_nodal.resize(_vars.size());
  for (unsigned int i = 0; i < _vars.size(); i++)
    if (system_base->isArrayVariable(_vars[i]))
    {
      ArrayMooseVariable & var = _problem.getArrayVariable(0, _vars[i]);
      _var_numbers[i] = var.number();
      _var_components[i] = var.count();
      _is_nodal[i] = var.isNodal();
      if (var.numberOfDofs() > 1)
        mooseError("I only need constant order, someone else can implement more.");
    }
    else
    {
      _var_numbers[i] = _sys.variable_number(_vars[i]);
      _var_components[i] = 0;
      // not used, arbitrarily set to true
      _is_nodal[i] = true;
    }
}

// Splitting Constructor
AllLocalDofIndicesThread::AllLocalDofIndicesThread(AllLocalDofIndicesThread & x,
                                                   Threads::split /*split*/)
  : ParallelObject(x._sys.comm()),
    _problem(x._problem),
    _sys(x._sys),
    _dof_map(x._dof_map),
    _vars(x._vars),
    _var_numbers(x._var_numbers),
    _include_semilocal(x._include_semilocal)
{
}

void
AllLocalDofIndicesThread::operator()(const ConstElemRange & range)
{
  ParallelUniqueId puid;
  _tid = puid.id;

  for (const auto & elem : range)
  {
    std::vector<dof_id_type> dof_indices;

    dof_id_type local_dof_begin = _dof_map.first_dof();
    dof_id_type local_dof_end = _dof_map.end_dof();

    // prepare variables
    for (unsigned int i = 0; i < _var_numbers.size(); i++)
    {
      // need to distinguish between array and standard variables here
      // because interface is bad
      if (_var_components[i] == 0)
      {
        _dof_map.dof_indices(elem, dof_indices, _var_numbers[i]);
        for (unsigned int j = 0; j < dof_indices.size(); j++)
        {
          dof_id_type dof = dof_indices[j];

          if (_include_semilocal || (dof >= local_dof_begin && dof < local_dof_end))
            _all_dof_indices.insert(dof);
        }
      }
      else
      {
        if (_is_nodal[i])
          mooseError("I only need elemental variables so that's on you now.");

        dof_id_type initial_dof = elem->dof_number(_sys.number(), _var_numbers[i], 0);
        for (unsigned int j = 0; j < _var_components[i]; ++j)
        {
          dof_id_type dof = initial_dof + j;
          if (_include_semilocal || (dof >= local_dof_begin && dof < local_dof_end))
            _all_dof_indices.insert(dof);
        }
      }
    }
  }
}

void
AllLocalDofIndicesThread::join(const AllLocalDofIndicesThread & y)
{
  _all_dof_indices.insert(y._all_dof_indices.begin(), y._all_dof_indices.end());
}

void
AllLocalDofIndicesThread::dofIndicesSetUnion()
{
  _communicator.set_union(_all_dof_indices);
}
