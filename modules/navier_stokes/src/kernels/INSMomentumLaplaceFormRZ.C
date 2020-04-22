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
  const Real & r = _q_point[_qp](0);
  return INSBase::strongViscousTermLaplace() +
         // To understand the code below, visit
         // https://en.wikipedia.org/wiki/Del_in_cylindrical_and_spherical_coordinates.
         // The u_r / r^2 term comes from the vector Laplacian. The -du_r/dr * 1/r term comes from
         // the scalar Laplacian. The scalar Laplacian in axisymmetric cylindrical coordinates is
         // equivalent to the Cartesian Laplacian plus a 1/r * df/dr term. And of course we are
         // applying a minus sign here because the strong form is -\nabala^2 * \vec{u}
         RealVectorValue(_mu[_qp] * (_u_vel[_qp] / (r * r) - _grad_u_vel[_qp](0) / r),
                         // Again we need the 1/r * df/dr term
                         -_mu[_qp] * _grad_v_vel[_qp](0) / r,
                         0);
}

RealVectorValue
INSMomentumLaplaceFormRZ::dStrongViscDUCompLaplace(unsigned comp)
{
  const Real & r = _q_point[_qp](0);
  RealVectorValue add_jac(0, 0, 0);
  if (comp == 0)
    add_jac(0) = _mu[_qp] * (_phi[_j][_qp] / (r * r) - _grad_phi[_j][_qp](0) / r);
  else if (comp == 1)
    add_jac(1) = -_mu[_qp] * _grad_phi[_j][_qp](0) / r;

  return INSBase::dStrongViscDUCompLaplace(comp) + add_jac;
}

Real
INSMomentumLaplaceFormRZ::computeQpResidual()
{
  // Base class residual contribution
  Real res_base = INSMomentumLaplaceForm::computeQpResidual();

  if (_component == 0)
  {
    const Real r = _q_point[_qp](0);

    // If this is the radial component of momentum, there is an extra term for RZ.
    // The only difference between this and the traction form is a factor of 2.
    res_base += _mu[_qp] * _u_vel[_qp] / (r * r) * _test[_i][_qp];

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
  if (_component == 0)
  {
    const Real r = _q_point[_qp](0);
    // The only difference between this and the traction form is a factor of 2.
    jac_base += _mu[_qp] * _phi[_j][_qp] * _test[_i][_qp] / (r * r);
  }

  return jac_base;
}

Real
INSMomentumLaplaceFormRZ::computeQpOffDiagJacobian(unsigned jvar)
{
  // Base class jacobian contribution
  Real jac_base = INSMomentumLaplaceForm::computeQpOffDiagJacobian(jvar);

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
