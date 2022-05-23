//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "MooseMesh.h"
#include "ThreadedNodeLoop.h"
#include "TheWarehouse.h"
#include "AuxKernel.h"

class AuxiliarySystem;
template <typename>
class MooseObjectTagWarehouse;
template <typename>
class ExecuteMooseObjectWarehouse;

class BoundaryNodeIntegrityCheckThread
  : public ThreadedNodeLoop<ConstBndNodeRange, ConstBndNodeRange::const_iterator>
{
public:
  BoundaryNodeIntegrityCheckThread(FEProblemBase & fe_problem, const TheWarehouse::Query & query);

  // Splitting Constructor
  BoundaryNodeIntegrityCheckThread(BoundaryNodeIntegrityCheckThread & x, Threads::split split);

  virtual void onNode(ConstBndNodeRange::const_iterator & node_it) override;

  void join(const BoundaryNodeIntegrityCheckThread & /*y*/);

protected:
  /// The auxiliary system to whom we'll delegate the boundary variable dependency integrity check
  const AuxiliarySystem & _aux_sys;

  /// Nodal auxiliary kernels acting on standard field variables
  const ExecuteMooseObjectWarehouse<AuxKernel> & _nodal_aux;

  /// Nodal auxiliary kernels acting on vector field variables
  const ExecuteMooseObjectWarehouse<VectorAuxKernel> & _nodal_vec_aux;

  /// Nodal auxiliary kernels acting on array field variables
  const ExecuteMooseObjectWarehouse<ArrayAuxKernel> & _nodal_array_aux;

  /// A warehouse query that we will use to obtain user objects for boundary variable dependency
  /// integrity checks
  const TheWarehouse::Query & _query;

  /// Node to element map. Used for determining vertex vs. non-vertex nodes
  const std::map<dof_id_type, std::vector<dof_id_type>> _node_to_elem_map;
};
