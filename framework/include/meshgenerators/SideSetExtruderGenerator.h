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

#include "libmesh/mesh_generation.h"

/**
 * Extrude a 1D, 2D, or 3D mesh outward within the same dimension
 */
class SideSetExtruderGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  SideSetExtruderGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// Mesh that comes from another generator
  std::unique_ptr<MeshBase> * _build_mesh;

  const MeshGeneratorName _original_input;
  const RealVectorValue _extrusion_vector;
  const unsigned int _num_layers;
  const BoundaryName _sideset_name;
};
