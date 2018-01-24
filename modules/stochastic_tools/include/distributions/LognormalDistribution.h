//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef LOGNORMALDISTRIBUTION_H
#define LOGNORMALDISTRIBUTION_H

#include "BoostDistribution.h"

class LognormalDistribution;

template <>
InputParameters validParams<LognormalDistribution>();

/**
 * A class used to generate Lognormal distribution via Boost
 */
class LognormalDistribution : public BoostDistribution<boost::math::lognormal_distribution<Real>>
{
public:
  LognormalDistribution(const InputParameters & parameters);
};

#endif // LOGNORMALDISTRIBUTION_H
