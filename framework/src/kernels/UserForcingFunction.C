#include "UserForcingFunction.h"

template<>
InputParameters validParams<UserForcingFunction>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredParam<std::string>("function", "The forcing function");
  return params;
}

UserForcingFunction::UserForcingFunction(std::string name,
                       MooseSystem &sys,
                       InputParameters parameters)
  :Kernel(name, sys, parameters),
  _func(getFunction("function"))
{
}

Real
UserForcingFunction::f()
{
  return _func(_t, _q_point[_qp](0), _q_point[_qp](1), _q_point[_qp](2));
}

Real
UserForcingFunction::computeQpResidual()
{
  return -_test[_i][_qp] * f();
}
