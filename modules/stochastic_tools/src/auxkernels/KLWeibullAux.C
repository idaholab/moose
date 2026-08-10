// KLWeibullAux.C
#include "KLWeibullAux.h"
#include "KLE_uo.h"
#include <cmath>

registerMooseObject("MooseApp", KLWeibullAux);

InputParameters
KLWeibullAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Generates a sample of correlated Weibull-distributed random field.");
  params.addRequiredParam<UserObjectName>("kl_user_object",
                                          "Name of the KLExpansionUserObject to sample");
  params.addRequiredParam<Real>("shape", "Weibull shape parameter (k > 0)");
  params.addRequiredParam<Real>("scale", "Weibull scale parameter (lambda > 0)");
  return params;
}

KLWeibullAux::KLWeibullAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _kl_uo(getUserObject<KLExpansionUserObject>("kl_user_object")),
    _shape(getParam<Real>("shape")),
    _scale(getParam<Real>("scale"))
{
  if (_shape <= 0.0)
    mooseError("KLWeibullAux: 'shape' must be > 0, got ", _shape);
  if (_scale <= 0.0)
    mooseError("KLWeibullAux: 'scale' must be > 0, got ", _scale);
}

Real
KLWeibullAux::computeValue()
{
  // inverse transform of correlated uniform from KLExpansionUserObject
  const Point & p = isNodal() ? *_current_node : _q_point[_qp];
  const Real u = _kl_uo.getCopulaUniform(p);
  return _scale * std::pow(-std::log(1.0 - u), 1.0 / _shape);
}
