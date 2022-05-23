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
#include "BoundaryElemIntegrityCheckThread.h"
#include "AuxiliarySystem.h"
#include "NonlinearSystemBase.h"
#include "FEProblemBase.h"
#include "NodalUserObject.h"
#include "MooseMesh.h"
#include "MooseObjectTagWarehouse.h"

#include "libmesh/threads.h"
#include "libmesh/node.h"
#include "libmesh/mesh_base.h"

#include <vector>

BoundaryNodeIntegrityCheckThread::BoundaryNodeIntegrityCheckThread(
    FEProblemBase & fe_problem, const TheWarehouse::Query & query)
  : ThreadedNodeLoop<ConstBndNodeRange, ConstBndNodeRange::const_iterator>(fe_problem),
    _aux_sys(fe_problem.getAuxiliarySystem()),
    _nodal_aux(_aux_sys.nodalAuxWarehouse()),
    _nodal_vec_aux(_aux_sys.nodalVectorAuxWarehouse()),
    _nodal_array_aux(_aux_sys.nodalArrayAuxWarehouse()),
    _query(query),
    _node_to_elem_map(fe_problem.mesh().nodeToActiveSemilocalElemMap())
{
}

// Splitting Constructor
BoundaryNodeIntegrityCheckThread::BoundaryNodeIntegrityCheckThread(
    BoundaryNodeIntegrityCheckThread & x, Threads::split split)
  : ThreadedNodeLoop<ConstBndNodeRange, ConstBndNodeRange::const_iterator>(x, split),
    _aux_sys(x._aux_sys),
    _nodal_aux(x._nodal_aux),
    _nodal_vec_aux(x._nodal_vec_aux),
    _nodal_array_aux(x._nodal_array_aux),
    _query(x._query),
    _node_to_elem_map(x._node_to_elem_map)
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

  auto & mesh = _fe_problem.mesh();

  // Only check vertices. Variables may not be defined on non-vertex nodes (think first order
  // Lagrange on a second order mesh) and user-code can often handle that
  const Elem * const an_elem =
      mesh.getMesh().elem_ptr(libmesh_map_find(_node_to_elem_map, node->id()).front());
  if (!an_elem->is_vertex(an_elem->get_node_index(node)))
    return;

  const auto & bnd_name = mesh.getBoundaryName(boundary_id);

  // uo check
  std::vector<NodalUserObject *> objs;
  _query.clone()
      .condition<AttribThread>(_tid)
      .condition<AttribInterfaces>(Interfaces::NodalUserObject)
      .condition<AttribBoundaries>(boundary_id, true)
      .queryInto(objs);
  for (const auto & uo : objs)
    if (uo->checkVariableBoundaryIntegrity())
      boundaryIntegrityCheckError(*uo, uo->checkAllVariables(*node), bnd_name);

  auto check = [node, boundary_id, &bnd_name, this](const auto & warehouse)
  {
    if (!warehouse.hasBoundaryObjects(boundary_id, _tid))
      return;

    const auto & bnd_objects = warehouse.getBoundaryObjects(boundary_id, _tid);
    for (const auto & bnd_object : bnd_objects)
      // Skip if this object uses geometric search because coupled variables may be defined on
      // paired boundaries instead of the boundary this node is on
      if (!bnd_object->requiresGeometricSearch() && bnd_object->checkVariableBoundaryIntegrity())
        boundaryIntegrityCheckError(*bnd_object, bnd_object->checkAllVariables(*node), bnd_name);
  };

  check(_nodal_aux);
  check(_nodal_vec_aux);
  check(_nodal_array_aux);
}

void
BoundaryNodeIntegrityCheckThread::join(const BoundaryNodeIntegrityCheckThread & /*y*/)
{
}
