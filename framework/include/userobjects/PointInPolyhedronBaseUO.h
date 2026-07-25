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
#include "PointContainmentClassifier.h"

/**
 * Common base class providing shared parameters and validation for
 * point-containment user objects. The input file selects a backend via the
 * `point_containment_method` enum; the typed PointContainmentMethod is exposed
 * to subclasses along with the shared tuning/debug parameters.
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
  /// Assemble the ray-backend tuning/debug options from the shared parameters.
  /// (Ignored by the fixed_x_ray / TriangleManifold backend.)
  PcaRayOptions pcaRayOptions() const;

  /// Selected point-containment backend.
  const PointContainmentMethod _method;

  /// User-supplied ray direction (used only by user_selected_ray).
  const Point _ray_direction;

  /// eps / surface tolerance for on-surface and intersection checks.
  const Real _tolerance;

  /// Maximum number of elements in a leaf node of the KD-tree (ray backends).
  const int _leaf_max_size;

  /// Oriented Bounding Box (OBB) debug file name (ray backends only).
  const FileName _obb_file_name;

  /// Ray debug file name (ray backends only).
  const FileName _ray_file_name;
};
