#include "LinearVectorPenaltyDirichletBC.h"
#include "Function.h"

template <>
InputParameters
validParams<LinearVectorPenaltyDirichletBC>()
{
  InputParameters params = validParams<VectorIntegratedBC>();
  params.addRequiredParam<Real>("penalty", "The penalty coefficient");
  params.addParam<FunctionName>("x_exact_sln", 0, "The exact solution for the x component");
  params.addParam<FunctionName>("y_exact_sln", 0, "The exact solution for the y component");
  return params;
}

LinearVectorPenaltyDirichletBC::LinearVectorPenaltyDirichletBC(const InputParameters & parameters)
  : VectorIntegratedBC(parameters),
    _penalty(getParam<Real>("penalty")),
    _exact_x(getFunction("x_exact_sln")),
    _exact_y(getFunction("y_exact_sln"))
{
}

Real
LinearVectorPenaltyDirichletBC::computeQpResidual()
{
  RealVectorValue u_exact = {
      _exact_x.value(_t, _q_point[_qp]), _exact_y.value(_t, _q_point[_qp]), 0};

  return -_penalty * _test[_i][_qp] * u_exact;
}

Real
LinearVectorPenaltyDirichletBC::computeQpJacobian()
{
  return _penalty * _test[_i][_qp] * _phi[_j][_qp];
}
