//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// App includes
#include "GhostHigherDLowerDPointNeighbors.h"
#include "Executioner.h"
#include "FEProblemBase.h"
#include "MooseApp.h"
#include "GhostLowerDElems.h"

// libMesh includes
#include "libmesh/elem.h"
#include "libmesh/mesh_base.h"
#include "libmesh/boundary_info.h"

registerMooseObject("MooseApp", GhostHigherDLowerDPointNeighbors);

using namespace libMesh;

InputParameters
GhostHigherDLowerDPointNeighbors::validParams()
{
  InputParameters params = RelationshipManager::validParams();
  params.set<bool>("attach_geometric_early") = false;
  return params;
}

GhostHigherDLowerDPointNeighbors::GhostHigherDLowerDPointNeighbors(const InputParameters & params)
  : RelationshipManager(params)
{
}

GhostHigherDLowerDPointNeighbors::GhostHigherDLowerDPointNeighbors(
    const GhostHigherDLowerDPointNeighbors & other)
  : RelationshipManager(other)
{
}

std::string
GhostHigherDLowerDPointNeighbors::getInfo() const
{
  std::ostringstream oss;
  oss << "GhostHigherDLowerDPointNeighbors";
  return oss.str();
}

void
GhostHigherDLowerDPointNeighbors::operator()(const MeshBase::const_element_iterator & range_begin,
                                             const MeshBase::const_element_iterator & range_end,
                                             const processor_id_type p,
                                             map_type & coupled_elements)
{
  mooseAssert(_moose_mesh,
              "The MOOSE mesh must be non-null in order for this relationship manager to work.");
  const auto & node_to_elem_map = _moose_mesh->nodeToElemMap();

  static const CouplingMatrix * const null_mat = nullptr;
  const auto mesh_dim = _mesh->mesh_dimension();
  std::unordered_set<dof_id_type> visited_nodes;

  for (const Elem * const elem : as_range(range_begin, range_end))
  {
    if (elem->dim() != mesh_dim)
      // not a higher-dimensional element
      continue;

    for (const auto & node : elem->node_ref_range())
    {
      // We want to ghost the adjoining lower-d elements if the node hasn't been partitioned yet
      // (e.g. we don't know what processes it will be) or if its processor id matches our
      // processes's id
      const auto node_pid = node.processor_id();
      if (node_pid == p || node_pid == DofObject::invalid_processor_id)
      {
        const auto [_, inserted] = visited_nodes.insert(node.id());
        if (!inserted)
          // We've already considered this node
          continue;

        const auto & elem_node_neighbors = libmesh_map_find(node_to_elem_map, node.id());
        for (const auto elem_id : elem_node_neighbors)
        {
          const Elem * const elem_node_neighbor = _mesh->elem_ptr(elem_id);
          if (elem_node_neighbor->dim() != mesh_dim && elem_node_neighbor->processor_id() != p)
            coupled_elements.emplace(elem_node_neighbor, null_mat);
        }
      }
    }
  }
}

bool
GhostHigherDLowerDPointNeighbors::operator>=(const RelationshipManager & other) const
{
  return dynamic_cast<const GhostHigherDLowerDPointNeighbors *>(&other) ||
         dynamic_cast<const GhostLowerDElems *>(&other);
}

std::unique_ptr<GhostingFunctor>
GhostHigherDLowerDPointNeighbors::clone() const
{
  return _app.getFactory().copyConstruct(*this);
}
