#include "LinearFunction.h"

registerMooseObject("THMApp", LinearFunction);

template <>
InputParameters
validParams<LinearFunction>()
{
  InputParameters params = validParams<Function>();
  params.addRequiredParam<FunctionName>("x_func", "The x function");
  params.addRequiredParam<Real>("a", "The constant in a + b * x");
  params.addRequiredParam<Real>("b", "The gradient value in a + b *x");
  return params;
}

LinearFunction::LinearFunction(const InputParameters & parameters)
  : Function(parameters),
    FunctionInterface(this),
    _x_func(getFunction("x_func")),
    _a(getParam<Real>("a")),
    _b(getParam<Real>("b"))
{
}

Real
LinearFunction::value(Real t, const Point & p) const
{
  return _a + _b * _x_func.value(t, p);
}

RealVectorValue
LinearFunction::gradient(Real /*t*/, const Point & /*p*/) const
{
  mooseError(name(), ": ", __PRETTY_FUNCTION__, " is not implemented.");
}
