//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "GeneralPostprocessor.h"

/**
 * Returns the number of fixed point iterations taken by the Executioner as a Postprocessor.
 */
class NumFixedPointIterations : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  NumFixedPointIterations(const InputParameters & parameters);

  virtual void execute() override {}
  virtual void initialize() override{};

  virtual Real getValue() override;
};
