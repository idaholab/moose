//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DesorptionToPorespace.h"

#include <iostream>

registerMooseObject("ChemicalReactionsApp", DesorptionToPorespace);

InputParameters
DesorptionToPorespace::validParams()
{
  InputParameters params = Kernel::validParams();
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
