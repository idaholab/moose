/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "INSMomentumNoBCBCLaplaceForm.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<INSMomentumNoBCBCLaplaceForm>()
{
  InputParameters params = validParams<INSMomentumNoBCBCBase>();

  params.addClassDescription("This class implements the 'No BC' boundary condition based on the "
                             "'Laplace' form of the viscous stress tensor.");
  return params;
}

INSMomentumNoBCBCLaplaceForm::INSMomentumNoBCBCLaplaceForm(const InputParameters & parameters)
  : INSMomentumNoBCBCBase(parameters)
{
}

Real
INSMomentumNoBCBCLaplaceForm::computeQpResidual()
{
  // -mu * (grad(u).n) * test
  Real viscous_part = -_mu * (_grad_u[_qp] * _normals[_qp]) * _test[_i][_qp];

  // pIn * test
  Real pressure_part = 0.;
  if (_integrate_p_by_parts)
    pressure_part = _p[_qp] * _normals[_qp](_component) * _test[_i][_qp];

  return viscous_part + pressure_part;
}

Real
INSMomentumNoBCBCLaplaceForm::computeQpJacobian()
{
  return -_mu * (_grad_phi[_j][_qp] * _normals[_qp]) * _test[_i][_qp];
}

Real
INSMomentumNoBCBCLaplaceForm::computeQpOffDiagJacobian(unsigned jvar)
{
  if (jvar == _p_var_number && _integrate_p_by_parts)
    return _phi[_j][_qp] * _normals[_qp](_component) * _test[_i][_qp];

  else
    return 0.;
}
