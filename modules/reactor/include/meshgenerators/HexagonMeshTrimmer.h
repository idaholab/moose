//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once
#include "PolygonMeshTrimmerBase.h"
#include "MooseEnum.h"

/**
 * This HexagonMeshTrimmer object takes in a hexagonal assembly or core mesh and perform peripheral
 * and/or center trimming on it.
 */
class HexagonMeshTrimmer : public PolygonMeshTrimmerBase
{
public:
  static InputParameters validParams();

  HexagonMeshTrimmer(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;
};
