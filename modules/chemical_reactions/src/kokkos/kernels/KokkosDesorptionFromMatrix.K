//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosDesorptionFromMatrix.h"

registerKokkosResidualObject("ChemicalReactionsApp", KokkosDesorptionFromMatrix);

InputParameters
KokkosDesorptionFromMatrix::validParams()
{
  InputParameters params = KernelValue::validParams();
  params.addRequiredCoupledVar(
      "pressure_var",
      "Variable representing the porepressure of the fluid adsorbed into the matrix");
  params.addClassDescription(
      "Mass flow rate from the matrix to the porespace.  Add this to TimeDerivative kernel to get "
      "complete DE for the fluid adsorbed in the matrix");
  return params;
}

KokkosDesorptionFromMatrix::KokkosDesorptionFromMatrix(const InputParameters & parameters)
  : KernelValue(parameters),
    _coupled_var(coupled("pressure_var")),
    _mass_rate_from_matrix(getKokkosMaterialProperty<Real>("mass_rate_from_matrix")),
    _dmass_rate_from_matrix_dC(getKokkosMaterialProperty<Real>("dmass_rate_from_matrix_dC")),
    _dmass_rate_from_matrix_dp(getKokkosMaterialProperty<Real>("dmass_rate_from_matrix_dp"))
{
}
