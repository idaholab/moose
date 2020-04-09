//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ThreadedNodeLoop.h"
// libMesh includes
#include "libmesh/node_range.h"

class NonlinearSystem;

class MaxVarNDofsPerNode : public ThreadedNodeLoop<ConstNodeRange, ConstNodeRange::const_iterator>
{
public:
  MaxVarNDofsPerNode(FEProblemBase & feproblem, NonlinearSystemBase & sys);

  // Splitting Constructor
  MaxVarNDofsPerNode(MaxVarNDofsPerNode & x, Threads::split split);

  virtual ~MaxVarNDofsPerNode();

  virtual void onNode(ConstNodeRange::const_iterator & node_it) override;

  void join(const MaxVarNDofsPerNode &);

  dof_id_type max() { return _max; }

protected:
  /// The nonlinear system
  NonlinearSystemBase & _system;

  /// Maximum number of dofs for any one variable on any one node
  size_t _max;

  /// DOF map
  const DofMap & _dof_map;

  /// Reusable storage
  std::vector<dof_id_type> _dof_indices;
};
