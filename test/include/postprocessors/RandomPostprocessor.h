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
#include "MooseRandom.h"

/**
 * Just returns a random number.
 */
class RandomPostprocessor : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  RandomPostprocessor(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override {}

  virtual Real getValue() override;

private:
  const unsigned int _generator_id;

  MooseRandom & _random;
};
