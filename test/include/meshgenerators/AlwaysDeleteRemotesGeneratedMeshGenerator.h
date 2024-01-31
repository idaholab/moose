//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneratedMeshGenerator.h"
#include "MooseEnum.h"

/**
 * Generates a line, square, or cube mesh with uniformly spaced or biased elements.
 */
class AlwaysDeleteRemotesGeneratedMeshGenerator : public GeneratedMeshGenerator
{
public:
  static InputParameters validParams();

  AlwaysDeleteRemotesGeneratedMeshGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;
};
