/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "INSMomentumLaplaceForm.h"

template <>
InputParameters
validParams<INSMomentumLaplaceForm>()
{
  InputParameters params = validParams<INSMomentumBase>();
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
  return _mu * (_grad_u[_qp] * _grad_test[_i][_qp]);
}

Real
INSMomentumLaplaceForm::computeQpJacobianViscousPart()
{
  // Viscous part, Laplacian version
  return _mu * (_grad_phi[_j][_qp] * _grad_test[_i][_qp]);
}

Real
INSMomentumLaplaceForm::computeQpOffDiagJacobianViscousPart(unsigned /*jvar*/)
{
  return 0.;
}
