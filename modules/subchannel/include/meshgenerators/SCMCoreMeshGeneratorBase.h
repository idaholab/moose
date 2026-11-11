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
 * Base class for mesh generators that place SCM assembly meshes on a core lattice and merge them
 * into one mesh. Each placed assembly keeps its own subdomains and boundaries, renamed with the
 * assembly index, and is tagged with the "assembly_id" extra element integer. Assemblies are not
 * connected to each other. Derived classes define the lattice and the overlap test.
 */
class SCMCoreMeshGeneratorBase : public MeshGenerator
{
public:
  static InputParameters validParams();

  SCMCoreMeshGeneratorBase(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /**
   * Read the duct size metadata of every input. Called once before the overlap checks; derived
   * classes store what overlap() needs.
   */
  virtual void readAssemblySizes() = 0;

  /**
   * Whether the ducts of two inputs overlap when their centers are \p offset apart. Ducts that
   * only touch do not overlap.
   */
  virtual bool overlap(unsigned int input_a, unsigned int input_b, const Point & offset) const = 0;

  /// Names of the input SCM assembly, duct, or detailed assembly mesh generators
  const std::vector<MeshGeneratorName> & _input_names;
  /// Input meshes
  const std::vector<std::unique_ptr<MeshBase> *> _meshes;
  /// Core pattern of indices into the inputs; -1 marks an empty position
  const std::vector<std::vector<int>> & _pattern;
  /// Center-to-center distance between neighboring assembly positions [m]
  const Real _assembly_pitch;
  /// Placed assemblies in pattern order: (index into the inputs, x-y center of the assembly)
  std::vector<std::pair<unsigned int, Point>> _assemblies;

  /**
   * Check that a pattern entry is -1 or a valid input index and, if it is an index, place that
   * input at \p center.
   */
  void addAssembly(int entry, const Point & center);
};
