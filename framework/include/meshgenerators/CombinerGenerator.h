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
#include "libmesh/replicated_mesh.h"
#include "MooseEnum.h"

// Forward declarations
class CombinerGenerator;

template <>
InputParameters validParams<CombinerGenerator>();

/**
 * Collects multiple meshes into a single (unconnected) mesh
 */
class CombinerGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  CombinerGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

  /// Fill the offset positions for each mesh
  void fillPositions();

protected:
  /**
   * Helper funciton for copying one mesh into another
   */
  void copyIntoMesh(UnstructuredMesh & destination, const UnstructuredMesh & source);

  /// The mesh generators to use
  const std::vector<MeshGeneratorName> & _input_names;

  /// The (offsets) positions for each mesh
  std::vector<Point> _positions;

  // Holds pointers to the mesh smart pointers (to be populated later).
  std::vector<std::unique_ptr<MeshBase> *> _meshes;
};
