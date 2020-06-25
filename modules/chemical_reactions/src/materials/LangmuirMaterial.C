//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LangmuirMaterial.h"
#include "libmesh/utility.h"

registerMooseObject("ChemicalReactionsApp", LangmuirMaterial);

InputParameters
LangmuirMaterial::validParams()
{
  InputParameters params = Material::validParams();

  params.addRequiredCoupledVar(
      "one_over_desorption_time_const",
      "Time constant for Langmuir desorption (gas moving from matrix to porespace).  Units [s]");
  params.addRequiredCoupledVar(
      "one_over_adsorption_time_const",
      "Time constant for Langmuir adsorption (gas moving from porespace to matrix).  Units [s].");
  params.addRequiredParam<Real>("langmuir_density",
                                "This is (Langmuir volume)*(density of gas at standard temp and "
                                "pressure).  Langmuir volume is measured in (gas volume)/(matrix "
                                "volume).  (Methane density(101kPa, 20degC) = 0.655kg/m^3.  "
                                "Methane density(101kPa, 0degC) = 0.715kg/m^3.)  Units [kg/m^3]");
  params.addRequiredParam<Real>("langmuir_pressure", "Langmuir pressure.  Units Pa");
  params.addRequiredCoupledVar("conc_var", "The concentration of gas variable");
  params.addRequiredCoupledVar("pressure_var", "The gas porepressure variable");
  params.addClassDescription("Material type that holds info regarding Langmuir desorption from "
                             "matrix to porespace and viceversa");
  return params;
}

LangmuirMaterial::LangmuirMaterial(const InputParameters & parameters)
  : Material(parameters),
    _one_over_de_time_const(coupledValue("one_over_desorption_time_const")),
    _one_over_ad_time_const(coupledValue("one_over_adsorption_time_const")),

    _langmuir_dens(getParam<Real>("langmuir_density")),
    _langmuir_p(getParam<Real>("langmuir_pressure")),

    _conc(coupledValue("conc_var")),
    _pressure(coupledValue("pressure_var")),

    _mass_rate_from_matrix(declareProperty<Real>("mass_rate_from_matrix")),
    _dmass_rate_from_matrix_dC(declareProperty<Real>("dmass_rate_from_matrix_dC")),
    _dmass_rate_from_matrix_dp(declareProperty<Real>("dmass_rate_from_matrix_dp"))
{
}

void
LangmuirMaterial::computeQpProperties()
{
  Real equilib_conc = _langmuir_dens * (_pressure[_qp]) / (_langmuir_p + _pressure[_qp]);
  Real dequilib_conc_dp =
      _langmuir_dens / (_langmuir_p + _pressure[_qp]) -
      _langmuir_dens * (_pressure[_qp]) / Utility::pow<2>(_langmuir_p + _pressure[_qp]);

  // form the base rate and derivs without the appropriate time const
  _mass_rate_from_matrix[_qp] = _conc[_qp] - equilib_conc;
  _dmass_rate_from_matrix_dC[_qp] = 1.0;
  _dmass_rate_from_matrix_dp[_qp] = -dequilib_conc_dp;

  // multiply by the appropriate time const
  if (_conc[_qp] > equilib_conc)
  {
    _mass_rate_from_matrix[_qp] *= _one_over_de_time_const[_qp];
    _dmass_rate_from_matrix_dC[_qp] *= _one_over_de_time_const[_qp];
    _dmass_rate_from_matrix_dp[_qp] *= _one_over_de_time_const[_qp];
  }
  else
  {
    _mass_rate_from_matrix[_qp] *= _one_over_ad_time_const[_qp];
    _dmass_rate_from_matrix_dC[_qp] *= _one_over_ad_time_const[_qp];
    _dmass_rate_from_matrix_dp[_qp] *= _one_over_ad_time_const[_qp];
  }
}
