#include "FunctionNeumannBC.h"
#include "Function.h"

template<>
InputParameters validParams<FunctionNeumannBC>()
{
  InputParameters params = validParams<BoundaryCondition>();
  params.addRequiredParam<std::string>("function", "The forcing function.");
  return params;
}

FunctionNeumannBC::FunctionNeumannBC(std::string name,
                       MooseSystem &sys,
                       InputParameters parameters)
  :BoundaryCondition(name, sys, parameters),
  _func(getFunction("function"))
{
}

Real
FunctionNeumannBC::f()
{

  return _func(_t, _q_point[_qp](0), _q_point[_qp](1), _q_point[_qp](2));
}

Real
FunctionNeumannBC::computeQpResidual()
{
  return -_test[_i][_qp] * f();
}
