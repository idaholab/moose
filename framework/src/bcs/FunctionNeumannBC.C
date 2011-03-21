#include "FunctionNeumannBC.h"
#include "Function.h"

template<>
InputParameters validParams<FunctionNeumannBC>()
{
  InputParameters params = validParams<IntegratedBC>();
  params.addRequiredParam<std::string>("function", "The forcing function.");
  return params;
}

FunctionNeumannBC::FunctionNeumannBC(const std::string & name, InputParameters parameters) :
    IntegratedBC(name, parameters),
    _func(getFunction("function"))
{
}

Real
FunctionNeumannBC::computeQpResidual()
{
  return -_test[_i][_qp] * _normals[_qp] * _func.gradient(_t, _q_point[_qp](0), _q_point[_qp](1), _q_point[_qp](2));
}
