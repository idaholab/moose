//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MESHCOLLECTIONGENERATOR_H
#define MESHCOLLECTIONGENERATOR_H

#include "MeshGenerator.h"
#include "libmesh/replicated_mesh.h"
#include "MooseEnum.h"

// Forward declarations
class MeshCollectionGenerator;

template <>
InputParameters validParams<MeshCollectionGenerator>();

/**
 * Collects multiple meshes into a single (unconnected) mesh
 */
class MeshCollectionGenerator : public MeshGenerator
{
public:
  MeshCollectionGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// The mesh generators to read
  const std::vector<MeshGeneratorName> & _input_names;

  // Holds pointers to the mesh smart pointers (to be populated later).
  std::vector<std::unique_ptr<MeshBase> *> _meshes;
};

#endif // MESHCOLLECTIONGENERATOR_H
