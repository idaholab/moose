//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSMomentumTractionFormRZ.h"

registerMooseObject("NavierStokesApp", INSMomentumTractionFormRZ);

InputParameters
INSMomentumTractionFormRZ::validParams()
{
  InputParameters params = INSMomentumTractionForm::validParams();
  params.addClassDescription("This class computes additional momentum equation residual and "
                             "Jacobian contributions for the incompressible Navier-Stokes momentum "
                             "equation in RZ (axisymmetric cylindrical) coordinates.");
  return params;
}

INSMomentumTractionFormRZ::INSMomentumTractionFormRZ(const InputParameters & parameters)
  : INSMomentumTractionForm(parameters)
{
}

RealVectorValue
INSMomentumTractionFormRZ::strongViscousTermTraction()
{
  const Real & r = _q_point[_qp](0);
  return INSBase::strongViscousTermTraction() +
         RealVectorValue(2. * _mu[_qp] * (_u_vel[_qp] / (r * r) - _grad_u_vel[_qp](0) / r),
                         -_mu[_qp] / r * (_grad_v_vel[_qp](0) + _grad_u_vel[_qp](1)),
                         0);
}

RealVectorValue
INSMomentumTractionFormRZ::dStrongViscDUCompTraction(unsigned comp)
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
INSMomentumTractionFormRZ::computeQpResidual()
{
  // Base class residual contribution
  Real res_base = INSMomentumTractionForm::computeQpResidual();

  if (_component == 0)
  {
    const Real r = _q_point[_qp](0);

    // If this is the radial component of momentum, there is an extra term for RZ.
    res_base += 2. * _mu[_qp] * _u_vel[_qp] / (r * r) * _test[_i][_qp];

    // If the pressure is also integrated by parts, there is an extra term in RZ.
    if (_integrate_p_by_parts)
      res_base += -_p[_qp] / r * _test[_i][_qp];
  }

  return res_base;
}

Real
INSMomentumTractionFormRZ::computeQpJacobian()
{
  // Base class jacobian contribution
  Real jac_base = INSMomentumTractionForm::computeQpJacobian();

  // If this is the radial component of momentum, there is an extra term for RZ.
  if (_component == 0)
  {
    const Real r = _q_point[_qp](0);
    jac_base += 2. * _mu[_qp] * _phi[_j][_qp] * _test[_i][_qp] / (r * r);
  }

  return jac_base;
}

Real
INSMomentumTractionFormRZ::computeQpOffDiagJacobian(unsigned jvar)
{
  // Base class jacobian contribution
  Real jac_base = INSMomentumTractionForm::computeQpOffDiagJacobian(jvar);

  // If we're getting the pressure Jacobian contribution, and we
  // integrated the pressure term by parts, there is an extra term for
  // RZ.
  if (jvar == _p_var_number && _component == 0 && _integrate_p_by_parts)
  {
    const Real r = _q_point[_qp](0);
    jac_base += -_phi[_j][_qp] / r * _test[_i][_qp];
  }

  return jac_base;
}
