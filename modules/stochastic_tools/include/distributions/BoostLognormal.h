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
 * A deprecated class used to generate a lognormal distribution via Boost
 */
class BoostLognormal : public BoostDistribution<boost::math::lognormal_distribution<Real>>
{
public:
  static InputParameters validParams();

  BoostLognormal(const InputParameters & parameters);
};
