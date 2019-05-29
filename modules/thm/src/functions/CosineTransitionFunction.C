#include "CosineTransitionFunction.h"

registerMooseObject("THMApp", CosineTransitionFunction);

template <>
InputParameters
validParams<CosineTransitionFunction>()
{
  InputParameters params = validParams<Function>();

  MooseEnum axis("x=0 y=1 z=2 t=3");
  params.addRequiredParam<MooseEnum>(
      "axis", axis, "Coordinate axis on which the transition occurs");
  params.addRequiredParam<Real>("begin_coordinate", "Coordinate at which to begin transition");
  params.addRequiredParam<Real>("transition_width", "Width of the transition region");
  params.addRequiredParam<Real>("begin_value", "Value at the beginning of the transition");
  params.addRequiredParam<Real>("end_value", "Value at the end of the transition");

  params.addClassDescription(
      "Computes a cosine transtition of a user-specified width between two values");

  return params;
}

CosineTransitionFunction::CosineTransitionFunction(const InputParameters & parameters)
  : Function(parameters),

    _component(getParam<MooseEnum>("axis")),
    _use_time(_component == 3),
    _transition_width(getParam<Real>("transition_width")),
    _begin_coordinate(getParam<Real>("begin_coordinate")),
    _end_coordinate(_begin_coordinate + _transition_width),

    _begin_value(getParam<Real>("begin_value")),
    _end_value(getParam<Real>("end_value")),
    _amplitude(0.5 * (_end_value - _begin_value))
{
}

Real
CosineTransitionFunction::value(Real t, const Point & p) const
{
  const Real x = _use_time ? t : p(_component);

  if (x < _begin_coordinate)
    return _begin_value;
  else if (x > _end_coordinate)
    return _end_value;
  else if (_end_value > _begin_value)
    return _begin_value +
           _amplitude * (1 + std::cos((x - _begin_coordinate) / _transition_width * M_PI + M_PI));
  else
    return _begin_value +
           _amplitude * (1 - std::cos((x - _begin_coordinate) / _transition_width * M_PI));
}

RealVectorValue
CosineTransitionFunction::gradient(Real /*t*/, const Point & /*p*/) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " is not implemented.");
}
