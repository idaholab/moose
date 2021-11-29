//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Moose Includes
#include "NodalDamper.h"

/**
 * This class implements a damper that limits the value of a variable to be within
 * user-specified bounds.
 */
class BoundingValueNodalDamper : public NodalDamper
{
public:
  static InputParameters validParams();

  BoundingValueNodalDamper(const InputParameters & parameters);

protected:
  /// The maximum permissible value of the variable
  const Real & _max_value;
  /// The minimum permissible value of the variable
  const Real & _min_value;
  /// Compute the damping for the current node
  virtual Real computeQpDamping() override;
};
