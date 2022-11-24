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

/**
 * Creates a new mesh out of one or more subdomains/blocks from another mesh
 */
class BlockToMeshConverterGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  BlockToMeshConverterGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// Mesh that comes from another generator
  std::unique_ptr<MeshBase> & _input;

  /// Blocks to extract to form a mesh
  const std::vector<SubdomainName> _target_blocks;
};
