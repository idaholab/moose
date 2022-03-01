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

/**
 * A mesh generator to generate new sidesets from all faces matching the normal
 */
class SideSetsFromNormalsGenerator : public SideSetsGeneratorBase
{
public:
  static InputParameters validParams();

  SideSetsFromNormalsGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// the mesh to add the sidesets to
  std::unique_ptr<MeshBase> & _input;
  /// holds the boundary names for the sidesets
  std::vector<BoundaryName> _boundary_names;
  /// holds the normals used to generate sidesets
  std::vector<Point> _normals;
  /// a map from the boundaries to the normals
  std::map<BoundaryID, RealVectorValue> & _boundary_to_normal_map;
  /// a tolerance for comparing normals
  const Real _tolerance;
};
