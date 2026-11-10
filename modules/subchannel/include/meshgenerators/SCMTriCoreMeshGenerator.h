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
 * Places triangular-lattice SCM assembly meshes on a hexagonal core lattice and merges them into
 * one mesh. Each placed assembly keeps its own subdomains and boundaries, renamed with the
 * assembly index, and is tagged with the "assembly_id" extra element integer. Assemblies are not
 * connected to each other.
 */
class SCMTriCoreMeshGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  SCMTriCoreMeshGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// Names of the input SCM assembly (or duct) mesh generators
  const std::vector<MeshGeneratorName> & _input_names;
  /// Input meshes
  const std::vector<std::unique_ptr<MeshBase> *> _meshes;
  /// Hexagonal core pattern of indices into the inputs; -1 marks an empty position
  const std::vector<std::vector<int>> & _pattern;
  /// Center-to-center distance between neighboring assembly positions [m]
  const Real _assembly_pitch;
  /// Placed assemblies in pattern order: (index into the inputs, x-y center of the assembly)
  std::vector<std::pair<unsigned int, Point>> _assemblies;
};
