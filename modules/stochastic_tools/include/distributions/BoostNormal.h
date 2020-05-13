//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "BoostDistribution.h"

/**
 * A deprecated class used to generate a normal distribution via Boost
 */
class BoostNormal : public BoostDistribution<boost::math::normal_distribution<Real>>
{
public:
  static InputParameters validParams();

  BoostNormal(const InputParameters & parameters);
};
