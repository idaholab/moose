#include "VectorSecondTimeDerivative.h"

registerMooseObject("ElkApp", VectorSecondTimeDerivative);

template <>
InputParameters
validParams<VectorSecondTimeDerivative>()
{
  InputParameters params = validParams<VectorTimeKernel>();
  params.addClassDescription(
      "The second time derivative operator with the weak form of $(\\vec{\\psi_i}, "
      "a(\\vec{r}) \\frac{\\partial^2 \\vec{u_h}}{\\partial t^2})$, where $a(\\vec{r})$ is a "
      "coefficient (default, a(\\vec{r}) = 1.0).");
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
