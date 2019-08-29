#include "VectorSecondTimeDerivative.h"

registerMooseObject("ElkApp", VectorSecondTimeDerivative);

template <>
InputParameters
validParams<VectorSecondTimeDerivative>()
{
  InputParameters params = validParams<VectorTimeKernel>();
  params.addClassDescription(
      "The second time derivative operator with the weak form of $(\\vec{\\psi_i}, "
      "\\frac{\\partial^2 \\vec{u_h}}{\\partial t^2})$.");
  return params;
}

VectorSecondTimeDerivative::VectorSecondTimeDerivative(const InputParameters & parameters)
  : VectorTimeKernel(parameters), _u_dot_dot(dotDot()), _du_dot_dot_du(dotDotDu())
{
}

Real
VectorSecondTimeDerivative::computeQpResidual()
{
  return _test[_i][_qp] * _u_dot_dot[_qp];
}

Real
VectorSecondTimeDerivative::computeQpJacobian()
{
  return _test[_i][_qp] * _phi[_j][_qp] * _du_dot_dot_du[_qp];
}
