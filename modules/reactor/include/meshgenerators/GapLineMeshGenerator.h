//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PolygonMeshGeneratorBase.h"

/**
 * Generate a polyline mesh that is based on an input 2D-XY mesh. The 2D-XY mesh needs to be a
 * connected mesh with only one outer boundary manifold. The polyline mesh generated along with the
 * boundary of the input mesh form a gap with a specified thickness.
 */
class GapLineMeshGenerator : public PolygonMeshGeneratorBase
{
public:
  static InputParameters validParams();

  GapLineMeshGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// Input mesh defining the boundary
  std::unique_ptr<MeshBase> & _input;

  /// The thickness of the gap to be created
  const Real _thickness;

  /// The direction in which the gap is created with respect to the boundary of the input mesh
  const enum class GapDirection { OUTWARD, INWARD } _gap_direction;

  /// The boundary IDs around which the gap will be created
  const std::vector<boundary_id_type> _boundary_ids;
};
