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
class Poly2TriMeshGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  Poly2TriMeshGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  std::unique_ptr<MeshBase> & _bdy_ptr;

  const unsigned int _interpolate_bdy;

  const bool _refine_bdy;

  const bool _smooth_tri;

  // Holds pointers to the pointers to the meshes.
  const std::vector<std::unique_ptr<MeshBase> *> _hole_ptrs;

  const std::vector<bool> _stitch_holes;

  const std::vector<unsigned int> _interpolate_holes;

  const std::vector<bool> _refine_holes;

  const Real _desired_area;

  const std::string _desired_area_func;

  /// Type of algorithm used to find matching nodes (binary or exhaustive)
  const MooseEnum _algorithm;

  const bool _verbose_stitching;
};
