//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SideSetsFromNormalsGenerator.h"
#include "Parser.h"
#include "InputParameters.h"
#include "MooseMeshUtils.h"
#include "CastUniquePointer.h"
#include "MooseApp.h"
#include "MeshGeneratorSystem.h"

#include "libmesh/mesh_generation.h"
#include "libmesh/mesh.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/quadrature_gauss.h"
#include "libmesh/point_locator_base.h"
#include "libmesh/distributed_mesh.h"
#include "libmesh/elem.h"
#include "libmesh/fe_base.h"

#include <typeinfo>

registerMooseObject("MooseApp", SideSetsFromNormalsGenerator);

InputParameters
SideSetsFromNormalsGenerator::validParams()
{
  InputParameters params = SideSetsGeneratorBase::validParams();

  params.addClassDescription(
      "Adds a new named sideset to the mesh for all faces matching the specified normal.");
  params.addRequiredParam<std::vector<Point>>(
      "normals", "A list of normals for which to start painting sidesets");
  params.addParam<Real>("tolerance", "Tolerance for comparing the face normal");
  params.deprecateParam("tolerance", "normal_tol", "4/01/2025");

  // We want to use a different normal_tol for this generator than from the base class to preserve
  // old behavior.
  params.setParameters("normal_tol", 1e-5);

  // We are using 'normals' instead
  params.suppressParameter<Point>("normal");

  // It doesn't make sense to allow internal sides for this side set generator.
  params.setParameters("include_only_external_sides", true);
  params.suppressParameter<bool>("include_only_external_sides");

  return params;
}

SideSetsFromNormalsGenerator::SideSetsFromNormalsGenerator(const InputParameters & parameters)
  : SideSetsGeneratorBase(parameters),
    _normals(getParam<std::vector<Point>>("normals")),
    _boundary_to_normal_map(
        declareMeshProperty<std::map<BoundaryID, RealVectorValue>>("boundary_normals"))
{
  // Get the BoundaryIDs from the mesh
  if (_normals.size() != _boundary_names.size())
    mooseError("normal list and boundary list are not the same length");

  // Make sure that the normals are normalized
  for (auto & normal : _normals)
  {
    if (normal.norm() < 1e-5)
      mooseError("Normal is zero");
    normal /= normal.norm();
  }

  _using_normal = true;
}

std::unique_ptr<MeshBase>
SideSetsFromNormalsGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);
  if (!_fixed_normal && !mesh->is_replicated())
    mooseError("SideSetsFromNormalsGenerator is not implemented for distributed meshes when "
               "fixed_normal = false");

  std::vector<BoundaryID> boundary_ids =
      MooseMeshUtils::getBoundaryIDs(*mesh, _boundary_names, true);

  setup(*mesh);

  _visited.clear();

  // Request to compute normal vectors
  const std::vector<Point> & face_normals = _fe_face->get_normals();

  // We'll need to loop over all of the elements to find ones that match this normal.
  // We can't rely on flood catching them all here...
  for (const auto & elem : mesh->element_ptr_range())
    for (const auto side : make_range(elem->n_sides()))
    {
      if (elem->neighbor_ptr(side))
        continue;

      _fe_face->reinit(elem, side);

      // We'll just use the normal of the first qp
      const Point & face_normal = face_normals[0];

      for (const auto i : make_range(boundary_ids.size()))
      {
        if (normalsWithinTol(_normals[i], face_normal, _normal_tol))
          flood(elem, _normals[i], boundary_ids[i], *mesh);
      }
    }

  finalize();

  BoundaryInfo & boundary_info = mesh->get_boundary_info();
  for (const auto i : make_range(boundary_ids.size()))
  {
    boundary_info.sideset_name(boundary_ids[i]) = _boundary_names[i];
    _boundary_to_normal_map[boundary_ids[i]] = _normals[i];
  }

  // This is a terrible hack that we'll want to remove once BMBBG isn't terrible
  if (!_app.getMeshGeneratorSystem().hasBreakMeshByBlockGenerator())
    mesh->set_isnt_prepared();
  return dynamic_pointer_cast<MeshBase>(mesh);
}
