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

  unsigned int _interpolate_bdy;

  bool _refine_bdy;

  // Holds pointers to the pointers to the meshes.
  std::vector<std::unique_ptr<MeshBase> *> _hole_ptrs;

  std::vector<bool> _stitch_holes;

  std::vector<unsigned int> _interpolate_holes;

  std::vector<bool> _refine_holes;

  Real _desired_area;

  std::string _desired_area_func;

  /// Type of algorithm used to find matching nodes (binary or exhaustive)
  MooseEnum _algorithm;

  bool _verbose_stitching;
};
