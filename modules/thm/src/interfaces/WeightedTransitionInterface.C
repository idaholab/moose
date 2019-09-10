#include "WeightedTransitionInterface.h"

template <>
InputParameters
validParams<WeightedTransitionInterface>()
{
  InputParameters params = validParams<SmoothTransitionInterface>();
  return params;
}

WeightedTransitionInterface::WeightedTransitionInterface(const MooseObject * moose_object)
  : SmoothTransitionInterface(moose_object)
{
}

Real
WeightedTransitionInterface::computeTransitionValue(const Real & x,
                                                    const Real & f1,
                                                    const Real & f2) const
{
  if (x <= _x1)
    return f1;
  else if (x >= _x2)
    return f2;
  else
  {
    const Real w = computeWeight(x);
    return w * f1 + (1.0 - w) * f2;
  }
}

Real
WeightedTransitionInterface::computeTransitionValueDerivative(
    const Real & x, const Real & f1, const Real & f2, const Real & df1dx, const Real & df2dx) const
{
  if (x <= _x1)
    return df1dx;
  else if (x >= _x2)
    return df2dx;
  else
  {
    const Real w = computeWeight(x);
    const Real dwdx = -0.5 * std::sin(M_PI / (_x2 - _x1) * (x - _x1)) * M_PI / (_x2 - _x1);
    return w * df1dx + (1.0 - w) * df2dx + dwdx * (f1 - f2);
  }
}

Real
WeightedTransitionInterface::computeWeight(const Real & x) const
{
  return 0.5 * (std::cos(M_PI / (_x2 - _x1) * (x - _x1)) + 1.0);
}
