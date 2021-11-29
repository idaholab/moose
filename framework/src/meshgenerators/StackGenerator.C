//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "StackGenerator.h"
#include "CastUniquePointer.h"

#include "libmesh/replicated_mesh.h"
#include "libmesh/distributed_mesh.h"
#include "libmesh/boundary_info.h"
#include "libmesh/mesh_modification.h"
#include "libmesh/bounding_box.h"
#include "libmesh/mesh_tools.h"
#include "libmesh/point.h"
#include "MooseMeshUtils.h"

#include <typeinfo>

registerMooseObject("MooseApp", StackGenerator);

InputParameters
StackGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  MooseEnum dims("2=2 3=3");
  params.addRequiredParam<MooseEnum>("dim", dims, "The dimension of the mesh to be generated");

  params.addRequiredParam<std::vector<MeshGeneratorName>>("inputs",
                                                          "The meshes we want to stitch together");

  params.addParam<Real>("bottom_height", 0, "The height of the bottom of the final mesh");

  // y boundary names (2D case)
  params.addParam<BoundaryName>("top_boundary", "top", "name of the top (y) boundary");
  params.addParam<BoundaryName>("bottom_boundary", "bottom", "name of the bottom (y) boundary");

  // z boundary names (3D case)
  params.addParam<BoundaryName>("front_boundary", "front", "name of the front (z) boundary");
  params.addParam<BoundaryName>("back_boundary", "back", "name of the back (z) boundary");

  params.addClassDescription("Use the supplied meshes and stitch them on top of each other");

  return params;
}

StackGenerator::StackGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _dim(getParam<MooseEnum>("dim")),
    _mesh_ptrs(getMeshes("inputs")),
    _input_names(getParam<std::vector<MeshGeneratorName>>("inputs")),
    _bottom_height(getParam<Real>("bottom_height"))
{
}

std::unique_ptr<MeshBase>
StackGenerator::generate()
{
  std::unique_ptr<ReplicatedMesh> mesh = dynamic_pointer_cast<ReplicatedMesh>(*_mesh_ptrs[0]);
  if (mesh == nullptr)
    mooseError("StackGenerator only works with ReplicatedMesh : mesh from Meshgenerator ",
               _input_names[0],
               "is not a ReplicatedMesh.");

  int dim = static_cast<int>(_dim);

  if (dim != int(mesh->mesh_dimension()))
    paramError("dim",
               "incompatible mesh dimensions: dim=",
               dim,
               " and first mesh dimension is ",
               mesh->mesh_dimension());

  // Reserve spaces for the other meshes (no need to store the first one another time)
  _meshes.reserve(_input_names.size() - 1);

  // Read in all of the other meshes
  for (MooseIndex(_input_names) i = 1; i < _input_names.size(); ++i)
    _meshes.push_back(dynamic_pointer_cast<ReplicatedMesh>(*_mesh_ptrs[i]));

  // Check that the casts didn't fail, and that the dimensions match
  for (MooseIndex(_meshes) i = 0; i < _meshes.size(); ++i)
  {
    if (_meshes[i] == nullptr)
      mooseError("StackGenerator only works with ReplicatedMesh : mesh from Meshgenerator ",
                 _input_names[i + 1],
                 "is not a ReplicatedMesh.");
    if (static_cast<int>(_meshes[i]->mesh_dimension()) != dim)
      mooseError("Mesh from MeshGenerator : ", _input_names[i + 1], " is not in ", _dim, "D.");
  }

  // Getting the boundaries provided by the user
  std::vector<BoundaryName> boundary_names = {getParam<BoundaryName>("top_boundary"),
                                              getParam<BoundaryName>("bottom_boundary")};
  if (dim == 3)
    boundary_names = {getParam<BoundaryName>("front_boundary"),
                      getParam<BoundaryName>("back_boundary")};

  std::vector<boundary_id_type> ids =
      MooseMeshUtils::getBoundaryIDs(*_meshes[0], boundary_names, true);

  mooseAssert(ids.size() == boundary_names.size(),
              "Unexpected number of ids returned for MooseMeshUtils::getBoundaryIDs");

  boundary_id_type first = ids[0], second = ids[1];

  // Getting the width of each mesh
  std::vector<Real> heights;
  heights.push_back(computeWidth(*mesh, _dim) + _bottom_height);
  for (MooseIndex(_meshes) i = 0; i < _meshes.size(); ++i)
    heights.push_back(computeWidth(*_meshes[i], _dim) + *heights.rbegin());

  // Move the first mesh at the provided height
  switch (_dim)
  {
    case 2:
      MeshTools::Modification::translate(*mesh, 0, _bottom_height, 0);
      break;
    case 3:
      MeshTools::Modification::translate(*mesh, 0, 0, _bottom_height);
      break;
  }

  // Move all of the other meshes in the right spots then stitch them one by one to the first one
  for (MooseIndex(_meshes) i = 0; i < _meshes.size(); ++i)
  {
    switch (_dim)
    {
      case 2:
        MeshTools::Modification::translate(*_meshes[i], 0, heights[i], 0);
        break;
      case 3:
        MeshTools::Modification::translate(*_meshes[i], 0, 0, heights[i]);
        break;
    }
    mesh->stitch_meshes(
        *_meshes[i], first, second, TOLERANCE, /*clear_stitched_boundary_ids=*/true);
  }

  return dynamic_pointer_cast<MeshBase>(mesh);
}

Real
StackGenerator::computeWidth(const MeshBase & mesh, const int & dim)
{
  BoundingBox bbox = MeshTools::create_bounding_box(mesh);
  return bbox.max()(dim - 1) - bbox.min()(dim - 1);
}
