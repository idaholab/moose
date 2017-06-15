/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef WEIBULLDISTRIBUTION_H
#define WEIBULLDISTRIBUTION_H

#include "BoostDistribution.h"

#ifdef LIBMESH_HAVE_BOOST

#include <boost/math/distributions/weibull.hpp>

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

#endif // LIBMESH_HAVE_BOOST
#endif // WEIBULLDISTRIBUTION_H
