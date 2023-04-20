//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AllSideSetsByNormalsGenerator.h"
#include "Parser.h"
#include "InputParameters.h"
#include "CastUniquePointer.h"

#include "libmesh/fe_base.h"
#include "libmesh/mesh_generation.h"
#include "libmesh/mesh.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/quadrature_gauss.h"
#include "libmesh/point_locator_base.h"
#include "libmesh/distributed_mesh.h"
#include "libmesh/elem.h"

#include <typeinfo>

registerMooseObject("MooseApp", AllSideSetsByNormalsGenerator);

InputParameters
AllSideSetsByNormalsGenerator::validParams()
{
  InputParameters params = SideSetsGeneratorBase::validParams();

  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  params.addClassDescription("Adds sidesets to the entire mesh based on unique normals.");
  return params;
}

AllSideSetsByNormalsGenerator::AllSideSetsByNormalsGenerator(const InputParameters & parameters)
  : SideSetsGeneratorBase(parameters),
    _input(getMesh("input")),
    _boundary_to_normal_map(
        declareMeshProperty<std::map<BoundaryID, RealVectorValue>>("boundary_normals"))
{
}

std::unique_ptr<MeshBase>
AllSideSetsByNormalsGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);
  if (!mesh->is_replicated())
    mooseError("AllSideSetsByNormalsGenerator is not implemented for distributed meshes");
  setup(*mesh);

  // Get the current list of boundaries so we can generate new ones that won't conflict
  _mesh_boundary_ids = mesh->get_boundary_info().get_boundary_ids();

  _visited.clear();

  // We'll need to loop over all of the elements to find ones that match this normal.
  // We can't rely on flood catching them all here...
  for (const auto & elem : mesh->element_ptr_range())
    for (unsigned int side = 0; side < elem->n_sides(); ++side)
    {
      if (elem->neighbor_ptr(side))
        continue;

      const std::vector<Point> & normals = _fe_face->get_normals();
      _fe_face->reinit(elem, side);

      {
        // See if we've seen this normal before (linear search)
        const std::map<BoundaryID, RealVectorValue>::value_type * item = nullptr;
        for (const auto & id_pair : _boundary_to_normal_map)
          if (std::abs(1.0 - id_pair.second * normals[0]) < 1e-5)
          {
            item = &id_pair;
            break;
          }

        if (item)
          flood(elem, normals[0], item->first, *mesh);
        else
        {
          boundary_id_type id = getNextBoundaryID();
          _boundary_to_normal_map[id] = normals[0];
          flood(elem, normals[0], id, *mesh);
        }
      }
    }

  finalize();

  mesh->set_isnt_prepared();
  return dynamic_pointer_cast<MeshBase>(mesh);
}

boundary_id_type
AllSideSetsByNormalsGenerator::getNextBoundaryID()
{
  std::set<boundary_id_type>::iterator it;
  boundary_id_type next_id = 1;

  while ((it = _mesh_boundary_ids.find(next_id)) != _mesh_boundary_ids.end())
    ++next_id;

  _mesh_boundary_ids.insert(next_id);

  return next_id;
}
