/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef LOGNORMALDISTRIBUTION_H
#define LOGNORMALDISTRIBUTION_H

#include "BoostDistribution.h"

class LognormalDistribution;

template <>
InputParameters validParams<LognormalDistribution>();

/**
 * A class used to generate Lognormal distribution via Boost
 */
#ifdef LIBMESH_HAVE_EXTERNAL_BOOST
class LognormalDistribution : public BoostDistribution<boost::math::lognormal_distribution<Real>>
#else
class LognormalDistribution : public BoostDistribution<>
#endif
{
public:
  LognormalDistribution(const InputParameters & parameters);
};

#endif // LOGNORMALDISTRIBUTION_H
