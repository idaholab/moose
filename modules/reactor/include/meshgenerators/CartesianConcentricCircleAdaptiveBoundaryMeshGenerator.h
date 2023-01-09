//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PolygonConcentricCircleMeshGeneratorBase.h"

/**
 * This CartesianConcentricCircleAdaptiveBoundaryMeshGenerator object is designed to generate
 * cartesian meshes with adaptive boundary to facilitate stitching.
 */
class CartesianConcentricCircleAdaptiveBoundaryMeshGenerator
  : public PolygonConcentricCircleMeshGeneratorBase
{
public:
  static InputParameters validParams();

  CartesianConcentricCircleAdaptiveBoundaryMeshGenerator(const InputParameters & parameters);

protected:
  /// Name of input mesh generator
  const std::vector<MeshGeneratorName> _input_names;
};
