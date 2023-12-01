//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSMomentumLaplaceFormRZ.h"

registerMooseObject("NavierStokesApp", INSMomentumLaplaceFormRZ);

InputParameters
INSMomentumLaplaceFormRZ::validParams()
{
  InputParameters params = INSMomentumLaplaceForm::validParams();
  params.addClassDescription("This class computes additional momentum equation residual and "
                             "Jacobian contributions for the incompressible Navier-Stokes momentum "
                             "equation in RZ (axisymmetric cylindrical) coordinates, using the "
                             "'Laplace' form of the governing equations.");
  return params;
}

INSMomentumLaplaceFormRZ::INSMomentumLaplaceFormRZ(const InputParameters & parameters)
  : INSMomentumLaplaceForm(parameters)
{
}

RealVectorValue
INSMomentumLaplaceFormRZ::strongViscousTermLaplace()
{
  return INSBase::strongViscousTermLaplace() + strongViscousTermLaplaceRZ();
}

RealVectorValue
INSMomentumLaplaceFormRZ::dStrongViscDUCompLaplace(const unsigned int comp)
{
  return INSBase::dStrongViscDUCompLaplace(comp) + dStrongViscDUCompLaplaceRZ(comp);
}

Real
INSMomentumLaplaceFormRZ::computeQpResidual()
{
  // Base class residual contribution
  Real res_base = INSMomentumLaplaceForm::computeQpResidual();

  if (_component == _rz_radial_coord)
  {
    const auto r = _q_point[_qp](_rz_radial_coord);

    // If this is the radial component of momentum, there is an extra term for RZ.
    // The only difference between this and the traction form is a factor of 2.
    res_base +=
        _mu[_qp] * ((_rz_radial_coord == 0) ? _u_vel[_qp] : _v_vel[_qp]) / (r * r) * _test[_i][_qp];

    // If the pressure is also integrated by parts, there is an extra term in RZ.
    if (_integrate_p_by_parts)
      res_base += -_p[_qp] / r * _test[_i][_qp];
  }

  return res_base;
}

Real
INSMomentumLaplaceFormRZ::computeQpJacobian()
{
  // Base class jacobian contribution
  Real jac_base = INSMomentumLaplaceForm::computeQpJacobian();

  // If this is the radial component of momentum, there is an extra term for RZ.
  if (_component == _rz_radial_coord)
  {
    const auto r = _q_point[_qp](_rz_radial_coord);
    // The only difference between this and the traction form is a factor of 2.
    jac_base += _mu[_qp] * _phi[_j][_qp] * _test[_i][_qp] / (r * r);
  }

  return jac_base;
}

Real
INSMomentumLaplaceFormRZ::computeQpOffDiagJacobian(const unsigned int jvar)
{
  // Base class jacobian contribution
  Real jac_base = INSMomentumLaplaceForm::computeQpOffDiagJacobian(jvar);

  // If we're getting the pressure Jacobian contribution, and we
  // integrated the pressure term by parts, there is an extra term for
  // RZ.
  if ((jvar == _p_var_number) && (_component == _rz_radial_coord) && _integrate_p_by_parts)
  {
    const auto r = _q_point[_qp](_rz_radial_coord);
    jac_base += -_phi[_j][_qp] / r * _test[_i][_qp];
  }

  return jac_base;
}
