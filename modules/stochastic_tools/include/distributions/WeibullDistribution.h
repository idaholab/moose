//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef WEIBULLDISTRIBUTION_H
#define WEIBULLDISTRIBUTION_H

#include "BoostDistribution.h"

class WeibullDistribution;

template <>
InputParameters validParams<WeibullDistribution>();

/**
 * A class used to generate Weibull distribution via Boost
 */
class WeibullDistribution : public BoostDistribution<boost::math::weibull_distribution<Real>>
{
public:
  WeibullDistribution(const InputParameters & parameters);
};

#endif // WEIBULLDISTRIBUTION_H
