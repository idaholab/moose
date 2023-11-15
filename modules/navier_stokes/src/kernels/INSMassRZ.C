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
  return INSBase::strongViscousTermLaplace() + strongViscousTermLaplaceRZ();
}

RealVectorValue
INSMassRZ::dStrongViscDUCompLaplace(const unsigned int comp)
{
  return INSBase::dStrongViscDUCompLaplace(comp) + dStrongViscDUCompLaplaceRZ(comp);
}

RealVectorValue
INSMassRZ::strongViscousTermTraction()
{
  return INSBase::strongViscousTermTraction() + strongViscousTermTractionRZ();
}

RealVectorValue
INSMassRZ::dStrongViscDUCompTraction(const unsigned int comp)
{
  return INSBase::dStrongViscDUCompTraction(comp) + dStrongViscDUCompTractionRZ(comp);
}

Real
INSMassRZ::computeQpResidual()
{
  // Base class residual contribution
  auto res_base = INSMass::computeQpResidual();

  // The radial coordinate value.
  const auto r = _q_point[_qp](_rz_radial_coord);

  // The sign of this term is multiplied by -1 here.
  res_base -= ((_rz_radial_coord == 0) ? _u_vel[_qp] : _v_vel[_qp]) / r * _test[_i][_qp];

  return res_base;
}

Real
INSMassRZ::computeQpOffDiagJacobian(const unsigned int jvar)
{
  // Base class jacobian contribution
  auto jac_base = INSMass::computeQpOffDiagJacobian(jvar);

  // The radial coordinate value.
  const auto r = _q_point[_qp](_rz_radial_coord);

  if (jvar == ((_rz_radial_coord == 0) ? _u_vel_var_number : _v_vel_var_number))
    jac_base -= _phi[_j][_qp] / r * _test[_i][_qp];

  return jac_base;
}
