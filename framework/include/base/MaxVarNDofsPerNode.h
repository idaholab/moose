//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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

class SolverSystem;

/**
 * Computes the maximum number of DoFs on any node for all variables
 */
class MaxVarNDofsPerNode final
  : public ThreadedNodeLoop<ConstNodeRange, ConstNodeRange::const_iterator, MaxVarNDofsPerNode>
{
public:
  MaxVarNDofsPerNode(FEProblemBase & feproblem, SolverSystem & sys);

  // Splitting Constructor
  MaxVarNDofsPerNode(MaxVarNDofsPerNode & x, Threads::split split);

  virtual ~MaxVarNDofsPerNode();

  virtual void onNode(ConstNodeRange::const_iterator & node_it);

  void join(const MaxVarNDofsPerNode &);

  dof_id_type max() { return _max; }

protected:
  /// The nonlinear system
  SolverSystem & _system;

  /// Maximum number of dofs for any one variable on any one node
  size_t _max;

  /// DOF map
  const DofMap & _dof_map;

  /// Reusable storage
  std::vector<dof_id_type> _dof_indices;
};
