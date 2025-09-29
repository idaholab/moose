//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearFVAdvectionDiffusionBC.h"

/**
 * Base class that allows us for error checking and BC fetching in an efficient way.
 */
class LinearFVCHTBCInterface
{
public:
  /**
   * Class constructor.
   */
  LinearFVCHTBCInterface() {}

  static InputParameters validParams();
};
