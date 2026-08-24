//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MeshGenerator.h"

/**
 * Meshes a region bounded by input curves with quadrilaterals in one step, by chaining the
 * generators of the all-quad pipeline as sub-generators: XYFrontalDelaunayGenerator triangulates
 * the region, TriToQuadConverter recombines the triangles into quadrilaterals,
 * MoveNodesToCurveGenerator moves the boundary nodes the conversion added back onto each
 * parametric boundary curve, and SmoothMeshGenerator optionally relaxes the result.
 */
class XYQuadrilateralMeshFromBoundaryCurve : public MeshGenerator
{
public:
  static InputParameters validParams();

  XYQuadrilateralMeshFromBoundaryCurve(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// Mesh of the final sub-generator of the chain, which is the mesh this generator produces
  std::unique_ptr<MeshBase> * _build_mesh;
};
