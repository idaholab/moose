//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PatternedMesh.h"
#include "Parser.h"
#include "InputParameters.h"

#include "libmesh/mesh_modification.h"
#include "libmesh/serial_mesh.h"
#include "libmesh/exodusII_io.h"

registerMooseObject("MooseApp", PatternedMesh);

InputParameters
PatternedMesh::validParams()
{
  InputParameters params = MooseMesh::validParams();
  params.addRequiredParam<std::vector<MeshFileName>>("files",
                                                     "The name of the mesh files to read. "
                                                     " They are automatically assigned "
                                                     "ids starting with zero.");

  params.addRangeCheckedParam<Real>(
      "x_width", 0, "x_width>=0", "The tile width in the x direction");
  params.addRangeCheckedParam<Real>(
      "y_width", 0, "y_width>=0", "The tile width in the y direction");
  params.addRangeCheckedParam<Real>(
      "z_width", 0, "z_width>=0", "The tile width in the z direction");

  // x boundary names
  params.addParam<BoundaryName>("left_boundary", "left_boundary", "name of the left (x) boundary");
  params.addParam<BoundaryName>(
      "right_boundary", "right_boundary", "name of the right (x) boundary");

  // y boundary names
  params.addParam<BoundaryName>("top_boundary", "top_boundary", "name of the top (y) boundary");
  params.addParam<BoundaryName>(
      "bottom_boundary", "bottom_boundary", "name of the bottom (y) boundary");

  params.addRequiredParam<std::vector<std::vector<unsigned int>>>(
      "pattern", "A double-indexed array starting with the upper-left corner");

  params.addClassDescription("Creates a 2D mesh from a specified set of unique 'tiles' meshes and "
                             "a two-dimensional pattern.");

  return params;
}

PatternedMesh::PatternedMesh(const InputParameters & parameters)
  : MooseMesh(parameters),
    _files(getParam<std::vector<MeshFileName>>("files")),
    _pattern(getParam<std::vector<std::vector<unsigned int>>>("pattern")),
    _x_width(getParam<Real>("x_width")),
    _y_width(getParam<Real>("y_width")),
    _z_width(getParam<Real>("z_width"))
{
  // The PatternedMesh class only works with ReplicatedMesh
  errorIfDistributedMesh("PatternedMesh");

  _meshes.reserve(_files.size());
}

PatternedMesh::PatternedMesh(const PatternedMesh & other_mesh)
  : MooseMesh(other_mesh),
    _files(other_mesh._files),
    _pattern(other_mesh._pattern),
    _x_width(other_mesh._x_width),
    _y_width(other_mesh._y_width),
    _z_width(other_mesh._z_width)
{
}

std::unique_ptr<MooseMesh>
PatternedMesh::safeClone() const
{
  return std::make_unique<PatternedMesh>(*this);
}

void
PatternedMesh::buildMesh()
{
  // Read in all of the meshes
  for (MooseIndex(_files) i = 0; i < _files.size(); ++i)
  {
    _meshes.emplace_back(std::make_unique<ReplicatedMesh>(_communicator));
    auto & mesh = _meshes.back();

    mesh->read(_files[i]);
  }

  // Create a mesh for all n-1 rows, the first row is the original mesh
  _row_meshes.reserve(_pattern.size() - 1);
  for (MooseIndex(_pattern) i = 0; i < _pattern.size() - 1; ++i)
    _row_meshes.emplace_back(std::make_unique<ReplicatedMesh>(_communicator));

  // Local pointers to simplify algorithm
  std::vector<ReplicatedMesh *> row_meshes;
  row_meshes.reserve(_pattern.size());
  // First row is the original mesh
  row_meshes.push_back(static_cast<ReplicatedMesh *>(&getMesh()));
  // Copy the remaining raw pointers into the local vector
  for (const auto & row_mesh : _row_meshes)
    row_meshes.push_back(row_mesh.get());

  BoundaryID left = getBoundaryID(getParam<BoundaryName>("left_boundary"));
  BoundaryID right = getBoundaryID(getParam<BoundaryName>("right_boundary"));
  BoundaryID top = getBoundaryID(getParam<BoundaryName>("top_boundary"));
  BoundaryID bottom = getBoundaryID(getParam<BoundaryName>("bottom_boundary"));

  // Build each row mesh
  for (MooseIndex(_pattern) i = 0; i < _pattern.size(); ++i)
    for (MooseIndex(_pattern) j = 0; j < _pattern[i].size(); ++j)
    {
      Real deltax = j * _x_width, deltay = i * _y_width;

      // If this is the first cell of the row initialize the row mesh
      if (j == 0)
      {
        row_meshes[i]->read(_files[_pattern[i][j]]);

        MeshTools::Modification::translate(*row_meshes[i], deltax, -deltay, 0);

        continue;
      }

      ReplicatedMesh & cell_mesh = *_meshes[_pattern[i][j]];

      // Move the mesh into the right spot.  -i because we are starting at the top
      MeshTools::Modification::translate(cell_mesh, deltax, -deltay, 0);

      row_meshes[i]->stitch_meshes(dynamic_cast<ReplicatedMesh &>(cell_mesh),
                                   right,
                                   left,
                                   TOLERANCE,
                                   /*clear_stitched_boundary_ids=*/true);

      // Undo the translation
      MeshTools::Modification::translate(cell_mesh, -deltax, deltay, 0);
    }

  // Now stitch together the rows
  // We're going to stitch them all to row 0 (which is the real mesh)
  for (MooseIndex(_pattern) i = 1; i < _pattern.size(); i++)
    row_meshes[0]->stitch_meshes(
        *row_meshes[i], bottom, top, TOLERANCE, /*clear_stitched_boundary_ids=*/true);
}
