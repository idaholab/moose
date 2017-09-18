/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef NORMALDISTRIBUTION_H
#define NORMALDISTRIBUTION_H

#include "BoostDistribution.h"

class NormalDistribution;

template <>
InputParameters validParams<NormalDistribution>();

/**
 * A class used to generate Normal distribution via Boost
 */
#ifdef LIBMESH_HAVE_EXTERNAL_BOOST
class NormalDistribution : public BoostDistribution<boost::math::normal_distribution<Real>>
#else
class NormalDistribution : public BoostDistribution<>
#endif
{
public:
  NormalDistribution(const InputParameters & parameters);
};

#endif // NORMALDISTRIBUTION_H
