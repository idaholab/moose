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
 * This PatternedCartPeripheralModifier object removes the outmost layer of the input mesh and add a
 * transition layer mesh to facilitate stitching.
 */
class PatternedCartPeripheralModifier : public PatternedPolygonPeripheralModifierBase
{
public:
  static InputParameters validParams();

  PatternedCartPeripheralModifier(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// MeshMetaData: whether the peripheral area of the generated mesh can be trimmed by PolygonMeshTrimmer
  const bool & _square_peripheral_trimmability;
  /// MeshMetaData: whether the generated mesh can be trimmed through its center by PolygonMeshTrimmer
  bool & _square_center_trimmability;
};
