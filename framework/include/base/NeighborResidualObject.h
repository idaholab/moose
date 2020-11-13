//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ResidualObject.h"

/**
 * This is a base class for objects that can provide residual contributions for both local and
 * neighbor elements
 */
class NeighborResidualObject : public ResidualObject
{
public:
  static InputParameters validParams();

  /**
   * Class constructor.
   * @param parameters The InputParameters for the object
   */
  NeighborResidualObject(const InputParameters & parameters);

  /**
   * Prepare neighbor shape functions
   * @param var_num The variable number whose neighbor shape functions should be prepared
   */
  void prepareNeighborShapes(unsigned int var_num);
};
