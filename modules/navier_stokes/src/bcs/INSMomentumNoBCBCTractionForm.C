//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSMomentumNoBCBCTractionForm.h"
#include "MooseMesh.h"

registerMooseObject("NavierStokesApp", INSMomentumNoBCBCTractionForm);

InputParameters
INSMomentumNoBCBCTractionForm::validParams()
{
  InputParameters params = INSMomentumNoBCBCBase::validParams();

  params.addClassDescription("This class implements the 'No BC' boundary condition based on the "
                             "'traction' form of the viscous stress tensor.");
  return params;
}

INSMomentumNoBCBCTractionForm::INSMomentumNoBCBCTractionForm(const InputParameters & parameters)
  : INSMomentumNoBCBCBase(parameters)
{
}

Real
INSMomentumNoBCBCTractionForm::computeQpResidual()
{
  // Compute n . sigma . v, where n is unit normal and v is the test function.
  RealTensorValue sigma;

  // First row
  sigma(0, 0) = 2. * _mu[_qp] * _grad_u_vel[_qp](0);
  sigma(0, 1) = _mu[_qp] * (_grad_u_vel[_qp](1) + _grad_v_vel[_qp](0));
  sigma(0, 2) = _mu[_qp] * (_grad_u_vel[_qp](2) + _grad_w_vel[_qp](0));

  // Second row
  sigma(1, 0) = _mu[_qp] * (_grad_v_vel[_qp](0) + _grad_u_vel[_qp](1));
  sigma(1, 1) = 2. * _mu[_qp] * _grad_v_vel[_qp](1);
  sigma(1, 2) = _mu[_qp] * (_grad_v_vel[_qp](2) + _grad_w_vel[_qp](1));

  // Third row
  sigma(2, 0) = _mu[_qp] * (_grad_w_vel[_qp](0) + _grad_u_vel[_qp](2));
  sigma(2, 1) = _mu[_qp] * (_grad_w_vel[_qp](1) + _grad_v_vel[_qp](2));
  sigma(2, 2) = 2. * _mu[_qp] * _grad_w_vel[_qp](2);

  // If the pressure term is integrated by parts, it is part of the
  // no-BC-BC, otherwise, it is not.
  if (_integrate_p_by_parts)
  {
    sigma(0, 0) -= _p[_qp];
    sigma(1, 1) -= _p[_qp];
    sigma(2, 2) -= _p[_qp];
  }

  // Set up test function
  RealVectorValue test;
  test(_component) = _test[_i][_qp];

  return -_normals[_qp] * (sigma * test);
}

Real
INSMomentumNoBCBCTractionForm::computeQpJacobian()
{
  // The extra contribution comes from the "2" on the diagonal of the viscous stress tensor
  return -_mu[_qp] *
         (_grad_phi[_j][_qp] * _normals[_qp] +
          _grad_phi[_j][_qp](_component) * _normals[_qp](_component)) *
         _test[_i][_qp];
}

Real
INSMomentumNoBCBCTractionForm::computeQpOffDiagJacobian(unsigned jvar)
{
  if (jvar == _u_vel_var_number)
    return -_mu[_qp] * _grad_phi[_j][_qp](_component) * _normals[_qp](0) * _test[_i][_qp];

  else if (jvar == _v_vel_var_number)
    return -_mu[_qp] * _grad_phi[_j][_qp](_component) * _normals[_qp](1) * _test[_i][_qp];

  else if (jvar == _w_vel_var_number)
    return -_mu[_qp] * _grad_phi[_j][_qp](_component) * _normals[_qp](2) * _test[_i][_qp];

  else if (jvar == _p_var_number)
  {
    if (_integrate_p_by_parts)
      return _phi[_j][_qp] * _normals[_qp](_component) * _test[_i][_qp];
    else
      return 0.;
  }

  else
    return 0.;
}
