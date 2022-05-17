//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "BoundaryNodeIntegrityCheckThread.h"
#include "AuxiliarySystem.h"
#include "FEProblem.h"
#include "AuxKernel.h"
#include "NodalUserObject.h"
#include "MooseMesh.h"

#include "libmesh/threads.h"
#include "libmesh/node.h"
#include "libmesh/mesh_base.h"

#include <vector>

BoundaryNodeIntegrityCheckThread::BoundaryNodeIntegrityCheckThread(
    FEProblemBase & fe_problem, const TheWarehouse::Query & query)
  : ThreadedNodeLoop<ConstBndNodeRange, ConstBndNodeRange::const_iterator>(fe_problem),
    _aux_sys(fe_problem.getAuxiliarySystem()),
    _query(query)
{
}

// Splitting Constructor
BoundaryNodeIntegrityCheckThread::BoundaryNodeIntegrityCheckThread(
    BoundaryNodeIntegrityCheckThread & x, Threads::split split)
  : ThreadedNodeLoop<ConstBndNodeRange, ConstBndNodeRange::const_iterator>(x, split),
    _aux_sys(x._aux_sys),
    _query(x._query)
{
}

void
BoundaryNodeIntegrityCheckThread::onNode(ConstBndNodeRange::const_iterator & node_it)
{
  const BndNode * const bnode = *node_it;
  const auto boundary_id = bnode->_bnd_id;
  const Node * const node = bnode->_node;

  // We can distribute work just as the actual execution code will
  if (node->processor_id() != _fe_problem.processor_id())
    return;

  // Only check vertices. Variables may not be defined on non-vertex nodes (think first order
  // Lagrange on a second order mesh) and user-code can often handle that
  const auto & node_to_elem_map = _fe_problem.mesh().nodeToActiveSemilocalElemMap();
  const Elem * const an_elem =
      _fe_problem.mesh().getMesh().elem_ptr(libmesh_map_find(node_to_elem_map, node->id()).front());
  if (!an_elem->is_vertex(an_elem->get_node_index(node)))
    return;

  // aux check
  _aux_sys.boundaryAuxKernelIntegrityCheck(*node, boundary_id, _tid);

  // uo check
  std::vector<NodalUserObject *> objs;
  _query.clone()
      .condition<AttribThread>(_tid)
      .condition<AttribInterfaces>(Interfaces::NodalUserObject)
      .condition<AttribBoundaries>(boundary_id, true)
      .queryInto(objs);
  for (const auto & uo : objs)
    uo->checkEvaluable(*node);
}

void
BoundaryNodeIntegrityCheckThread::join(const BoundaryNodeIntegrityCheckThread & /*y*/)
{
}
