//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Moose includes
#include "Positions.h"
#include "BlockRestrictable.h"

/**
 * Positions from centroids of elements in the mesh
 */
class ElementCentroidPositions : public Positions, BlockRestrictable
{
public:
  static InputParameters validParams();
  ElementCentroidPositions(const InputParameters & parameters);
  virtual ~ElementCentroidPositions() = default;

  virtual void initialize() override;

  MooseMesh & _mesh;
};
