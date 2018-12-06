//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TiledMeshGenerator.h"
#include "CastUniquePointer.h"

#include "libmesh/replicated_mesh.h"
#include "libmesh/distributed_mesh.h"
#include "libmesh/boundary_info.h"
#include "libmesh/mesh_modification.h"

#include <typeinfo>

registerMooseObject("MooseApp", TiledMeshGenerator);

template <>
InputParameters
validParams<TiledMeshGenerator>()
{
  InputParameters params = validParams<MeshGenerator>();

  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to repeat");

  params.addParam<Real>("x_width", 0, "The tile width in the x direction");
  params.addParam<Real>("y_width", 0, "The tile width in the y direction");
  params.addParam<Real>("z_width", 0, "The tile width in the z direction");

  // x boundary names
  params.addParam<BoundaryName>("left_boundary", "left", "name of the left (x) boundary");
  params.addParam<BoundaryName>("right_boundary", "right", "name of the right (x) boundary");

  // y boundary names
  params.addParam<BoundaryName>("top_boundary", "top", "name of the top (y) boundary");
  params.addParam<BoundaryName>("bottom_boundary", "bottom", "name of the bottom (y) boundary");

  // z boundary names
  params.addParam<BoundaryName>("front_boundary", "front", "name of the front (z) boundary");
  params.addParam<BoundaryName>("back_boundary", "back", "name of the back (z) boundary");

  // The number of tiles is 1 in each direction unless otherwise specified.
  // An x_tiles value of 1 means do not stitch any extra meshes together in
  // the x-direction.
  params.addParam<unsigned int>(
      "x_tiles", 1, "Number of tiles to stitch together (left to right) in the x-direction");
  params.addParam<unsigned int>(
      "y_tiles", 1, "Number of tiles to stitch together (top to bottom) in the y-direction");
  params.addParam<unsigned int>(
      "z_tiles", 1, "Number of tiles to stitch together (front to back) in the z-direction");

  params.addClassDescription("Use the supplied mesh and create a tiled grid by repeating this mesh "
                             "in the x,y, and z directions.");

  return params;
}

TiledMeshGenerator::TiledMeshGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _x_width(getParam<Real>("x_width")),
    _y_width(getParam<Real>("y_width")),
    _z_width(getParam<Real>("z_width"))
{
  if (typeid(_input).name() == typeid(DistributedMesh).name())
    mooseError("TiledMesh only works with ReplicatedMesh.");
}

std::unique_ptr<MeshBase>
TiledMeshGenerator::generate()
{
  std::unique_ptr<MeshBase> initial_mesh = std::move(_input);
  std::unique_ptr<ReplicatedMesh> mesh = dynamic_pointer_cast<ReplicatedMesh>(initial_mesh);

  boundary_id_type left =
      mesh->get_boundary_info().get_id_by_name(getParam<BoundaryName>("left_boundary"));
  boundary_id_type right =
      mesh->get_boundary_info().get_id_by_name(getParam<BoundaryName>("right_boundary"));
  boundary_id_type top =
      mesh->get_boundary_info().get_id_by_name(getParam<BoundaryName>("top_boundary"));
  boundary_id_type bottom =
      mesh->get_boundary_info().get_id_by_name(getParam<BoundaryName>("bottom_boundary"));
  boundary_id_type front =
      mesh->get_boundary_info().get_id_by_name(getParam<BoundaryName>("front_boundary"));
  boundary_id_type back =
      mesh->get_boundary_info().get_id_by_name(getParam<BoundaryName>("back_boundary"));

  {
    std::unique_ptr<MeshBase> clone = mesh->clone();

    // Build X Tiles
    for (unsigned int i = 1; i < getParam<unsigned int>("x_tiles"); ++i)
    {
      MeshTools::Modification::translate(*clone, _x_width, 0, 0);
      mesh->stitch_meshes(dynamic_cast<ReplicatedMesh &>(*clone),
                          right,
                          left,
                          TOLERANCE,
                          /*clear_stitched_boundary_ids=*/true);
    }
  }

  {
    std::unique_ptr<MeshBase> clone = mesh->clone();

    // Build Y Tiles
    for (unsigned int i = 1; i < getParam<unsigned int>("y_tiles"); ++i)
    {
      MeshTools::Modification::translate(*clone, 0, _y_width, 0);
      mesh->stitch_meshes(dynamic_cast<ReplicatedMesh &>(*clone),
                          top,
                          bottom,
                          TOLERANCE,
                          /*clear_stitched_boundary_ids=*/true);
    }
  }

  {
    std::unique_ptr<MeshBase> clone = mesh->clone();

    // Build Z Tiles
    for (unsigned int i = 1; i < getParam<unsigned int>("z_tiles"); ++i)
    {
      MeshTools::Modification::translate(*clone, 0, 0, _z_width);
      mesh->stitch_meshes(dynamic_cast<ReplicatedMesh &>(*clone),
                          front,
                          back,
                          TOLERANCE,
                          /*clear_stitched_boundary_ids=*/true);
    }
  }

  return dynamic_pointer_cast<MeshBase>(mesh);
}
