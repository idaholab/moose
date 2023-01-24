//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once
#include "PatternedPolygonPeripheralModifierBase.h"
#include "MooseEnum.h"
#include "MeshMetaDataInterface.h"
#include "LinearInterpolation.h"

/**
 * This PatternedHexPeripheralModifier object removes the outmost layer of the input mesh and add a
 * transition layer mesh to facilitate stitching.
 */
class PatternedHexPeripheralModifier : public PatternedPolygonPeripheralModifierBase
{
public:
  static InputParameters validParams();

  PatternedHexPeripheralModifier(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;
};
