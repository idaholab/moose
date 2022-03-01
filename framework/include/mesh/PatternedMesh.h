//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseMesh.h"

namespace libMesh
{
class ReplicatedMesh;
}

/**
 * Reads one or more 2D mesh files and stitches them together based on
 * a provided two-dimensional pattern array.  Assigns new boundary
 * ids/names to the left, right, top, and bottom boundaries as
 * specified by user.  The boundary nodes of the read in meshes must
 * match up relative to the stitching pattern specified by the user --
 * no new nodes are added in order to generate a conforming grid, and
 * non-conforming grids (with nodes in the middle of edges) are not
 * allowed.
 */
class PatternedMesh : public MooseMesh
{
public:
  static InputParameters validParams();

  PatternedMesh(const InputParameters & parameters);
  PatternedMesh(const PatternedMesh & other_mesh);
  virtual ~PatternedMesh() = default;

  virtual std::unique_ptr<MooseMesh> safeClone() const override;

  virtual void buildMesh() override;

protected:
  // The mesh files to read
  const std::vector<MeshFileName> & _files;

  // The pattern, starting with the upper left corner
  const std::vector<std::vector<unsigned int>> & _pattern;

  // Pointer to the original "row" mesh to be repeated and stitched
  ReplicatedMesh * _original_mesh;

  // Holds the pointers to the meshes
  std::vector<std::unique_ptr<ReplicatedMesh>> _meshes;

  // Holds a mesh for each row, these will be stitched together in the end
  std::vector<std::unique_ptr<ReplicatedMesh>> _row_meshes;

  const Real _x_width;
  const Real _y_width;
  const Real _z_width;
};
