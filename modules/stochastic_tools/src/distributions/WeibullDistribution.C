/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "WeibullDistribution.h"

template <>
InputParameters
validParams<WeibullDistribution>()
{
  InputParameters params = validParams<Distribution>();
  params.addClassDescription("Boost Weibull distribution.");
  params.addRequiredParam<Real>("shape", "The Weibull shape parameter.");
  params.addParam<Real>("scale", 1, "The Weibull scale parameter.");
  return params;
}

WeibullDistribution::WeibullDistribution(const InputParameters & parameters)
  : BoostDistribution<boost::math::weibull_distribution<Real>>(parameters)
{
  _distribution_unique_ptr = libmesh_make_unique<boost::math::weibull_distribution<Real>>(
      getParam<Real>("shape"), getParam<Real>("scale"));
}
