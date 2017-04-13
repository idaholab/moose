/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "DesorptionFromMatrix.h"

#include <iostream>

template <>
InputParameters
validParams<DesorptionFromMatrix>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredCoupledVar(
      "pressure_var",
      "Variable representing the porepressure of the fluid adsorbed into the matrix");
  params.addClassDescription("Mass flow rate from the matrix to the porespace.  Add this to "
                             "TimeDerivative kernel to get complete DE for the fluid in adsorbed "
                             "in the matrix");
  return params;
}

DesorptionFromMatrix::DesorptionFromMatrix(const InputParameters & parameters)
  : Kernel(parameters),
    _pressure_var(coupled("pressure_var")),
    _mass_rate_from_matrix(getMaterialProperty<Real>("mass_rate_from_matrix")),
    _dmass_rate_from_matrix_dC(getMaterialProperty<Real>("dmass_rate_from_matrix_dC")),
    _dmass_rate_from_matrix_dp(getMaterialProperty<Real>("dmass_rate_from_matrix_dp"))
{
}

Real
DesorptionFromMatrix::computeQpResidual()
{
  return _test[_i][_qp] * _mass_rate_from_matrix[_qp];
}

Real
DesorptionFromMatrix::computeQpJacobian()
{
  return _test[_i][_qp] * _dmass_rate_from_matrix_dC[_qp] * _phi[_j][_qp];
}

Real
DesorptionFromMatrix::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar != _pressure_var)
    return 0.0;
  return _test[_i][_qp] * _dmass_rate_from_matrix_dp[_qp] * _phi[_j][_qp];
}
