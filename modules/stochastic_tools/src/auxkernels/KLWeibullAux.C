//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KLWeibullAux.h"
#include "KLExpansionUserObject.h"
#include "Weibull.h"

registerMooseObject("StochasticToolsApp", KLWeibullAux);

InputParameters
KLWeibullAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Generates a sample of correlated Weibull-distributed random field.");
  params.addRequiredParam<UserObjectName>("kl_user_object",
                                          "Name of the KLExpansionUserObject to sample");
  params.addRequiredRangeCheckedParam<Real>("shape", "shape > 0", "Weibull shape parameter k");
  params.addRequiredRangeCheckedParam<Real>("scale", "scale > 0", "Weibull scale parameter lambda");
  return params;
}

KLWeibullAux::KLWeibullAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _kl_uo(getUserObject<KLExpansionUserObject>("kl_user_object")),
    _shape(getParam<Real>("shape")),
    _scale(getParam<Real>("scale"))
{
}

Real
KLWeibullAux::computeValue()
{
  // inverse transform of correlated uniform from KLExpansionUserObject
  const Point & p = isNodal() ? *_current_node : _q_point[_qp];
  return Weibull::quantile(_kl_uo.getCopulaUniform(p), 0.0, _scale, _shape);
}
