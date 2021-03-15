#include "VectorSecondTimeDerivative.h"
#include "Function.h"

registerMooseObject("ElkApp", VectorSecondTimeDerivative);

InputParameters
VectorSecondTimeDerivative::validParams()
{
  InputParameters params = VectorTimeKernel::validParams();
  params.addClassDescription(
      "The second time derivative operator for vector variables.");
  params.addParam<FunctionName>("coefficient", 1.0, "Coefficient function.");
  return params;
}

VectorSecondTimeDerivative::VectorSecondTimeDerivative(const InputParameters & parameters)
  : VectorTimeKernel(parameters),
    _u_dot_dot(dotDot()),
    _du_dot_dot_du(dotDotDu()),
    _coeff(getFunction("coefficient"))
{
}

Real
VectorSecondTimeDerivative::computeQpResidual()
{
  return _test[_i][_qp] * _coeff.value(_t, _q_point[_qp]) * _u_dot_dot[_qp];
}

Real
VectorSecondTimeDerivative::computeQpJacobian()
{
  return _test[_i][_qp] * _coeff.value(_t, _q_point[_qp]) * _phi[_j][_qp] * _du_dot_dot_du[_qp];
}
