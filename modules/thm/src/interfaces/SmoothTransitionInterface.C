#include "SmoothTransitionInterface.h"
#include "MooseObject.h"

template <>
InputParameters
validParams<SmoothTransitionInterface>()
{
  InputParameters params = emptyInputParameters();
  params.addRequiredParam<Real>("transition_center", "Center position of transition");
  params.addRequiredParam<Real>("transition_width", "Width of transition");
  return params;
}

SmoothTransitionInterface::SmoothTransitionInterface(const MooseObject * moose_object)
  : _x_center(moose_object->parameters().get<Real>("transition_center")),
    _transition_width(moose_object->parameters().get<Real>("transition_width")),
    _x1(_x_center - 0.5 * _transition_width),
    _x2(_x_center + 0.5 * _transition_width)
{
}
