//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef NORMALDISTRIBUTION_H
#define NORMALDISTRIBUTION_H

#include "BoostDistribution.h"

class NormalDistribution;

template <>
InputParameters validParams<NormalDistribution>();

/**
 * A class used to generate Normal distribution via Boost
 */
class NormalDistribution : public BoostDistribution<boost::math::normal_distribution<Real>>
{
public:
  NormalDistribution(const InputParameters & parameters);
};

#endif // NORMALDISTRIBUTION_H
