#include "UserForcingFunction.h"
#include "Function.h"

template<>
InputParameters validParams<UserForcingFunction>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredParam<std::string>("function", "The forcing function");
  return params;
}

UserForcingFunction::UserForcingFunction(const std::string & name, InputParameters parameters) :
    Kernel(name, parameters),
    _func(getFunction("function"))
{
}

Real
UserForcingFunction::f()
{
  return _func.value(_t, _q_point[_qp](0), _q_point[_qp](1), _q_point[_qp](2));
}

Real
UserForcingFunction::computeQpResidual()
{
  return -_test[_i][_qp] * f();
}
