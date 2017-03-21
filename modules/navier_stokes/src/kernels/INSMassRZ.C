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

INSMassRZ::INSMassRZ(const InputParameters & parameters)
  : INSMass(parameters), _u_vel(coupledValue("u"))
{
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
