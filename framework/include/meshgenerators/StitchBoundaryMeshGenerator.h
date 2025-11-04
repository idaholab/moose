//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "StitchMeshGeneratorBase.h"
#include "libmesh/replicated_mesh.h"

/**
 * Allows a pair of boundaries to be "stitched" together.
 */
class StitchBoundaryMeshGenerator : public StitchMeshGeneratorBase
{
public:
  static InputParameters validParams();

  StitchBoundaryMeshGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// the input mesh
  std::unique_ptr<MeshBase> & _input;
};
