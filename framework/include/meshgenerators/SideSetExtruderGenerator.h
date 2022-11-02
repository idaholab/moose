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
 * Extrude a sideset from a mesh in a given direction
 */
class SideSetExtruderGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  SideSetExtruderGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// Mesh generated from each sub-generator
  std::unique_ptr<MeshBase> * _build_mesh;

  /// Name of the base mesh containing the sideset to extruded
  const MeshGeneratorName _original_input;

  /// Extrusion vector containing both the direction and the magnitude of the sideset extrusion
  const RealVectorValue _extrusion_vector;

  /// Number of element layers in the direction of the extrusion when extruding
  const unsigned int _num_layers;

  /// Name of the sideset to extrude
  const BoundaryName _sideset_name;
};
