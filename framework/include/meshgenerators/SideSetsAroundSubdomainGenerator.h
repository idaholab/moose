//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SideSetsGeneratorBase.h"

#include "libmesh/point.h"

/**
 * Adds the faces on the boundary of given block
 * to the sidesets specified by "boundary"
 * Optionally, only adds faces that have a normal
 * equal to specified normal up to a tolerance
 */
class SideSetsAroundSubdomainGenerator : public SideSetsGeneratorBase
{
public:
  static InputParameters validParams();

  SideSetsAroundSubdomainGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  std::unique_ptr<MeshBase> & _input;

  /// names of the sidesets to which the faces will be added
  std::vector<BoundaryName> _boundary_names;

  /// true if only faces close to "normal" will be added
  bool _using_normal;

  /// Only apply the sideset to external boundaries
  const bool _external_only;
  /**
   * if normal is specified, then faces are only added
   * if face_normal.normal_hat <= 1 - normal_tol
   * where normal_hat = _normal/|_normal|
   */
  Real _normal_tol;

  /// if specified, then faces are only added if their normal is close to this
  Point _normal;
};
