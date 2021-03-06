#include "VectorSecondTimeDerivative.h"
#include "Function.h"

registerMooseObject("ElkApp", VectorSecondTimeDerivative);

InputParameters
VectorSecondTimeDerivative::validParams()
{
  InputParameters params = VectorTimeKernel::validParams();
  params.addClassDescription(
      "The second time derivative operator with the weak form of $(\\vec{\\psi_i}, "
      "a(\\vec{r}, t) \\frac{\\partial^2 \\vec{u_h}}{\\partial t^2})$, where $a(\\vec{r}, t)$ is a "
      "coefficient (default, a(\\vec{r}, t) = 1.0).");
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
