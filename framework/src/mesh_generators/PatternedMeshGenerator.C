//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PatternedMeshGenerator.h"

#include "CastUniquePointer.h"

#include "libmesh/replicated_mesh.h"
#include "libmesh/boundary_info.h"

registerMooseObject("MooseApp", PatternedMeshGenerator);

template <>
InputParameters
validParams<PatternedMeshGenerator>()
{
  InputParameters params = validParams<MeshGenerator>();

  params.addRequiredParam<std::vector<MeshGeneratorName>>("inputs", "The input MeshGenerators.");
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

PatternedMeshGenerator::PatternedMeshGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input_names(getParam<std::vector<MeshGeneratorName>>("inputs")),
    _pattern(getParam<std::vector<std::vector<unsigned int>>>("pattern")),
    _x_width(getParam<Real>("x_width")),
    _y_width(getParam<Real>("y_width")),
    _z_width(getParam<Real>("z_width"))
{
  // The PatternedMesh class only works with ReplicatedMesh
  // Must find the equivalent : this is for MooseMesh derived objects
  //errorIfDistributedMesh("PatternedMesh");

  _meshes.reserve(_input_names.size());
  // Read in all of the meshes
  for (auto i = beginIndex(_input_names); i < _input_names.size(); ++i)
  {
    _meshes.emplace_back(libmesh_make_unique<ReplicatedMesh>(comm()));
    _meshes.back() = dynamic_pointer_cast<MeshBase, ReplicatedMesh>(getMeshByName(_input_names[i]));
  }

  _row_meshes.reserve(_pattern.size() - 1);
  for (auto i = beginIndex(_pattern); i < _pattern.size() - 1; ++i)
    _row_meshes.emplace_back(libmesh_make_unique<ReplicatedMesh>(comm()));
}

std::unique_ptr<MeshBase>
PatternedMeshGenerator::generate()
{
  std::unique_ptr<ReplicatedMesh> mesh = dynamic_pointer_cast<MeshBase, ReplicatedMesh>(getMeshByName(_input_names[_pattern[0][0]]));

  boundary_id_type left = mesh->get_id_by_name(getParam<BoundaryName>("left_boundary"));
  boundary_id_type right = mesh->get_id_by_name(getParam<BoundaryName>("right_boundary"));
  boundary_id_type top = mesh->get_id_by_name(getParam<BoundaryName>("top_boundary"));
  boundary_id_type bottom = mesh->get_id_by_name(getParam<BoundaryName>("bottom_boundary"));

  // Build the first row
  for (auto j = beginIndex(_pattern[0]); j < _pattern[0].size(); ++j)
  {
    Real deltax = j * _x_width;
    ReplicatedMesh & cell_mesh = *_meshes[_pattern[0][j]];

    // Move the mesh into the right spot.  -i because we are starting at the top
    MeshTools::Modification::translate(cell_mesh, deltax, 0, 0);

    mesh->stitch_meshes(cell_mesh,
                                   right,
                                   left,
                                   TOLERANCE,
                                   /*clear_stitched_boundary_ids=*/true);

    // Undo the translation
    MeshTools::Modification::translate(cell_mesh, -deltax, 0, 0);
  }
  
  // Build each row mesh
  for (auto i = beginIndex(_pattern,1); i < _pattern.size(); ++i)
    for (auto j = beginIndex(_pattern[i]); j < _pattern[i].size(); ++j)
    {
      Real deltax = j * _x_width, deltay = i * _y_width;

      // If this is the first cell of the row initialize the row mesh
      if (j == 0)
      {
        _row_meshes[i] = dynamic_pointer_cast<MeshBase, ReplicatedMesh>(getMeshByName(_input_names[_pattern[i][j]]));

        MeshTools::Modification::translate(*_row_meshes[i], deltax, -deltay, 0);

        continue;
      }

      ReplicatedMesh & cell_mesh = *_meshes[_pattern[i][j]];

      // Move the mesh into the right spot.  -i because we are starting at the top
      MeshTools::Modification::translate(cell_mesh, deltax, -deltay, 0);

      _row_meshes[i]->stitch_meshes(cell_mesh,
                                   right,
                                   left,
                                   TOLERANCE,
                                   /*clear_stitched_boundary_ids=*/true);

      // Undo the translation
      MeshTools::Modification::translate(cell_mesh, -deltax, deltay, 0);
    }

  // Now stitch together the rows
  // We're going to stitch them all to row 0 (which is the real mesh)
  for (auto i = beginIndex(_pattern, 1); i < _pattern.size(); i++)
    mesh->stitch_meshes(
        *_row_meshes[i], bottom, top, TOLERANCE, /*clear_stitched_boundary_ids=*/true);
  
  return mesh;
}
