#include "CoupledGradBC.h"
#include "Function.h"

template <>
InputParameters
validParams<CoupledGradBC>()
{
  InputParameters params = validParams<IntegratedBC>();
  params.addParam<Real>(
      "sign",
      1.0,
      "Sign of the coupled gradient term in the weak form (1.0, -1.0, positive default)");
  params.addParam<Real>(
      "coefficient", 1.0, "Optional coefficient multiplier for coupled gradient term.");
  params.addParam<FunctionName>(
      "func", 1.0, "Optional function multiplier for coupled gradient term.");
  params.addRequiredCoupledVar("coupled_field", "Coupled field variable.");
  return params;
}

CoupledGradBC::CoupledGradBC(const InputParameters & parameters)
  : IntegratedBC(parameters),

    _sign(getParam<Real>("sign")),
    _coefficient(getParam<Real>("coefficient")),
    _func(getFunction("func")),
    _coupled_grad(coupledGradient("coupled_field"))
{
}

Real
CoupledGradBC::computeQpResidual()
{
  return _sign * _coefficient * _func.value(_t, _q_point[_qp]) * _coupled_grad[_qp] * _normals[_qp];
}

Real
CoupledGradBC::computeQpJacobian()
{
  return 0.0;
}
