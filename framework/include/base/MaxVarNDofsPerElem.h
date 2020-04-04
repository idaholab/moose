//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ThreadedElementLoop.h"
// libMesh includes
#include "libmesh/elem_range.h"

class NonlinearSystem;

class MaxVarNDofsPerElem : public ThreadedElementLoop<ConstElemRange>
{
public:
  MaxVarNDofsPerElem(FEProblemBase & feproblem, NonlinearSystemBase & sys);

  // Splitting Constructor
  MaxVarNDofsPerElem(MaxVarNDofsPerElem & x, Threads::split split);

  virtual ~MaxVarNDofsPerElem();

  virtual void onElement(const Elem * elem);

  void join(const MaxVarNDofsPerElem &);

  dof_id_type max() { return _max; }

protected:
  /// The nonlinear system
  NonlinearSystemBase & _system;

  /// Maximum number of dofs for any one variable on any one element
  size_t _max;

  /// DOF map
  const DofMap & _dof_map;

  /// Reusable storage
  std::vector<dof_id_type> _dof_indices;
};
