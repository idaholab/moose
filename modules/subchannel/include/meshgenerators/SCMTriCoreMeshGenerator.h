//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SCMCoreMeshGeneratorBase.h"

/**
 * Places triangular-lattice SCM assembly meshes on a hexagonal core lattice and merges them into
 * one mesh. Each placed assembly keeps its own subdomains and boundaries, renamed with the
 * assembly index, and is tagged with the "assembly_id" extra element integer. Assemblies are not
 * connected to each other.
 */
class SCMTriCoreMeshGenerator : public SCMCoreMeshGeneratorBase
{
public:
  static InputParameters validParams();

  SCMTriCoreMeshGenerator(const InputParameters & parameters);

protected:
  void readAssemblySizes() override;
  bool overlap(unsigned int input_a, unsigned int input_b, const Point & offset) const override;

  /// Flat-to-flat duct size of each input [m]
  std::vector<Real> _flat_to_flat;
};
