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

class BoostLognormalDistribution;

template <>
InputParameters validParams<BoostLognormalDistribution>();

/**
 * A class used to generate Lognormal distribution via Boost
 */
class BoostLognormalDistribution
  : public BoostDistribution<boost::math::lognormal_distribution<Real>>
{
public:
  BoostLognormalDistribution(const InputParameters & parameters);
};

