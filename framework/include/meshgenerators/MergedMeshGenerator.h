//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MERGEDMESHGENERATOR_H
#define MERGEDMESHGENERATOR_H

#include "MeshGenerator.h"
#include "libmesh/replicated_mesh.h"
#include "MooseEnum.h"

// Forward declarations
class MergedMeshGenerator;

template <>
InputParameters validParams<MergedMeshGenerator>();

/**
 * Merge multiple meshes into a single unconnected mesh
 */
class MergedMeshGenerator : public MeshGenerator
{
public:
  MergedMeshGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate();

protected:
  /// The mesh generators to read
  const std::vector<MeshGeneratorName> & _input_names;

  // Holds pointers to the mesh smart pointers (to be populated later).
  std::vector<std::unique_ptr<MeshBase> *> _meshes;
};

#endif // MERGEDMESHGENERATOR_H
