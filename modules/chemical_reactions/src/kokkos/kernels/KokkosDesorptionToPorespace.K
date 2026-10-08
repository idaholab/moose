//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosDesorptionToPorespace.h"

registerKokkosResidualObject("ChemicalReactionsApp", KokkosDesorptionToPorespace);

InputParameters
KokkosDesorptionToPorespace::validParams()
{
  InputParameters params = KernelValue::validParams();
  params.addRequiredCoupledVar("conc_var",
                               "Variable representing the concentration (kg/m^3) of fluid in the "
                               "matrix that will be desorped to porespace");
  params.addClassDescription("Mass flow rate to the porespace from the matrix.  Add this to the "
                             "other kernels for the porepressure variable to form the complete DE");
  return params;
}

KokkosDesorptionToPorespace::KokkosDesorptionToPorespace(const InputParameters & parameters)
  : KernelValue(parameters),
    _coupled_var(coupled("conc_var")),
    _mass_rate_from_matrix(getKokkosMaterialProperty<Real>("mass_rate_from_matrix")),
    _dmass_rate_from_matrix_dC(getKokkosMaterialProperty<Real>("dmass_rate_from_matrix_dC")),
    _dmass_rate_from_matrix_dp(getKokkosMaterialProperty<Real>("dmass_rate_from_matrix_dp"))
{
}
