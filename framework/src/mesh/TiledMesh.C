//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TiledMesh.h"
#include "Parser.h"
#include "InputParameters.h"

#include "libmesh/mesh_modification.h"
#include "libmesh/serial_mesh.h"
#include "libmesh/exodusII_io.h"

registerMooseObject("MooseApp", TiledMesh);

InputParameters
TiledMesh::validParams()
{
  InputParameters params = MooseMesh::validParams();
  params.addRequiredParam<MeshFileName>("file", "The name of the mesh file to read");

  params.addParam<Real>("x_width", 0, "The tile width in the x direction");
  params.addParam<Real>("y_width", 0, "The tile width in the y direction");
  params.addParam<Real>("z_width", 0, "The tile width in the z direction");

  // x boundary names
  params.addParam<BoundaryName>("left_boundary", "left_boundary", "name of the left (x) boundary");
  params.addParam<BoundaryName>(
      "right_boundary", "right_boundary", "name of the right (x) boundary");

  // y boundary names
  params.addParam<BoundaryName>("top_boundary", "top_boundary", "name of the top (y) boundary");
  params.addParam<BoundaryName>(
      "bottom_boundary", "bottom_boundary", "name of the bottom (y) boundary");

  // z boundary names
  params.addParam<BoundaryName>(
      "front_boundary", "front_boundary", "name of the front (z) boundary");
  params.addParam<BoundaryName>("back_boundary", "back_boundary", "name of the back (z) boundary");

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

TiledMesh::TiledMesh(const InputParameters & parameters)
  : MooseMesh(parameters),
    _x_width(getParam<Real>("x_width")),
    _y_width(getParam<Real>("y_width")),
    _z_width(getParam<Real>("z_width"))
{
  // The TiledMesh class only works with ReplicatedMesh
  errorIfDistributedMesh("TiledMesh");
}

TiledMesh::TiledMesh(const TiledMesh & other_mesh)
  : MooseMesh(other_mesh),
    _x_width(other_mesh._x_width),
    _y_width(other_mesh._y_width),
    _z_width(other_mesh._z_width)
{
}

std::unique_ptr<MooseMesh>
TiledMesh::safeClone() const
{
  return std::make_unique<TiledMesh>(*this);
}

std::string
TiledMesh::getFileName() const
{
  return getParam<MeshFileName>("file");
}

void
TiledMesh::buildMesh()
{
  // stitch_meshes() is only implemented for ReplicatedMesh.  So make sure
  // we have one here before continuing.
  ReplicatedMesh * serial_mesh = dynamic_cast<ReplicatedMesh *>(&getMesh());

  if (!serial_mesh)
    mooseError("Error, TiledMesh calls stitch_meshes() which only works on ReplicatedMesh.");
  else
  {
    std::string mesh_file(getParam<MeshFileName>("file"));

    if (mesh_file.rfind(".exd") < mesh_file.size() || mesh_file.rfind(".e") < mesh_file.size())
    {
      ExodusII_IO ex(*this);
      ex.read(mesh_file);
      serial_mesh->prepare_for_use();
    }
    else
      serial_mesh->read(mesh_file);

    BoundaryID left = getBoundaryID(getParam<BoundaryName>("left_boundary"));
    BoundaryID right = getBoundaryID(getParam<BoundaryName>("right_boundary"));
    BoundaryID top = getBoundaryID(getParam<BoundaryName>("top_boundary"));
    BoundaryID bottom = getBoundaryID(getParam<BoundaryName>("bottom_boundary"));
    BoundaryID front = getBoundaryID(getParam<BoundaryName>("front_boundary"));
    BoundaryID back = getBoundaryID(getParam<BoundaryName>("back_boundary"));

    {
      std::unique_ptr<MeshBase> clone = serial_mesh->clone();

      // Build X Tiles
      for (unsigned int i = 1; i < getParam<unsigned int>("x_tiles"); ++i)
      {
        MeshTools::Modification::translate(*clone, _x_width, 0, 0);
        serial_mesh->stitch_meshes(dynamic_cast<ReplicatedMesh &>(*clone),
                                   right,
                                   left,
                                   TOLERANCE,
                                   /*clear_stitched_boundary_ids=*/true);
      }
    }
    {
      std::unique_ptr<MeshBase> clone = serial_mesh->clone();

      // Build Y Tiles
      for (unsigned int i = 1; i < getParam<unsigned int>("y_tiles"); ++i)
      {
        MeshTools::Modification::translate(*clone, 0, _y_width, 0);
        serial_mesh->stitch_meshes(dynamic_cast<ReplicatedMesh &>(*clone),
                                   top,
                                   bottom,
                                   TOLERANCE,
                                   /*clear_stitched_boundary_ids=*/true);
      }
    }
    {
      std::unique_ptr<MeshBase> clone = serial_mesh->clone();

      // Build Z Tiles
      for (unsigned int i = 1; i < getParam<unsigned int>("z_tiles"); ++i)
      {
        MeshTools::Modification::translate(*clone, 0, 0, _z_width);
        serial_mesh->stitch_meshes(dynamic_cast<ReplicatedMesh &>(*clone),
                                   front,
                                   back,
                                   TOLERANCE,
                                   /*clear_stitched_boundary_ids=*/true);
      }
    }
  }
}
