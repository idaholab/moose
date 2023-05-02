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
 * Simple positions from an input parameter
 */
class InputPositions : public Positions
{
public:
  static InputParameters validParams();
  InputPositions(const InputParameters & parameters);
  virtual ~InputPositions() = default;

  void initialize() override {}
};
