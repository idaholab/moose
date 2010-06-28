#include "ForcingFunction.h"

/**
 * This function defines the valid parameters for
 * this Kernel and their default values
 */
template<>
InputParameters validParams<ForcingFunction>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredParam<std::string>("function", "The forcing function");
  params.addParam<std::vector<std::string> >("vars", std::vector<std::string>(0), "The variables (excluding t,x,y,z) in the forcing function");
  params.addParam<std::vector<Real> >("vals", std::vector<Real>(0), "The variables (excluding t,x,y,z) in the forcing function");
  return params;
}

ForcingFunction::ForcingFunction(std::string name,
                       MooseSystem &sys,
                       InputParameters parameters)
  // You must call the constructor of the base class first
  // The "true" here specifies that this Kernel is to be integrated
  // over the domain.
  :Kernel(name, sys, parameters),
  _functor(parameters.get<std::string>("function"), parameters.get<std::vector<std::string> >("vars"))
{
  std::vector<std::string> vars = parameters.get<std::vector<std::string> >("vars");
  std::vector<Real> vals = parameters.get<std::vector<Real> >("vals");
  for (int i = 0; i < vars.size(); i++)
    _functor.getVarAddr(vars[i]) = vals[i];
}

Real
ForcingFunction::f()
{
  return _functor(_t, _q_point[_qp](0), _q_point[_qp](1), _q_point[_qp](2));
}

Real
ForcingFunction::computeQpResidual()
{
  //return -_test[_i][_qp] * 16 * pi * pi * sin( pi * 4 *_q_point[_qp](0) );
  return -_test[_i][_qp] * f();
}

Real
ForcingFunction::computeQpJacobian()
{
  return 0;
}
