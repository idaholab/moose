#include "CosineHumpFunction.h"

registerMooseObject("THMApp", CosineHumpFunction);

template <>
InputParameters
validParams<CosineHumpFunction>()
{
  InputParameters params = validParams<Function>();

  MooseEnum axis("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>("axis", axis, "Coordinate axis on which the hump occurs");
  params.addRequiredParam<Real>("hump_center_position", "Hump center position on selected axis");
  params.addRequiredParam<Real>("hump_width", "Width of the hump");
  params.addRequiredParam<Real>("hump_begin_value", "Value before and after the hump");
  params.addRequiredParam<Real>("hump_center_value", "Value at the center of the hump");

  params.addClassDescription("Computes a cosine hump of a user-specified width and height");

  return params;
}

CosineHumpFunction::CosineHumpFunction(const InputParameters & parameters)
  : Function(parameters),

    _component(getParam<MooseEnum>("axis")),
    _hump_width(getParam<Real>("hump_width")),
    _hump_center_position(getParam<Real>("hump_center_position")),

    _hump_begin_value(getParam<Real>("hump_begin_value")),
    _hump_center_value(getParam<Real>("hump_center_value")),

    _cosine_amplitude(0.5 * (_hump_center_value - _hump_begin_value)),
    _hump_mid_value(0.5 * (_hump_center_value + _hump_begin_value)),
    _hump_left_end(_hump_center_position - 0.5 * _hump_width),
    _hump_right_end(_hump_center_position + 0.5 * _hump_width)
{
}

Real
CosineHumpFunction::value(Real /*t*/, const Point & p)
{
  const Real x = p(_component);

  if (x < _hump_left_end)
    return _hump_begin_value;
  else if (x > _hump_right_end)
    return _hump_begin_value;
  else
    return _hump_mid_value -
           _cosine_amplitude *
               std::cos(2 * M_PI / _hump_width * (x - _hump_center_position + 0.5 * _hump_width));
}

RealVectorValue
CosineHumpFunction::gradient(Real /*t*/, const Point & /*p*/)
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " is not implemented.");
}
