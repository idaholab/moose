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

/**
 * Positions transformed using a simple operation (translation, rotation, scaling)
 */
class TransformedPositions : public Positions
{
public:
  static InputParameters validParams();
  TransformedPositions(const InputParameters & parameters);
  virtual ~TransformedPositions() = default;

  virtual void initialize() override;

private:
  /// Position object providing the transformed positions
  const Positions * _base_positions;

  /// Transformation to perform
  const MooseEnum & _transform;

  /// Vector to use for the operation
  const Point _vector_value;
};
