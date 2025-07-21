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
 * Convert the elements involved in a set of external boundaries to ensure that the boundary set
 * only contains TRI3 elements.
 */
class BoundaryTransitionGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  BoundaryTransitionGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// Mesh that possibly comes from another generator
  std::unique_ptr<MeshBase> & _input;
  ///The boundaries to be converted
  const std::vector<BoundaryName> _boundary_names;
  /// Number of layers of elements to be converted
  const unsigned int _conversion_element_layer_number;
  /// Whether to check if the provided boundaries are external
  const bool _external_boundaries_checking;
};
