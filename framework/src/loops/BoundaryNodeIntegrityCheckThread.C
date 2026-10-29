//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "BoundaryNodeIntegrityCheckThread.h"
#include "BoundaryElemIntegrityCheckThread.h"
#include "AuxKernelBase.h"
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
    _query(query)
{
}

// Splitting Constructor
BoundaryNodeIntegrityCheckThread::BoundaryNodeIntegrityCheckThread(
    BoundaryNodeIntegrityCheckThread & x, Threads::split split)
  : ThreadedNodeLoop<ConstBndNodeRange, ConstBndNodeRange::const_iterator>(x, split),
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

  const auto & bnd_name = _fe_problem.mesh().getBoundaryName(boundary_id);

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

  // AttribSystem groups AuxKernel/VectorAuxKernel/ArrayAuxKernel under the same "AuxKernel" tag,
  // so one query into the common AuxKernelBase type covers all three value types. Mortar
  // AuxKernels are excluded: their boundaryIDs() report the primary/secondary boundaries they
  // couple across, but their coupled variables are only required to be defined on the mortar
  // segment mesh, not on every node of those boundaries.
  std::vector<AuxKernelBase *> auxkernels;
  _fe_problem.theWarehouse()
      .query()
      .condition<AttribSystem>("AuxKernel")
      .condition<AttribThread>(_tid)
      .condition<AttribAuxKernelType>(AuxKernelType::Nodal | AuxKernelType::Elemental)
      .condition<AttribBoundaries>(boundary_id, true)
      .queryInto(auxkernels);
  for (const auto & aux : auxkernels)
    // Skip if this object uses geometric search because coupled variables may be defined on
    // paired boundaries instead of the boundary this node is on
    if (!aux->requiresGeometricSearch() && aux->checkVariableBoundaryIntegrity())
      boundaryIntegrityCheckError(*aux, aux->checkAllVariables(*node), bnd_name);
}

void
BoundaryNodeIntegrityCheckThread::join(const BoundaryNodeIntegrityCheckThread & /*y*/)
{
}
