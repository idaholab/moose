//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ALLLOCALDOFINDICESTHREAD_H
#define ALLLOCALDOFINDICESTHREAD_H

#include "Moose.h"
#include "MooseTypes.h"

#include "libmesh/elem_range.h"

// Forward declare classes in libMesh
namespace libMesh
{
class System;
class DofMap;
}

/**
 * Grab all the local dof indices for the variables passed in, in the system passed in.
 */
class AllLocalDofIndicesThread
{
public:
  AllLocalDofIndicesThread(System & sys, std::vector<std::string> vars);
  // Splitting Constructor
  AllLocalDofIndicesThread(AllLocalDofIndicesThread & x, Threads::split split);

  void operator()(const ConstElemRange & range);

  void join(const AllLocalDofIndicesThread & y);

  std::set<dof_id_type> _all_dof_indices;

protected:
  System & _sys;
  DofMap & _dof_map;
  std::vector<std::string> _vars;
  std::vector<unsigned int> _var_numbers;
  THREAD_ID _tid;
};

#endif // ALLLOCALDOFINDICESTHREAD_H
