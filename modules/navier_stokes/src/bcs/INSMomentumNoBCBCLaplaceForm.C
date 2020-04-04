//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSMomentumNoBCBCLaplaceForm.h"
#include "MooseMesh.h"

registerMooseObject("NavierStokesApp", INSMomentumNoBCBCLaplaceForm);

InputParameters
INSMomentumNoBCBCLaplaceForm::validParams()
{
  InputParameters params = INSMomentumNoBCBCBase::validParams();

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
  Real viscous_part = -_mu[_qp] * (_grad_u[_qp] * _normals[_qp]) * _test[_i][_qp];

  // pIn * test
  Real pressure_part = 0.;
  if (_integrate_p_by_parts)
    pressure_part = _p[_qp] * _normals[_qp](_component) * _test[_i][_qp];

  return viscous_part + pressure_part;
}

Real
INSMomentumNoBCBCLaplaceForm::computeQpJacobian()
{
  return -_mu[_qp] * (_grad_phi[_j][_qp] * _normals[_qp]) * _test[_i][_qp];
}

Real
INSMomentumNoBCBCLaplaceForm::computeQpOffDiagJacobian(unsigned jvar)
{
  if (jvar == _p_var_number && _integrate_p_by_parts)
    return _phi[_j][_qp] * _normals[_qp](_component) * _test[_i][_qp];

  else
    return 0.;
}
