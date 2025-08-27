//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodePositions.h"

#include "libmesh/parallel_algebra.h"

registerMooseObject("MooseApp", NodePositions);

InputParameters
NodePositions::validParams()
{
  InputParameters params = Positions::validParams();
  params.addClassDescription("Positions of element nodes.");
  params += BlockRestrictable::validParams();
  params += BoundaryRestrictable::validParams();

  // Element nodes must be sorted to remove duplicates.
  params.suppressParameter<bool>("auto_sort");
  params.set<bool>("auto_sort") = true;
  // Gathered locally, should be broadcast on every process
  params.set<bool>("auto_broadcast") = true;

  return params;
}

NodePositions::NodePositions(const InputParameters & parameters)
  : Positions(parameters),
    BlockRestrictable(this),
    BoundaryRestrictable(this, true),
    _mesh(_fe_problem.mesh(getParam<bool>("use_displaced_mesh")))
{
  // Mesh is ready at construction
  initialize();
  // Trigger synchronization as the initialization is distributed
  finalize();
}

void
NodePositions::initialize()
{
  clearPositions();

  // Only needed for boundary restriction
  const BoundaryInfo * binfo = nullptr;
  if (boundaryRestricted())
    binfo = &_fe_problem.mesh().getMesh().get_boundary_info();

  for (const auto & [node_id, elems_ids] : _fe_problem.mesh().nodeToElemMap())
  {
    const auto * const node = _mesh.queryNodePtr(node_id);
    if (!node)
      continue;

    // Check that node is part of boundary restriction
    if (binfo)
    {
      if (!binfo->get_nodeset_map().count(node))
        continue;
      else
      {
        for (const auto [_, nodeset_id] : as_range(binfo->get_nodeset_map().equal_range(node)))
          if (hasBoundary(nodeset_id))
            goto inRestriction;
      }
      continue;
    }
  inRestriction:
    // Check the elements associated with the node to see if they're in the block
    // we're block-restricting. If so, add the node to the positions vector and move
    // on to the next node (to minimize duplicates).
    for (const auto elem_id : elems_ids)
    {
      const auto e = _mesh.queryElemPtr(elem_id);
      if (e && hasBlocks(e->subdomain_id()))
      {
        _positions.emplace_back(_mesh.nodeRef(node_id));
        break;
      }
    }
  }

  _initialized = true;
}

void
NodePositions::finalize()
{
  // Gather up the positions vector on all ranks
  mooseAssert(initialized(false), "Positions vector has not been initialized.");
  if (_need_broadcast)
    // The consumer/producer reporter interface can keep track of whether a reduction is needed
    // (for example if a consumer needs replicated data, but the producer is distributed) however,
    // we have currently made the decision that positions should ALWAYS be replicated
    _communicator.allgather(_positions, /* identical buffer lengths = */ false);

  // We always need to sort the positions as nodes on the boundary between different mesh partitions
  // are duplicated. We sort by X, then Y, then Z, and prune the positions list.
  std::sort(_positions.begin(), _positions.end());
  _positions.erase(std::unique(_positions.begin(), _positions.end()), _positions.end());

  // Make a KDTree with the positions
  _positions_kd_tree = std::make_unique<KDTree>(_positions, 1);

  // For now at least, we expect positions to be the same on all ranks
  mooseAssert(comm().verify(_positions), "Positions should be the same across all MPI processes.");
}
