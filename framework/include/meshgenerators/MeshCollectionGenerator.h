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

/**
 * Collects multiple meshes into a single (unconnected) mesh
 */
class MeshCollectionGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  MeshCollectionGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// The mesh generators to read
  const std::vector<MeshGeneratorName> & _input_names;

  // Holds pointers to the mesh smart pointers (to be populated later).
  const std::vector<std::unique_ptr<MeshBase> *> _meshes;
};
