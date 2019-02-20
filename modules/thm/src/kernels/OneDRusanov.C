#include "OneDRusanov.h"

registerMooseObject("THMApp", OneDRusanov);

template <>
InputParameters
validParams<OneDRusanov>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredCoupledVar("vel", "The velocity of the kth phase, aux variable");

  params.addRequiredParam<MaterialPropertyName>("c", "Sound speed material property");

  return params;
}

OneDRusanov::OneDRusanov(const InputParameters & parameters)
  : Kernel(parameters),
    _c(getMaterialProperty<Real>("c")),
    _vel(coupledValue("vel")),
    _velocity_grad(coupledGradient("vel"))
{
}

Real
OneDRusanov::coef()
{
  Real h = _current_elem->hmax();

  // Rusanov (modified based on Low Mach number preconditioning)
  Real soundspeed = _c[_qp];
  Real lambda1_abs = std::fabs(soundspeed);
  Real lambda2_abs = std::fabs(soundspeed + _vel[_qp]);
  Real lambda3_abs = std::fabs(soundspeed - _vel[_qp]);
  Real lambda_abs_max = lambda1_abs;
  if (lambda2_abs > lambda_abs_max)
    lambda_abs_max = lambda2_abs;
  if (lambda3_abs > lambda_abs_max)
    lambda_abs_max = lambda3_abs;
  Real c = 0.5 * h * lambda_abs_max;

  Real Mach = _vel[_qp] / soundspeed;
  Real theta = Mach > 1.0 ? 1.0 : Mach;
  theta = theta > 1.e-3 ? theta : 1.e-3;

  return c / theta;
}

Real
OneDRusanov::computeQpResidual()
{
  return coef() * _grad_u[_qp] * _grad_test[_i][_qp];
}

Real
OneDRusanov::computeQpJacobian()
{
  return coef() * _grad_phi[_j][_qp] * _grad_test[_i][_qp];
}
