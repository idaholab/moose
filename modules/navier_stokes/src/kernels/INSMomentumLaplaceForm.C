//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSMomentumLaplaceForm.h"

registerMooseObject("NavierStokesApp", INSMomentumLaplaceForm);

InputParameters
INSMomentumLaplaceForm::validParams()
{
  InputParameters params = INSMomentumBase::validParams();
  params.addClassDescription("This class computes momentum equation residual and Jacobian viscous "
                             "contributions for the 'Laplacian' form of the governing equations.");
  return params;
}

INSMomentumLaplaceForm::INSMomentumLaplaceForm(const InputParameters & parameters)
  : INSMomentumBase(parameters)
{
}

Real
INSMomentumLaplaceForm::computeQpResidualViscousPart()
{
  // Simplified version: mu * Laplacian(u_component)
  return _mu[_qp] * (_grad_u[_qp] * _grad_test[_i][_qp]);
}

Real
INSMomentumLaplaceForm::computeQpJacobianViscousPart()
{
  // Viscous part, Laplacian version
  return _mu[_qp] * (_grad_phi[_j][_qp] * _grad_test[_i][_qp]);
}

Real
INSMomentumLaplaceForm::computeQpOffDiagJacobianViscousPart(unsigned /*jvar*/)
{
  return 0.;
}
