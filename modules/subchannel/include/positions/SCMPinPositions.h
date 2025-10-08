//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Positions.h"
#include "SubChannelMesh.h"

/**
 * Creates positions (points) from the pins of a subchannel mesh
 */
class SCMPinPositions : public Positions
{
public:
  static InputParameters validParams();

  SCMPinPositions(const InputParameters & parameters);

  void initialize() override;

protected:
  /// Pointer to the subchannel mesh
  const SubChannelMesh * const _scm_mesh;
};
