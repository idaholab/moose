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
/*
 * Mesh generator to create a Overlay mesh
 */
class OverlayMeshGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();
  OverlayMeshGenerator(const InputParameters & parameters);
  std::unique_ptr<MeshBase> generate() override;

protected:
  /// The dimension of the mesh
  MooseEnum _dim;

  /// Hold the generated mesh
  std::unique_ptr<MeshBase> * _build_mesh;

  /// Name of the input mesh
  const MeshGeneratorName _mesh_name;

  /// Hold the input mesh
  std::unique_ptr<MeshBase> * _input_mesh;
};
