//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddAllSideSetsByNormals.h"
#include "Parser.h"
#include "InputParameters.h"
#include "MooseMesh.h"

#include "libmesh/mesh_generation.h"
#include "libmesh/mesh.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/quadrature_gauss.h"
#include "libmesh/point_locator_base.h"

template <>
InputParameters
validParams<AddAllSideSetsByNormals>()
{
  InputParameters params = validParams<AddSideSetsBase>();
  return params;
}

AddAllSideSetsByNormals::AddAllSideSetsByNormals(const InputParameters & parameters)
  : AddSideSetsBase(parameters)
{
}

void
AddAllSideSetsByNormals::modify()
{
  setup();

  if (!_mesh_ptr)
    mooseError("_mesh_ptr must be initialized before calling AddAllSideSetsByNormals::modify()!");

  // We can't call this in the constructor, it appears that _mesh_ptr is always NULL there.
  _mesh_ptr->errorIfDistributedMesh("AddAllSideSetsByNormals");

  // Get the current list of boundaries so we can generate new ones that won't conflict
  _mesh_boundary_ids = _mesh_ptr->meshBoundaryIds();

  // Create the map object that will be owned by MooseMesh
  using map_type = std::map<BoundaryID, RealVectorValue>;
  std::unique_ptr<map_type> boundary_map = libmesh_make_unique<map_type>();

  _visited.clear();

  // We'll need to loop over all of the elements to find ones that match this normal.
  // We can't rely on flood catching them all here...
  for (const auto & elem : _mesh_ptr->getMesh().element_ptr_range())
    for (unsigned int side = 0; side < elem->n_sides(); ++side)
    {
      if (elem->neighbor_ptr(side))
        continue;

      _fe_face->reinit(elem, side);
      const std::vector<Point> & normals = _fe_face->get_normals();

      {
        // See if we've seen this normal before (linear search)
        const map_type::value_type * item = nullptr;
        for (const auto & id_pair : *boundary_map)
          if (std::abs(1.0 - id_pair.second * normals[0]) < 1e-5)
          {
            item = &id_pair;
            break;
          }

        if (item)
          flood(elem, normals[0], item->first);
        else
        {
          BoundaryID id = getNextBoundaryID();
          (*boundary_map)[id] = normals[0];
          flood(elem, normals[0], id);
        }
      }
    }

  finalize();

  // Transfer ownership of the boundary map and boundary ID set.
  _mesh_ptr->setBoundaryToNormalMap(std::move(boundary_map));
  _mesh_ptr->setMeshBoundaryIDs(_mesh_boundary_ids);
}

BoundaryID
AddAllSideSetsByNormals::getNextBoundaryID()
{
  std::set<BoundaryID>::iterator it;
  BoundaryID next_id = 1;

  while ((it = _mesh_boundary_ids.find(next_id)) != _mesh_boundary_ids.end())
    ++next_id;

  _mesh_boundary_ids.insert(next_id);

  return next_id;
}
