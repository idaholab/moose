/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "INSMomentumTractionForm.h"

template <>
InputParameters
validParams<INSMomentumTractionForm>()
{
  InputParameters params = validParams<INSMomentumBase>();
  params.addClassDescription("This class computes momentum equation residual and Jacobian viscous "
                             "contributions for the 'traction' form of the governing equations.");
  return params;
}

INSMomentumTractionForm::INSMomentumTractionForm(const InputParameters & parameters)
  : INSMomentumBase(parameters)
{
}

Real
INSMomentumTractionForm::computeQpResidualViscousPart()
{
  // The component'th row (or col, it's symmetric) of the viscous stress tensor
  RealVectorValue tau_row;

  switch (_component)
  {
    case 0:
      tau_row(0) = 2. * _grad_u_vel[_qp](0);                  // 2*du/dx1
      tau_row(1) = _grad_u_vel[_qp](1) + _grad_v_vel[_qp](0); // du/dx2 + dv/dx1
      tau_row(2) = _grad_u_vel[_qp](2) + _grad_w_vel[_qp](0); // du/dx3 + dw/dx1
      break;

    case 1:
      tau_row(0) = _grad_v_vel[_qp](0) + _grad_u_vel[_qp](1); // dv/dx1 + du/dx2
      tau_row(1) = 2. * _grad_v_vel[_qp](1);                  // 2*dv/dx2
      tau_row(2) = _grad_v_vel[_qp](2) + _grad_w_vel[_qp](1); // dv/dx3 + dw/dx2
      break;

    case 2:
      tau_row(0) = _grad_w_vel[_qp](0) + _grad_u_vel[_qp](2); // dw/dx1 + du/dx3
      tau_row(1) = _grad_w_vel[_qp](1) + _grad_v_vel[_qp](2); // dw/dx2 + dv/dx3
      tau_row(2) = 2. * _grad_w_vel[_qp](2);                  // 2*dw/dx3
      break;

    default:
      mooseError("Unrecognized _component requested.");
  }

  // The viscous part, _mu * tau : grad(v)
  return _mu * (tau_row * _grad_test[_i][_qp]);
}

Real
INSMomentumTractionForm::computeQpJacobianViscousPart()
{
  // Viscous part, full stress tensor.  The extra contribution comes from the "2"
  // on the diagonal of the viscous stress tensor.
  return _mu * (_grad_phi[_j][_qp] * _grad_test[_i][_qp] +
                _grad_phi[_j][_qp](_component) * _grad_test[_i][_qp](_component));
}

Real
INSMomentumTractionForm::computeQpOffDiagJacobianViscousPart(unsigned jvar)
{
  // In Stokes/Laplacian version, off-diag Jacobian entries wrt u,v,w are zero
  if (jvar == _u_vel_var_number)
    return _mu * _grad_phi[_j][_qp](_component) * _grad_test[_i][_qp](0);

  else if (jvar == _v_vel_var_number)
    return _mu * _grad_phi[_j][_qp](_component) * _grad_test[_i][_qp](1);

  else if (jvar == _w_vel_var_number)
    return _mu * _grad_phi[_j][_qp](_component) * _grad_test[_i][_qp](2);

  else
    return 0;
}
