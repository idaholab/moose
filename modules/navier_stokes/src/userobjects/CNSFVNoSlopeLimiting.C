/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "CNSFVNoSlopeLimiting.h"

template <>
InputParameters
validParams<CNSFVNoSlopeLimiting>()
{
  InputParameters params = validParams<SlopeLimitingBase>();
  params.addClassDescription("A user object that does no slope limiting in multi-dimensions.");
  return params;
}

CNSFVNoSlopeLimiting::CNSFVNoSlopeLimiting(const InputParameters & parameters)
  : SlopeLimitingBase(parameters)
{
}

std::vector<RealGradient>
CNSFVNoSlopeLimiting::limitElementSlope() const
{
  return _rslope.getElementSlope(_current_elem->id());
}
