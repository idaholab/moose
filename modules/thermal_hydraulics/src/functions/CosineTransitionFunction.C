#include "CosineTransitionFunction.h"

registerMooseObject("ThermalHydraulicsApp", CosineTransitionFunction);

InputParameters
CosineTransitionFunction::validParams()
{
  InputParameters params = SmoothTransitionFunction::validParams();

  params.addClassDescription(
      "Computes a cosine transtition of a user-specified width between two values");

  return params;
}

CosineTransitionFunction::CosineTransitionFunction(const InputParameters & parameters)
  : SmoothTransitionFunction(parameters),

    _transition(_x_center, _transition_width)
{
}

Real
CosineTransitionFunction::value(Real t, const Point & p) const
{
  const Real x = _use_time ? t : p(_component);

  return _transition.value(x, _function1.value(t, p), _function2.value(t, p));
}

RealVectorValue
CosineTransitionFunction::gradient(Real /*t*/, const Point & /*p*/) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " is not implemented.");
}
