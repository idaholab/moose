/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "NormalDistribution.h"

template <>
InputParameters
validParams<NormalDistribution>()
{
  InputParameters params = validParams<Distribution>();
  params.addClassDescription("Boost Normal distribution.");
  params.addRequiredParam<Real>("mean", "Mean of the distribution.");
  params.addRequiredParam<Real>("standard_deviation", "Standard deviation of the distribution.");
  return params;
}

NormalDistribution::NormalDistribution(const InputParameters & parameters)
#ifdef LIBMESH_HAVE_EXTERNAL_BOOST
  : BoostDistribution<boost::math::normal_distribution<Real>>(parameters)
#else
  : BoostDistribution<>(parameters)
#endif
{
#ifdef LIBMESH_HAVE_EXTERNAL_BOOST
  _distribution_unique_ptr = libmesh_make_unique<boost::math::normal_distribution<Real>>(
      getParam<Real>("mean"), getParam<Real>("standard_deviation"));
#endif
}
