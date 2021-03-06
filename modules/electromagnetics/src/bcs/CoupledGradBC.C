#include "CoupledGradBC.h"
#include "Function.h"

registerMooseObject("ElkApp", CoupledGradBC);

InputParameters
CoupledGradBC::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  params.addClassDescription(
      "Imposes the integrated boundary condition arising from integration by parts of a Helmholtz "
      "equation, when that term is set equal to the gradient of a coupled variable. ");
  params.addParam<Real>(
      "sign",
      1.0,
      "Sign of the coupled gradient term in the weak form (1.0, -1.0, positive default)");
  params.addParam<Real>("coefficient", 1.0, "Optional coefficient for coupled gradient term.");
  params.addParam<FunctionName>(
      "func", 1.0, "Optional function coefficient for coupled gradient term.");
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
