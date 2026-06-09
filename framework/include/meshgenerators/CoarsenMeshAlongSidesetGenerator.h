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
 * MeshGenerator that coarsens a 2D-element (TRI3/QUAD4) surface mesh along a sideset by
 * collapsing alternate boundary nodes. The sideset may be internal: elements on both sides
 * of it are coarsened and the sideset itself is preserved.
 */
class CoarsenMeshAlongSidesetGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  CoarsenMeshAlongSidesetGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// Input mesh to coarsen
  std::unique_ptr<MeshBase> & _input;

  /// Sideset(s) to coarsen the mesh along
  const std::vector<BoundaryName> & _boundaries;

  /// Whether the mesh generator should be verbose to the console
  const bool _verbose;
};
