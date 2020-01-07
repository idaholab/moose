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
 * Adds a number of Dirac points with user-specified IDs to test
 * the Dirac point caching algorithm.
 */
class CachingPointSource : public DiracKernel
{
public:
  static InputParameters validParams();

  CachingPointSource(const InputParameters & parameters);

  virtual void addPoints();
  virtual Real computeQpResidual();
};
