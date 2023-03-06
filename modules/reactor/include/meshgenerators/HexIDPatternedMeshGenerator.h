//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PatternedHexMeshGenerator.h"
#include "MooseEnum.h"
#include "MeshMetaDataInterface.h"

/**
 * Generates patterned hexagonal meshes with a reporting ID
 */
class HexIDPatternedMeshGenerator : public PatternedHexMeshGenerator
{
public:
  static InputParameters validParams();

  HexIDPatternedMeshGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;
};
