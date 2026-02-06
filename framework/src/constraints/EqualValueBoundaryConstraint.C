//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "EqualValueBoundaryConstraint.h"
#include "MooseMesh.h"

#include "libmesh/null_output_iterator.h"
#include "libmesh/parallel.h"
#include "libmesh/parallel_elem.h"
#include "libmesh/parallel_node.h"

namespace // Anonymous namespace for helpers
{
/**
 * Specific weak ordering for Elem *'s to be used in a set.
 * We use the id, but first sort by level.  This guarantees
 * when traversing the set from beginning to end the lower
 * level (parent) elements are encountered first.
 *
 * This was swiped from libMesh mesh_communication.C, and ought to be
 * replaced with libMesh::CompareElemIdsByLevel just as soon as I refactor to
 * create that - @roystgnr
 */
struct CompareElemsByLevel
{
  bool operator()(const Elem * a, const Elem * b) const
  {
    libmesh_assert(a);
    libmesh_assert(b);
    const unsigned int al = a->level(), bl = b->level();
    const dof_id_type aid = a->id(), bid = b->id();

    return (al == bl) ? aid < bid : al < bl;
  }
};

} // anonymous namespace

registerMooseObject("MooseApp", EqualValueBoundaryConstraint);

InputParameters
EqualValueBoundaryConstraint::validParams()
{
  InputParameters params = NodalConstraint::validParams();
  params.addClassDescription(
      "Constraint for enforcing that variables on each side of a boundary are equivalent.");
  params.addParam<unsigned int>(
      "primary",
      "The ID of the primary node. If no ID is provided, first node of secondary set is chosen.");
  params.addParam<Point>("primary_node_coord", "Coordinates of the primary node to locate.");
  params.addParam<std::vector<unsigned int>>("secondary_node_ids", "The IDs of the secondary node");
  params.addParam<BoundaryName>("secondary", "The boundary ID associated with the secondary side");
  params.addRequiredParam<Real>("penalty", "The penalty used for the boundary term");
  return params;
}

EqualValueBoundaryConstraint::EqualValueBoundaryConstraint(const InputParameters & parameters)
  : NodalConstraint(parameters), _penalty(getParam<Real>("penalty"))
{
  updateConstrainedNodes();
}

void
EqualValueBoundaryConstraint::meshChanged()
{
  updateConstrainedNodes();
}

void
EqualValueBoundaryConstraint::updateConstrainedNodes()
{
  populateSecondaryNodes();
  pickPrimaryNode();
  ghostPrimary();
}

void
EqualValueBoundaryConstraint::populateSecondaryNodes()
{
  // user provided nothing
  if (!isParamValid("secondary_node_ids") && !isParamValid("secondary"))
    paramError("secondary", "Either secondary or secondary_node_ids must be provided.");

  // user provided conflicting params
  if (isParamValid("secondary_node_ids") && isParamValid("secondary"))
    paramError("secondary",
               "Both 'secondary' and 'secondary_node_ids' parameters are set. They are mutually "
               "exclusive.");

  // Fill in _connected_nodes, which defines local secondary nodes in the base class
  //
  // Note that later on when we pick the primary node from the secondary node set, we
  // will make sure to remove it from _connected_nodes
  _connected_nodes.clear();

  // user provided boundary name
  if (isParamValid("secondary"))
  {
    const auto & secondary_bnd = getParam<BoundaryName>("secondary");
    const auto & secondary_nodes = _mesh.getNodeList(_mesh.getBoundaryID(secondary_bnd));

    for (const auto & nid : secondary_nodes)
      if (_mesh.nodeRef(nid).processor_id() == _subproblem.processor_id())
        _connected_nodes.push_back(nid);
  }
  // user provided node ids
  else if (isParamValid("secondary_node_ids"))
  {
    const auto & secondary_node_ids = getParam<std::vector<unsigned int>>("secondary_node_ids");
    for (const auto & nid : secondary_node_ids)
      if (_mesh.queryNodePtr(nid) &&
          _mesh.nodeRef(nid).processor_id() == _subproblem.processor_id())
        _connected_nodes.push_back(nid);
  }
}

void
EqualValueBoundaryConstraint::pickPrimaryNode()
{
  // user provided nothing
  if (isParamValid("primary") && isParamValid("primary_node_coord"))
    mooseError(
        "Both 'primary' and 'primary_node_coord' parameters are set. They are mutually exclusive.");

  dof_id_type primary_node_id = Node::invalid_id;

  // user provided primary node coordinates
  if (isParamValid("primary_node_coord"))
    primary_node_id = getPrimaryNodeIDByCoord();
  // user provided primary node id
  else if (isParamValid("primary"))
    primary_node_id = getParam<unsigned int>("primary");
  // no primary node provided, so we pick one from secondary nodes
  else
  {
    // If the user provided secondary node ids, set primary node to first one
    if (isParamValid("secondary_node_ids"))
    {
      if (_connected_nodes.size())
        primary_node_id = _connected_nodes[0];
    }
    // otherwise, we pick the minimum node id from the secondary node set
    else
    {
      if (_connected_nodes.size())
        primary_node_id = (*std::min_element(_connected_nodes.begin(), _connected_nodes.end()));
      _mesh.comm().min(primary_node_id);
    }
  }

  mooseAssert(primary_node_id != Node::invalid_id, "We should have found a primary node");

  // Remove primary node from secondary nodes
  auto it = std::find(_connected_nodes.begin(), _connected_nodes.end(), primary_node_id);
  if (it != _connected_nodes.end())
    _connected_nodes.erase(it);

  // Store primary node id
  _primary_node_vector.resize(1);
  _primary_node_vector[0] = primary_node_id;
}

dof_id_type
EqualValueBoundaryConstraint::getPrimaryNodeIDByCoord() const
{
  const Real eps = libMesh::TOLERANCE;
  std::unordered_set<dof_id_type> local_primary_node_ids;
  const auto & primary_node_coord = getParam<Point>("primary_node_coord");

  // Gather local candidates
  for (const auto & bnd_node : *_mesh.getBoundaryNodeRange())
  {
    if ((*(bnd_node->_node) - primary_node_coord).norm() < eps)
    {
      // The primary node we found should belong to the secondary node set
      //
      // Note that we do not immediately break once a match is found because in theory, although it
      // is extremely unlikely, there could be multiple coinciding nodes on the secondary boundary
      // that match the provided coordinates. In that case, we would like to emit a meaning error
      // message.
      if (std::find(_connected_nodes.begin(), _connected_nodes.end(), bnd_node->_node->id()) !=
          _connected_nodes.end())
        local_primary_node_ids.insert(bnd_node->_node->id());
    }
  }

  // Gather all candidates from all ranks
  const std::vector<dof_id_type> local_node_vec(local_primary_node_ids.begin(),
                                                local_primary_node_ids.end());
  std::vector<std::vector<dof_id_type>> gathered_node_vecs;
  _mesh.comm().allgather(local_node_vec, gathered_node_vecs);

  // Deduplicate
  std::unordered_set<dof_id_type> global_primary_node_ids;
  for (const auto & vec : gathered_node_vecs)
    global_primary_node_ids.insert(vec.begin(), vec.end());

  // Make sure we found one and exactly one
  if (global_primary_node_ids.size() == 0)
    mooseError("Couldn't find a node ID for the specified primary_node_coord.");
  else if (global_primary_node_ids.size() > 1)
    mooseError("Multiple nodes found for the specified primary_node_coord.");

  return *global_primary_node_ids.begin();
}

void
EqualValueBoundaryConstraint::ghostPrimary()
{
  const auto & node_to_elem_map = _mesh.nodeToElemMap();
  auto node_to_elem_pair = node_to_elem_map.find(_primary_node_vector[0]);
  bool found_elems = (node_to_elem_pair != node_to_elem_map.end());

  // Add elements connected to primary node to Ghosted Elements.

  // On a distributed mesh, these elements might have already been
  // remoted, in which case we need to gather them back first.
  if (!_mesh.getMesh().is_serial())
  {
#ifndef NDEBUG
    bool someone_found_elems = found_elems;
    _mesh.getMesh().comm().max(someone_found_elems);
    mooseAssert(someone_found_elems, "Missing entry in node to elem map");
#endif

    std::set<Elem *, CompareElemsByLevel> primary_elems_to_ghost;
    std::set<Node *> nodes_to_ghost;
    if (found_elems)
    {
      for (dof_id_type id : node_to_elem_pair->second)
      {
        Elem * elem = _mesh.queryElemPtr(id);
        if (elem)
        {
          primary_elems_to_ghost.insert(elem);

          const unsigned int n_nodes = elem->n_nodes();
          for (unsigned int n = 0; n != n_nodes; ++n)
            nodes_to_ghost.insert(elem->node_ptr(n));
        }
      }
    }

    // Send nodes first since elements need them
    _mesh.getMesh().comm().allgather_packed_range(&_mesh.getMesh(),
                                                  nodes_to_ghost.begin(),
                                                  nodes_to_ghost.end(),
                                                  libMesh::null_output_iterator<Node>());

    _mesh.getMesh().comm().allgather_packed_range(&_mesh.getMesh(),
                                                  primary_elems_to_ghost.begin(),
                                                  primary_elems_to_ghost.end(),
                                                  libMesh::null_output_iterator<Elem>());

    // After allgather_packed_range(), rebuild internal connectivity.
    // This updates ghost nodes/elements across processors and reconstructs node_to_elem_map.
    _mesh.update();

    // Find elems again now that we know they're there
    const auto & new_node_to_elem_map = _mesh.nodeToElemMap();
    node_to_elem_pair = new_node_to_elem_map.find(_primary_node_vector[0]);
    found_elems = (node_to_elem_pair != new_node_to_elem_map.end());
  }

  if (!found_elems)
    mooseError("Couldn't find any elements connected to primary node");

  const std::vector<dof_id_type> & elems = node_to_elem_pair->second;

  if (elems.size() == 0)
    mooseError("Couldn't find any elements connected to primary node");
  _subproblem.addGhostedElem(elems[0]);
}

Real
EqualValueBoundaryConstraint::computeQpResidual(Moose::ConstraintType type)
{
  switch (type)
  {
    case Moose::Secondary:
      return (_u_secondary[_i] - _u_primary[_j]) * _penalty;
    case Moose::Primary:
      return (_u_primary[_j] - _u_secondary[_i]) * _penalty;
  }
  return 0.;
}

Real
EqualValueBoundaryConstraint::computeQpJacobian(Moose::ConstraintJacobianType type)
{
  switch (type)
  {
    case Moose::SecondarySecondary:
      return _penalty;
    case Moose::SecondaryPrimary:
      return -_penalty;
    case Moose::PrimaryPrimary:
      return _penalty;
    case Moose::PrimarySecondary:
      return -_penalty;
    default:
      mooseError("Unsupported type");
      break;
  }
  return 0.;
}
