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
 * Class to remesh surface subdomains using a triangle mesh inside the subdomain boundaries
 */
class SurfaceSubdomainsDelaunayRemesher : public SurfaceDelaunayGeneratorBase,
                                          public LevelSetMeshingHelper
{
public:
  static InputParameters validParams();

  SurfaceSubdomainsDelaunayRemesher(const InputParameters & parameters);

  virtual std::unique_ptr<MeshBase> generate() override;

protected:
  ///The input mesh name
  std::unique_ptr<MeshBase> & _input;

  /// The subdomains to be converted to a 2D mesh
  std::vector<std::vector<SubdomainName>> _subdomain_names;

  /// Number of groups of subdomains, also number of times we will call the Delaunay generator
  unsigned int _num_groups;

  ///The edge boundaries delineating holes
  const std::vector<std::vector<BoundaryName>> _hole_boundary_names;

  /// Number of points added to boundaries around each group of surface subdomains
  std::vector<unsigned int> _interpolate_boundaries;

  /// Target areas for the triangulation for each group of surface subdomains
  std::vector<Real> _desired_areas;

  /**
   * Generate a 2D mesh using Delaunay triangulation based on the input 2D surface mesh and the 2D
   * hole meshes
   * @param mesh_2d The input 2D boundary mesh
   * @param hole_meshes_2d The 2D hole meshes
   * @param group_i the index of the group
   * @return a unique pointer to the generated 2D mesh
   */
  std::unique_ptr<ReplicatedMesh>
  General2DDelaunay(std::unique_ptr<ReplicatedMesh> & mesh_2d,
                    std::vector<std::unique_ptr<ReplicatedMesh>> & hole_meshes_2d,
                    unsigned int group_i);
};
