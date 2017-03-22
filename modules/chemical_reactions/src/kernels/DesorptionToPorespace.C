/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "DesorptionToPorespace.h"

#include <iostream>

template <>
InputParameters
validParams<DesorptionToPorespace>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredCoupledVar("conc_var",
                               "Variable representing the concentration (kg/m^3) of "
                               "fluid in the matrix that will be desorped to "
                               "porespace");
  params.addClassDescription("Mass flow rate to the porespace from the matrix.  Add this to the "
                             "other kernels for the porepressure variable to form the complete DE");
  return params;
}

DesorptionToPorespace::DesorptionToPorespace(const InputParameters & parameters)
  : Kernel(parameters),
    _conc_var(coupled("conc_var")),
    _mass_rate_from_matrix(getMaterialProperty<Real>("mass_rate_from_matrix")),
    _dmass_rate_from_matrix_dC(getMaterialProperty<Real>("dmass_rate_from_matrix_dC")),
    _dmass_rate_from_matrix_dp(getMaterialProperty<Real>("dmass_rate_from_matrix_dp"))
{
}

Real
DesorptionToPorespace::computeQpResidual()
{
  return -_test[_i][_qp] * _mass_rate_from_matrix[_qp];
}

Real
DesorptionToPorespace::computeQpJacobian()
{
  return -_test[_i][_qp] * _dmass_rate_from_matrix_dp[_qp] * _phi[_j][_qp];
}

Real
DesorptionToPorespace::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar != _conc_var)
    return 0.0;
  return -_test[_i][_qp] * _dmass_rate_from_matrix_dC[_qp] * _phi[_j][_qp];
}
