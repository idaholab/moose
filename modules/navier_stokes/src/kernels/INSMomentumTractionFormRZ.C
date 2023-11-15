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
  return INSBase::strongViscousTermTraction() + strongViscousTermTractionRZ();
}

RealVectorValue
INSMomentumTractionFormRZ::dStrongViscDUCompTraction(const unsigned int comp)
{
  return INSBase::dStrongViscDUCompTraction(comp) + dStrongViscDUCompTractionRZ(comp);
}

Real
INSMomentumTractionFormRZ::computeQpResidual()
{
  // Base class residual contribution
  Real res_base = INSMomentumTractionForm::computeQpResidual();

  if (_component == _rz_radial_coord)
  {
    const auto r = _q_point[_qp](_rz_radial_coord);
    const auto r_vel = (_rz_radial_coord == 0) ? _u_vel[_qp] : _v_vel[_qp];

    // If this is the radial component of momentum, there is an extra term for RZ.
    res_base += 2. * _mu[_qp] * r_vel / (r * r) * _test[_i][_qp];

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

  if (_component == _rz_radial_coord)
  {
    const auto r = _q_point[_qp](_rz_radial_coord);

    // If this is the radial component of momentum, there is an extra term for RZ.
    jac_base += 2. * _mu[_qp] * _phi[_j][_qp] / (r * r) * _test[_i][_qp];
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
  if (jvar == _p_var_number && _component == _rz_radial_coord && _integrate_p_by_parts)
  {
    const auto r = _q_point[_qp](_rz_radial_coord);
    jac_base += -_phi[_j][_qp] / r * _test[_i][_qp];
  }

  return jac_base;
}
