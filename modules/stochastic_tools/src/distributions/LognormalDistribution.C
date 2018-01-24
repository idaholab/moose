/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "LognormalDistribution.h"

template <>
InputParameters
validParams<LognormalDistribution>()
{
  InputParameters params = validParams<Distribution>();
  params.addClassDescription("Boost Lognormal distribution.");
  params.addRequiredParam<Real>("location", "The Lognormal location parameter.");
  params.addRequiredParam<Real>("scale", "The Lognormal scale parameter.");
  return params;
}

LognormalDistribution::LognormalDistribution(const InputParameters & parameters)
  : BoostDistribution<boost::math::lognormal_distribution<Real>>(parameters)
{
  _distribution_unique_ptr = libmesh_make_unique<boost::math::lognormal_distribution<Real>>(
      getParam<Real>("location"), getParam<Real>("scale"));
}
