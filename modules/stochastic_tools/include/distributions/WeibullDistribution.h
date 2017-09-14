/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

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
