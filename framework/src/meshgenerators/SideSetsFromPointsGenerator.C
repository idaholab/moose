//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SideSetsFromPointsGenerator.h"
#include "Parser.h"
#include "InputParameters.h"
#include "MooseMeshUtils.h"
#include "CastUniquePointer.h"

#include "libmesh/mesh_generation.h"
#include "libmesh/mesh.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/quadrature_gauss.h"
#include "libmesh/point_locator_base.h"
#include "libmesh/enum_point_locator_type.h"
#include "libmesh/distributed_mesh.h"
#include "libmesh/elem.h"
#include "libmesh/fe_base.h"

#include <typeinfo>

registerMooseObject("MooseApp", SideSetsFromPointsGenerator);

InputParameters
SideSetsFromPointsGenerator::validParams()
{
  InputParameters params = SideSetsGeneratorBase::validParams();

  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  params.addClassDescription("Adds a new sideset starting at the specified point containing all "
                             "connected element faces with the same normal.");
  params.addRequiredParam<std::vector<BoundaryName>>("new_boundary",
                                                     "The name of the boundary to create");
  params.addRequiredParam<std::vector<Point>>(
      "points", "A list of points from which to start painting sidesets");

  return params;
}

SideSetsFromPointsGenerator::SideSetsFromPointsGenerator(const InputParameters & parameters)
  : SideSetsGeneratorBase(parameters),
    _input(getMesh("input")),
    _boundary_names(getParam<std::vector<BoundaryName>>("new_boundary")),
    _points(getParam<std::vector<Point>>("points"))
{
  if (_points.size() != _boundary_names.size())
    mooseError("point list and boundary list are not the same length");
}

std::unique_ptr<MeshBase>
SideSetsFromPointsGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);
  if (!mesh->is_replicated())
    mooseError("SideSetsFromPointsGenerator is not implemented for distributed meshes");

  // Get the BoundaryIDs from the mesh
  std::vector<BoundaryID> boundary_ids =
      MooseMeshUtils::getBoundaryIDs(*mesh, _boundary_names, true);

  setup(*mesh);

  _visited.clear();

  std::unique_ptr<PointLocatorBase> pl = PointLocatorBase::build(TREE, *mesh);

  for (unsigned int i = 0; i < boundary_ids.size(); ++i)
  {
    const Elem * elem = (*pl)(_points[i]);

    for (unsigned int side = 0; side < elem->n_sides(); ++side)
    {
      if (elem->neighbor_ptr(side))
        continue;

      // See if this point is on this side
      std::unique_ptr<const Elem> elem_side = elem->side_ptr(side);

      if (elem_side->contains_point(_points[i]))
      {
        // This is the side that we want to paint our sideset with
        // First get the normal
        const std::vector<Point> & normals = _fe_face->get_normals();
        _fe_face->reinit(elem, side);

        flood(elem, normals[0], boundary_ids[i], *mesh);
      }
    }
  }

  finalize();

  for (unsigned int i = 0; i < boundary_ids.size(); ++i)
    mesh->get_boundary_info().sideset_name(boundary_ids[i]) = _boundary_names[i];

  mesh->set_isnt_prepared();
  return dynamic_pointer_cast<MeshBase>(mesh);
}
