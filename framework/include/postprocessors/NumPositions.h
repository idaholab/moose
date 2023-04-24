//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralPostprocessor.h"

class Positions;

/**
 * Counts the number of positions from a Positions object
 */
class NumPositions : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  NumPositions(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override {}

  virtual Real getValue() override;

private:
  // Positions object to count the number of positions from
  const Positions & _positions;
};
