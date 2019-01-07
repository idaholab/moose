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

#include <typeinfo>

registerMooseObject("MooseApp", StackGenerator);

template <>
InputParameters
validParams<StackGenerator>()
{
  InputParameters params = validParams<MeshGenerator>();

  params.addRequiredParam<std::vector<MeshGeneratorName>>("inputs", "The meshes we want to stitch together");

  params.addParam<Real>("bottom_height", 0, "The height of the bottom of the final mesh");

  // z boundary names
  params.addParam<BoundaryName>("front_boundary", "front", "name of the front (z) boundary");
  params.addParam<BoundaryName>("back_boundary", "back", "name of the back (z) boundary");

  params.addClassDescription("Use the supplied meshes and stitch them on top of each other");

  return params;
}

StackGenerator::StackGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input_names(getParam<std::vector<MeshGeneratorName>>("inputs")),
    _bottom_height(getParam<Real>("bottom_height"))
{
  // Grab the input meshes
  _mesh_ptrs.reserve(_input_names.size());
  for (auto i = beginIndex(_input_names); i < _input_names.size(); ++i)
    _mesh_ptrs.push_back(&getMeshByName(_input_names[i]));


      
}

std::unique_ptr<MeshBase>
StackGenerator::generate()
{
  std::unique_ptr<ReplicatedMesh> mesh = dynamic_pointer_cast<ReplicatedMesh>(*_mesh_ptrs[0]);

  if (mesh->mesh_dimension() != 3 )
    mooseError("The first mesh is not in 3D !");

  // Reserve spaces for the other meshes (no need to store the first one another time)
  _meshes.reserve(_input_names.size() - 1);

  // Read in all of the other meshes
  for (auto i = beginIndex(_input_names, 1); i < _input_names.size(); ++i)
    _meshes.push_back(dynamic_pointer_cast<ReplicatedMesh>(*_mesh_ptrs[i]));
  
  // Check that we have 3D meshes
  for (auto i = beginIndex(_meshes); i < _meshes.size(); ++i)
    if (_meshes[i]->mesh_dimension() != 3)
      mooseError("Mesh from MeshGenerator : ", _input_names[i+1], " is not in 3D.");

  boundary_id_type front =
      mesh->get_boundary_info().get_id_by_name(getParam<BoundaryName>("front_boundary"));
  boundary_id_type back =
      mesh->get_boundary_info().get_id_by_name(getParam<BoundaryName>("back_boundary"));



  // Getting the z width of each mesh
  std::vector<Real> z_heights;
  z_heights.push_back(zWidth(*mesh) + _bottom_height);
  for (auto i = beginIndex(_meshes); i < _meshes.size(); ++i)
    z_heights.push_back(zWidth(*_meshes[i]) + *z_heights.rbegin());

  MeshTools::Modification::translate(*mesh, 0, 0, _bottom_height);

  for (auto i = beginIndex(_meshes); i < _meshes.size(); ++i)
  {
    MeshTools::Modification::translate(*_meshes[i], 0, 0, z_heights[i]);
    mesh->stitch_meshes(
			*_meshes[i], front, back, TOLERANCE, /*clear_stitched_boundary_ids=*/true);
  }

  return dynamic_pointer_cast<MeshBase>(mesh);
}

Real
StackGenerator::zWidth(const MeshBase & mesh)
{
  std::set<subdomain_id_type> sub_ids;
  mesh.subdomain_ids(sub_ids);
  BoundingBox bbox(Point(std::numeric_limits<Real>::max(), std::numeric_limits<Real>::max(), std::numeric_limits<Real>::max()),
		   Point(std::numeric_limits<Real>::lowest(), std::numeric_limits<Real>::lowest(), std::numeric_limits<Real>::lowest()));
  for (auto id : sub_ids)
  {
    BoundingBox sub_bbox = MeshTools::create_subdomain_bounding_box(mesh, id);
    bbox.union_with(sub_bbox);
  }
  
  return bbox.max()(2) - bbox.min()(2);
}
