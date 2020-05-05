//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "BoostNormal.h"

/**
 * A deprecated class used to generate a normal distribution via Boost
 */
class BoostNormalDistribution : public BoostNormal
{
public:
  static InputParameters validParams();
  BoostNormalDistribution(const InputParameters & parameters);
};
