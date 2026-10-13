//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SigmoidTrendWeibullAux.h"
#include "KLExpansionUserObject.h"
#include "Weibull.h"

registerMooseObject("StochasticToolsApp", SigmoidTrendWeibullAux);

InputParameters
SigmoidTrendWeibullAux::validParams()
{
  InputParameters params = SigmoidTrendAuxBase::validParams();
  params.addClassDescription("Weibull field via Gaussian copula, with a position-dependent scale "
                             "which depends on the distance from the centerline");
  params.addRequiredRangeCheckedParam<Real>(
      "shape", "shape > 0", "Weibull shape parameter k, spatially constant");
  return params;
}

SigmoidTrendWeibullAux::SigmoidTrendWeibullAux(const InputParameters & parameters)
  : SigmoidTrendAuxBase(parameters), _shape(getParam<Real>("shape"))
{
  if (_scale_hi <= 0.0)
    paramError("scale_max", "The Weibull scale must be positive.");
  if (_scale_lo <= 0.0)
    paramError("scale_min", "The Weibull scale must be positive.");
}

Real
SigmoidTrendWeibullAux::computeValue()
{
  const Point & p = isNodal() ? *_current_node : _q_point[_qp];
  const Real d = distanceToSegment(p);
  const Real scale = trendValue(d);

  return Weibull::quantile(_kl_uo.getCopulaUniform(p), 0.0, scale, _shape);
}
