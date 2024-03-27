//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InputParameters.h"

/**
 * This class is responsible for adding relationship managers
 * that describe geometric, algebraic and coupling ghosting
 * for finite volume computations.
 */
class FVRelationshipManagerInterface
{
public:
  static InputParameters validParams();

  /**
   * Helper function to set the relationship manager parameters
   */
  static void setRMParams(const InputParameters & obj_params,
                          InputParameters & rm_params,
                          unsigned short ghost_layers);

  FVRelationshipManagerInterface() {}
};
