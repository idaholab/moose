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

/**
 * Generates a triangulation in the XY plane, based on an input mesh
 * defining the outer boundary and an optional set of input meshes
 * defining inner hole boundaries.
 */
class XYDelaunayGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  XYDelaunayGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// Input mesh defining the boundary to triangulate within
  std::unique_ptr<MeshBase> & _bdy_ptr;

  /// How many more nodes to add in each outer boundary segment
  const unsigned int _add_nodes_per_boundary_segment;

  /// Whether to allow automatically refining the outer boundary
  const bool _refine_bdy;

  /// What subdomain_id to set on the generated triangles
  SubdomainID _output_subdomain_id;

  /// Whether to do Laplacian mesh smoothing on the generated triangles
  const bool _smooth_tri;

  /// Whether to verify holes do not intersect boundary or each other
  const bool _verify_holes;

  /// Holds pointers to the pointers to input meshes defining holes
  const std::vector<std::unique_ptr<MeshBase> *> _hole_ptrs;

  /// Whether to stitch to the mesh defining each hole
  const std::vector<bool> _stitch_holes;

  /// Whether to allow automatically refining each hole boundary
  const std::vector<bool> _refine_holes;

  /// Desired (maximum) triangle area
  const Real _desired_area;

  /// Desired triangle area as a (fparser-compatible) function of x,y
  const std::string _desired_area_func;

  /// Type of algorithm used to find matching nodes (binary or exhaustive)
  const MooseEnum _algorithm;

  /// Whether mesh stitching should have verbose output
  const bool _verbose_stitching;
};
