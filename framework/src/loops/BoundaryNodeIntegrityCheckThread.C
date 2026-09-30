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

  // Mortar AuxKernels are excluded here: their boundaryIDs() report the primary/secondary
  // boundaries they couple across, but their coupled variables are only required to be defined
  // on the mortar segment mesh, not on every node of those boundaries.
  auto check_aux_from_the_warehouse = [node, boundary_id, &bnd_name, this](auto & system_type)
  {
    std::vector<AuxKernelBase *> auxkernels;
    _fe_problem.theWarehouse()
        .query()
        .template condition<AttribSystem>(system_type)
        .template condition<AttribThread>(_tid)
        .template condition<AttribAuxKernelMortar>(false)
        .template condition<AttribBoundaries>(boundary_id, true)
        .queryInto(auxkernels);
    if (auxkernels.empty())
      return;

    for (const auto & aux : auxkernels)
      // Skip if this object uses geometric search because coupled variables may be defined on
      // paired boundaries instead of the boundary this node is on
      if (!aux->requiresGeometricSearch() && aux->checkVariableBoundaryIntegrity())
        boundaryIntegrityCheckError(*aux, aux->checkAllVariables(*node), bnd_name);
  };

  // AttribSystem groups AuxKernel/VectorAuxKernel/ArrayAuxKernel under the same "AuxKernel" tag
  // (they execute through the same AuxiliarySystem machinery), so one call already covers all
  // three value types here since we only ever query into the common AuxKernelBase type.
  check_aux_from_the_warehouse("AuxKernel");
}

void
BoundaryNodeIntegrityCheckThread::join(const BoundaryNodeIntegrityCheckThread & /*y*/)
{
}
