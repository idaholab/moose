//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSMassRZ.h"

registerMooseObject("NavierStokesApp", INSMassRZ);

InputParameters
INSMassRZ::validParams()
{
  InputParameters params = INSMass::validParams();
  params.addClassDescription("This class computes the mass equation residual and Jacobian "
                             "contributions for the incompressible Navier-Stokes momentum equation "
                             "in RZ coordinates.");
  return params;
}

INSMassRZ::INSMassRZ(const InputParameters & parameters) : INSMass(parameters) {}

RealVectorValue
INSMassRZ::strongViscousTermLaplace()
{
  const Real & r = _q_point[_qp](0);
  return INSBase::strongViscousTermLaplace() +
         RealVectorValue(_mu[_qp] * (_u_vel[_qp] / (r * r) - _grad_u_vel[_qp](0) / r),
                         -_mu[_qp] * _grad_v_vel[_qp](0) / r,
                         0);
}

RealVectorValue
INSMassRZ::dStrongViscDUCompLaplace(unsigned comp)
{
  const Real & r = _q_point[_qp](0);
  RealVectorValue add_jac(0, 0, 0);
  if (comp == 0)
    add_jac(0) = _mu[_qp] * (_phi[_j][_qp] / (r * r) - _grad_phi[_j][_qp](0) / r);
  else if (comp == 1)
    add_jac(1) = -_mu[_qp] * _grad_phi[_j][_qp](0) / r;

  return INSBase::dStrongViscDUCompLaplace(comp) + add_jac;
}

RealVectorValue
INSMassRZ::strongViscousTermTraction()
{
  const Real & r = _q_point[_qp](0);
  return INSBase::strongViscousTermTraction() +
         RealVectorValue(2. * _mu[_qp] * (_u_vel[_qp] / (r * r) - _grad_u_vel[_qp](0) / r),
                         -_mu[_qp] / r * (_grad_v_vel[_qp](0) + _grad_u_vel[_qp](1)),
                         0);
}

RealVectorValue
INSMassRZ::dStrongViscDUCompTraction(unsigned comp)
{
  const Real & r = _q_point[_qp](0);
  RealVectorValue add_jac(0, 0, 0);
  if (comp == 0)
  {
    add_jac(0) = 2. * _mu[_qp] * (_phi[_j][_qp] / (r * r) - _grad_phi[_j][_qp](0) / r);
    add_jac(1) = -_mu[_qp] / r * _grad_phi[_j][_qp](1);
  }
  else if (comp == 1)
    add_jac(1) = -_mu[_qp] * _grad_phi[_j][_qp](0) / r;

  return INSBase::dStrongViscDUCompTraction(comp) + add_jac;
}

Real
INSMassRZ::computeQpResidual()
{
  // Base class residual contribution
  Real res_base = INSMass::computeQpResidual();

  // The radial coordinate value.
  const Real r = _q_point[_qp](0);

  // The sign of this term is multiplied by -1 here.
  res_base -= _u_vel[_qp] / r * _test[_i][_qp];

  return res_base;
}

Real
INSMassRZ::computeQpOffDiagJacobian(unsigned jvar)
{
  // Base class jacobian contribution
  Real jac_base = INSMass::computeQpOffDiagJacobian(jvar);

  // The radial coordinate value.
  const Real r = _q_point[_qp](0);

  if (jvar == _u_vel_var_number)
    jac_base -= _phi[_j][_qp] / r * _test[_i][_qp];

  return jac_base;
}
