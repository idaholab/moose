//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InitialCondition.h"

class Function;

/**
 * Computes velocity in the direction of a 1-D element from a vector velocity function
 */
class VectorVelocityIC : public InitialCondition
{
public:
  VectorVelocityIC(const InputParameters & parameters);

  virtual Real value(const Point & p) override;

protected:
  /// Velocity function
  const Function & _vel_fn;
  /// Vector component to use
  const unsigned int _component;

public:
  static InputParameters validParams();
};
