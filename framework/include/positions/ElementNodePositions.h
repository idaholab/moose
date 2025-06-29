//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
 * Positions from nodes of elements in the mesh
 */
class ElementNodePositions : public Positions, BlockRestrictable
{
public:
  static InputParameters validParams();
  ElementNodePositions(const InputParameters & parameters);
  virtual ~ElementNodePositions() = default;

  virtual void initialize() override;

  // We need to override finalize to prune duplicate positions on distributed meshes.
  virtual void finalize() override;

  const MooseMesh & _mesh;
};
