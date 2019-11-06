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
#include "DiracKernel.h"

/**
 * This DiracKernel tries to cache points using the wrong IDs and
 * throws an error.
 */
class BadCachingPointSource : public DiracKernel
{
public:
  static InputParameters validParams();

  BadCachingPointSource(const InputParameters & parameters);

  virtual void addPoints();
  virtual Real computeQpResidual();

  /**
   * Gets incremented every time the addPoints() method is called. Simulates a user
   * adding points with a 'unique' ID that is already used by another point.
   */
  unsigned int _called;
};
