//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"
#include "libmesh/point.h"

/**
 * Common base class providing shared parameters and storage for
 * point-in-polyhedron checks.
 */
class PointInPolyhedronBaseUO : public GeneralUserObject
{
public:
  static InputParameters validParams();
  PointInPolyhedronBaseUO(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override {}
  virtual void finalize() override {}

protected:
  /// Ray shooting direction in Point format
  Point _ray_direction;

  /// Brute force means that we loop over all boundary elements to check if the point is inside.
  bool _brute_force;

  /// eps for intersection or on surface checking
  Real _eps;

  /// Maximum number of elements in a leaf node of the KD-tree
  int _leaf_max_size;

  /// The file name for the OBB
  FileName _obb_file_name;

  /// The file name for the ray
  FileName _ray_file_name;
};
