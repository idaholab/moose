#include "UserForcingFunction.h"

template<>
InputParameters validParams<UserForcingFunction>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredParam<std::string>("function", "The user defined forcing function.");
  params.addParam<std::vector<std::string> >("vars", std::vector<std::string>(0), "The variables (excluding t,x,y,z) in the forcing function.");
  params.addParam<std::vector<Real> >("vals", std::vector<Real>(0), "The values that correspond to the variables");
  return params;
}

UserForcingFunction::UserForcingFunction(std::string name,
                       MooseSystem &sys,
                       InputParameters parameters)
  :Kernel(name, sys, parameters),
  _functor(parameters.get<std::string>("function"),
           parameters.get<std::vector<std::string> >("vars"),
           parameters.get<std::vector<Real> >("vals"))
{
}

Real
UserForcingFunction::f()
{
  return _functor(_t, _q_point[_qp](0), _q_point[_qp](1), _q_point[_qp](2));
}

Real
UserForcingFunction::computeQpResidual()
{
  return -_test[_i][_qp] * f();
}

Real
UserForcingFunction::computeQpJacobian()
{
  return 0;
}
