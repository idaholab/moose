//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SurfaceDelaunayGeneratorBase.h"
#include "LevelSetMeshingHelper.h"

#include "libmesh/meshfree_interpolation.h"

/**
 * Mesh generator to remesh a boundary of a volumetric mesh using triangle elements
 */
class Boundary2DDelaunayGenerator : public SurfaceDelaunayGeneratorBase,
                                    public LevelSetMeshingHelper
{
public:
  static InputParameters validParams();

  Boundary2DDelaunayGenerator(const InputParameters & parameters);

  virtual std::unique_ptr<MeshBase> generate() override;

protected:
  ///The input mesh name
  std::unique_ptr<MeshBase> & _input;

  ///The boundaries to be converted to a 2D mesh
  const std::vector<BoundaryName> _boundary_names;

  ///The boundaries to be used as holes
  const std::vector<std::vector<BoundaryName>> _hole_boundary_names;

  /// The name of the external boundary of the mesh to generate
  BoundaryName _output_external_boundary_name;

  /**
   * Generate a 2D mesh using Delaunay triangulation based on the input 2D boundary mesh and the 2D
   * hole meshes
   * @param mesh_2d The input 2D boundary mesh
   * @param hole_meshes_2d The 2D hole meshes
   * @return a unique pointer to the generated 2D mesh
   */
  std::unique_ptr<MeshBase>
  General2DDelaunay(std::unique_ptr<MeshBase> & mesh_2d,
                    std::vector<std::unique_ptr<MeshBase>> & hole_meshes_2d);
};
