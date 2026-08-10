#include "SigmoidTrendWeibullAux.h"
#include "KLE_uo.h"
#include <cmath>

registerMooseObject("MooseApp", SigmoidTrendWeibullAux);

InputParameters
SigmoidTrendWeibullAux::validParams()
{
  InputParameters params = SigmoidTrendAuxBase::validParams();
  params.addClassDescription("Weibull field via Gaussian copula, with a position-dependent scale "
                             "which depends on the distance from the centerline");
  params.addRequiredParam<Real>("shape", "Weibull shape parameter (k > 0), spatially constant");
  return params;
}

SigmoidTrendWeibullAux::SigmoidTrendWeibullAux(const InputParameters & parameters)
  : SigmoidTrendAuxBase(parameters), _shape(getParam<Real>("shape"))
{
  if (_shape <= 0.0)
    mooseError("SigmoidTrendWeibullAux: 'shape' must be > 0, got ", _shape);
}

Real
SigmoidTrendWeibullAux::computeValue()
{
  const Point & p = isNodal() ? *_current_node : _q_point[_qp];
  const Real d = distanceToSegment(p);
  const Real scale = trendValue(d);

  const Real u = _kl_uo.getCopulaUniform(p);
  return scale * std::pow(-std::log(1.0 - u), 1.0 / _shape);
}
