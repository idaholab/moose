//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MollifiedLangmuirMaterial.h"

registerMooseObject("ChemicalReactionsApp", MollifiedLangmuirMaterial);

InputParameters
MollifiedLangmuirMaterial::validParams()
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
  params.addRangeCheckedParam<Real>("mollifier",
                                    0.1,
                                    "mollifier > 0",
                                    "The reciprocal of time constants will be "
                                    "one_over_time_const*tanh( |conc_var - "
                                    "equilib_conc|/(mollifier*langmuir_density)).  So for "
                                    "mollifier very small you will get a stepchange between "
                                    "desorption and adsorption, but for mollifier bigger you "
                                    "will be a gradual change");
  params.addClassDescription("Material type that holds info regarding MollifiedLangmuir desorption "
                             "from matrix to porespace and viceversa");
  return params;
}

MollifiedLangmuirMaterial::MollifiedLangmuirMaterial(const InputParameters & parameters)
  : Material(parameters),
    // coupledValue returns a reference (an alias) to a VariableValue, and the & turns it into a
    // pointer
    _one_over_de_time_const(coupledValue("one_over_desorption_time_const")),
    _one_over_ad_time_const(coupledValue("one_over_adsorption_time_const")),

    _langmuir_dens(getParam<Real>("langmuir_density")),
    _langmuir_p(getParam<Real>("langmuir_pressure")),

    _conc(coupledValue("conc_var")),
    _pressure(coupledValue("pressure_var")),

    _mollifier(getParam<Real>("mollifier")),

    _mass_rate_from_matrix(declareProperty<Real>("mass_rate_from_matrix")),
    _dmass_rate_from_matrix_dC(declareProperty<Real>("dmass_rate_from_matrix_dC")),
    _dmass_rate_from_matrix_dp(declareProperty<Real>("dmass_rate_from_matrix_dp"))
{
}

void
MollifiedLangmuirMaterial::computeQpProperties()
{
  Real equilib_conc = _langmuir_dens * (_pressure[_qp]) / (_langmuir_p + _pressure[_qp]);
  Real dequilib_conc_dp =
      _langmuir_dens / (_langmuir_p + _pressure[_qp]) -
      _langmuir_dens * (_pressure[_qp]) / std::pow(_langmuir_p + _pressure[_qp], 2);

  Real mol = std::tanh(std::abs(_conc[_qp] - equilib_conc) / (_mollifier * _langmuir_dens));
  Real deriv_tanh =
      1 - std::pow(std::tanh((_conc[_qp] - equilib_conc) / (_mollifier * _langmuir_dens)), 2);
  if (_conc[_qp] < equilib_conc)
    deriv_tanh *= -1;
  Real dmol_dC = deriv_tanh / (_mollifier * _langmuir_dens);
  Real dmol_dp = -dmol_dC * dequilib_conc_dp;

  /*
  Real de_plus_ad = _one_over_de_time_const[_qp] + _one_over_ad_time_const[_qp];
  Real de_minus_ad = _one_over_de_time_const[_qp] - _one_over_ad_time_const[_qp];

  Real one_over_tau = 0.5*de_plus_ad + 0.5*de_minus_ad*std::tanh( (_conc[_qp] -
  equilib_conc)/(_mollifier*_langmuir_dens));
  Real deriv_tanh = 1 - std::pow(std::tanh((_conc[_qp] -
  equilib_conc)/(_mollifier*_langmuir_dens)), 2);
  Real d_one_over_tau_dC = 0.5*de_minus_ad*deriv_tanh/(_mollifier*_langmuir_dens);
  Real d_one_over_tau_dp = -0.5*de_minus_ad*dequilib_conc_dp*deriv_tanh/(_mollifier*_langmuir_dens);
  */

  // form the base rate and derivs without the appropriate time const
  _mass_rate_from_matrix[_qp] = (_conc[_qp] - equilib_conc) * mol;
  _dmass_rate_from_matrix_dC[_qp] = mol + (_conc[_qp] - equilib_conc) * dmol_dC;
  _dmass_rate_from_matrix_dp[_qp] = -dequilib_conc_dp * mol + (_conc[_qp] - equilib_conc) * dmol_dp;

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
