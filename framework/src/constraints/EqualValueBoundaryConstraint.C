//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "EqualValueBoundaryConstraint.h"
#include "MooseMesh.h"

#include "libmesh/mesh_inserter_iterator.h"
#include "libmesh/parallel.h"
#include "libmesh/parallel_elem.h"
#include "libmesh/parallel_node.h"

// C++ includes
#include <limits.h>

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

defineLegacyParams(EqualValueBoundaryConstraint);

InputParameters
EqualValueBoundaryConstraint::validParams()
{
  InputParameters params = NodalConstraint::validParams();
  params.addParam<unsigned int>(
      "master",
      std::numeric_limits<unsigned int>::max(),
      "The ID of the master node. If no ID is provided, first node of secondary set is chosen.");
  params.addParam<std::vector<unsigned int>>("secondary_node_ids", "The IDs of the secondary node");
  params.addParam<BoundaryName>("secondary", "NaN", "The boundary ID associated with the secondary side");
  params.addRequiredParam<Real>("penalty", "The penalty used for the boundary term");
  return params;
}

EqualValueBoundaryConstraint::EqualValueBoundaryConstraint(const InputParameters & parameters)
  : NodalConstraint(parameters),
    _master_node_id(getParam<unsigned int>("master")),
    _secondary_node_ids(getParam<std::vector<unsigned int>>("secondary_node_ids")),
    _secondary_node_set_id(getParam<BoundaryName>("secondary")),
    _penalty(getParam<Real>("penalty"))
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
  _master_node_vector.clear();
  _connected_nodes.clear();

  if ((_secondary_node_ids.size() == 0) && (_secondary_node_set_id == "NaN"))
    mooseError("Please specify secondary node ids or boundary id.");
  else if ((_secondary_node_ids.size() == 0) && (_secondary_node_set_id != "NaN"))
  {
    std::vector<dof_id_type> nodelist = _mesh.getNodeList(_mesh.getBoundaryID(_secondary_node_set_id));
    std::vector<dof_id_type>::iterator in;

    // Set master node to first node of the secondary node set if no master node id is provided
    //_master_node_vector defines master nodes in the base class
    if (_master_node_id == std::numeric_limits<unsigned int>::max())
    {
      in = std::min_element(nodelist.begin(), nodelist.end());
      dof_id_type node_id = (in == nodelist.end()) ? DofObject::invalid_id : *in;
      _communicator.min(node_id);
      _master_node_vector.push_back(node_id);
    }
    else
      _master_node_vector.push_back(_master_node_id);

    // Fill in _connected_nodes, which defines secondary nodes in the base class
    for (in = nodelist.begin(); in != nodelist.end(); ++in)
    {
      if ((*in != _master_node_vector[0]) &&
          (_mesh.nodeRef(*in).processor_id() == _subproblem.processor_id()))
        _connected_nodes.push_back(*in);
    }
  }
  else if ((_secondary_node_ids.size() != 0) && (_secondary_node_set_id == "NaN"))
  {
    if (_master_node_id == std::numeric_limits<unsigned int>::max())
      _master_node_vector.push_back(
          _secondary_node_ids[0]); //_master_node_vector defines master nodes in the base class

    // Fill in _connected_nodes, which defines secondary nodes in the base class
    for (const auto & dof : _secondary_node_ids)
    {
      if (_mesh.queryNodePtr(dof) &&
          (_mesh.nodeRef(dof).processor_id() == _subproblem.processor_id()) &&
          (dof != _master_node_vector[0]))
        _connected_nodes.push_back(dof);
    }
  }

  const auto & node_to_elem_map = _mesh.nodeToElemMap();
  auto node_to_elem_pair = node_to_elem_map.find(_master_node_vector[0]);

  bool found_elems = (node_to_elem_pair != node_to_elem_map.end());

  // Add elements connected to master node to Ghosted Elements.

  // On a distributed mesh, these elements might have already been
  // remoted, in which case we need to gather them back first.
  if (!_mesh.getMesh().is_serial())
  {
#ifndef NDEBUG
    bool someone_found_elems = found_elems;
    _mesh.getMesh().comm().max(someone_found_elems);
    mooseAssert(someone_found_elems, "Missing entry in node to elem map");
#endif

    std::set<Elem *, CompareElemsByLevel> master_elems_to_ghost;
    std::set<Node *> nodes_to_ghost;
    if (found_elems)
    {
      for (dof_id_type id : node_to_elem_pair->second)
      {
        Elem * elem = _mesh.queryElemPtr(id);
        if (elem)
        {
          master_elems_to_ghost.insert(elem);

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
                                                  mesh_inserter_iterator<Node>(_mesh.getMesh()));

    _mesh.getMesh().comm().allgather_packed_range(&_mesh.getMesh(),
                                                  master_elems_to_ghost.begin(),
                                                  master_elems_to_ghost.end(),
                                                  mesh_inserter_iterator<Elem>(_mesh.getMesh()));

    _mesh.update(); // Rebuild node_to_elem_map

    // Find elems again now that we know they're there
    const auto & new_node_to_elem_map = _mesh.nodeToElemMap();
    node_to_elem_pair = new_node_to_elem_map.find(_master_node_vector[0]);
    found_elems = (node_to_elem_pair != new_node_to_elem_map.end());
  }

  if (!found_elems)
    mooseError("Couldn't find any elements connected to master node");

  const std::vector<dof_id_type> & elems = node_to_elem_pair->second;

  if (elems.size() == 0)
    mooseError("Couldn't find any elements connected to master node");
  _subproblem.addGhostedElem(elems[0]);
}

Real
EqualValueBoundaryConstraint::computeQpResidual(Moose::ConstraintType type)
{
  switch (type)
  {
    case Moose::Slave:
      return (_u_secondary[_i] - _u_master[_j]) * _penalty;
    case Moose::Master:
      return (_u_master[_j] - _u_secondary[_i]) * _penalty;
  }
  return 0.;
}

Real
EqualValueBoundaryConstraint::computeQpJacobian(Moose::ConstraintJacobianType type)
{
  switch (type)
  {
    case Moose::SlaveSlave:
      return _penalty;
    case Moose::SlaveMaster:
      return -_penalty;
    case Moose::MasterMaster:
      return _penalty;
    case Moose::MasterSlave:
      return -_penalty;
    default:
      mooseError("Unsupported type");
      break;
  }
  return 0.;
}
