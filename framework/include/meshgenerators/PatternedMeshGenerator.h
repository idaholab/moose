//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MeshGenerator.h"
#include "libmesh/replicated_mesh.h"

// Forward declarations
class PatternedMeshGenerator;

template <>
InputParameters validParams<PatternedMeshGenerator>();

/**
 * Reads one or more 2D mesh files and stitches them together based on
 * a provided two-dimensional pattern array.  Assigns new boundary
 * ids/names to the left, right, top, and bottom boundaries as
 * specified by user. The boundary nodes of the read in meshes must
 * match up relative to the stitching pattern specified by the user --
 * no new nodes are added in order to generate a conforming grid, and
 * non-conforming grids (with nodes in the middle of edges) are not
 * allowed.
 */
class PatternedMeshGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  PatternedMeshGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// The mesh generators to read
  const std::vector<MeshGeneratorName> & _input_names;

  /// The pattern, starting with the upper left corner
  const std::vector<std::vector<unsigned int>> & _pattern;

  /// Holds pointers to the meshes before they are generated
  std::vector<std::unique_ptr<MeshBase> *> _mesh_ptrs;

  /// Holds the pointers to the input generated meshes
  std::vector<std::unique_ptr<ReplicatedMesh>> _meshes;

  /// Holds a mesh for each row, these will be stitched together in the end
  std::vector<std::unique_ptr<ReplicatedMesh>> _row_meshes;

  Real _x_width;
  Real _y_width;
  Real _z_width;
};
