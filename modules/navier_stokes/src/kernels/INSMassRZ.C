/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "INSMassRZ.h"

template <>
InputParameters
validParams<INSMassRZ>()
{
  InputParameters params = validParams<INSMass>();
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
         RealVectorValue(_mu[_qp] * _u_vel[_qp] / (r * r), 0, 0);
}

RealVectorValue
INSMassRZ::dStrongViscDUCompLaplace(unsigned comp)
{
  const Real & r = _q_point[_qp](0);
  return INSBase::dStrongViscDUCompLaplace(comp) +
         RealVectorValue(comp == 0 ? _mu[_qp] * _phi[_j][_qp] / (r * r) : 0, 0, 0);
}

RealVectorValue
INSMassRZ::strongViscousTermTraction()
{
  const Real & r = _q_point[_qp](0);
  return INSBase::strongViscousTermTraction() +
         RealVectorValue(2. * _mu[_qp] * _u_vel[_qp] / (r * r), 0, 0);
}

RealVectorValue
INSMassRZ::dStrongViscDUCompTraction(unsigned comp)
{
  const Real & r = _q_point[_qp](0);
  return INSBase::dStrongViscDUCompTraction(comp) +
         RealVectorValue(comp == 0 ? 2. * _mu[_qp] * _phi[_j][_qp] / (r * r) : 0, 0, 0);
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
