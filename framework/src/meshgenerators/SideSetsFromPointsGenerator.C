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

#include "libmesh/mesh.h"
#include "libmesh/mesh_generation.h"
#include "libmesh/mesh_serializer.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/quadrature_gauss.h"
#include "libmesh/point_locator_base.h"
#include "libmesh/enum_point_locator_type.h"
#include "libmesh/distributed_mesh.h"
#include "libmesh/elem.h"
#include "libmesh/fe_base.h"
#include "libmesh/mesh_tools.h"

#include <typeinfo>

registerMooseObject("MooseApp", SideSetsFromPointsGenerator);

InputParameters
SideSetsFromPointsGenerator::validParams()
{
  InputParameters params = SideSetsGeneratorBase::validParams();

  params.addClassDescription("Adds a new sideset starting at the specified point containing all "
                             "connected element faces with the same normal.");
  params.addRequiredParam<std::vector<Point>>(
      "points", "A list of points from which to start painting sidesets");

  params.suppressParameter<Point>("normal");
  params.suppressParameter<Real>("normal_tol");

  // It doesn't make sense to allow internal sides for this side set generator.
  params.setParameters("include_only_external_sides", true);
  params.suppressParameter<bool>("include_only_external_sides");

  return params;
}

SideSetsFromPointsGenerator::SideSetsFromPointsGenerator(const InputParameters & parameters)
  : SideSetsGeneratorBase(parameters), _points(getParam<std::vector<Point>>("points"))
{
  if (_points.size() != _boundary_names.size())
    mooseError("point list and boundary list are not the same length");
  _using_normal = true;
}

std::unique_ptr<MeshBase>
SideSetsFromPointsGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  // Our flood fill doesn't do any communication, so it requires a
  // serialized mesh
  libMesh::MeshSerializer serial(*mesh);

  // Get the BoundaryIDs from the mesh
  std::vector<BoundaryID> boundary_ids =
      MooseMeshUtils::getBoundaryIDs(*mesh, _boundary_names, true);

  setup(*mesh);

  _visited.clear();

  std::unique_ptr<libMesh::PointLocatorBase> pl =
      libMesh::PointLocatorBase::build(libMesh::TREE, *mesh);

  for (const auto i : index_range(boundary_ids))
  {
    std::set<const Elem *> candidate_elements;
    (*pl)(_points[i], candidate_elements);

    const Elem * elem_to_flood = nullptr;
    Point normal_to_flood;

    for (const Elem * elem : candidate_elements)
      for (const auto side : make_range(elem->n_sides()))
      {
        if (elem->neighbor_ptr(side))
          continue;

        // See if this point is on this side
        std::unique_ptr<const Elem> elem_side = elem->side_ptr(side);

        if (elem_side->contains_point(_points[i]))
        {
          // This is a good side to paint our sideset with.
          // Get the normal.
          const std::vector<Point> & normals = _fe_face->get_normals();
          _fe_face->reinit(elem, side);

          // If we *already* found a good but different side to paint
          // our sideset with, we've got an ambiguity here.
          if (elem_to_flood && (!normalsWithinTol(normal_to_flood, normals[0], _normal_tol) ||
                                elem_to_flood->which_neighbor_am_i(elem) == libMesh::invalid_uint))
            mooseError("Two ambiguous potential sideset sources found for boundary `",
                       _boundary_names[i],
                       "' at ",
                       _points[i],
                       ":\nElement ",
                       elem_to_flood->id(),
                       " normal ",
                       normal_to_flood,
                       " and\n",
                       ":\nElement ",
                       elem->id(),
                       " normal ",
                       normals[0]);

          elem_to_flood = elem;
          normal_to_flood = normals[0];
          // Don't just flood here; keep looking for possible ambiguity
        }
      }

    flood(elem_to_flood, normal_to_flood, boundary_ids[i], *mesh);
  }

#ifdef DEBUG
  MeshTools::libmesh_assert_valid_boundary_ids(*mesh);
#endif

  finalize();

  for (const auto i : index_range(boundary_ids))
    mesh->get_boundary_info().sideset_name(boundary_ids[i]) = _boundary_names[i];

  mesh->set_isnt_prepared();
  return dynamic_pointer_cast<MeshBase>(mesh);
}
