//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosLangmuirMaterial.h"

registerKokkosMaterial("ChemicalReactionsApp", KokkosLangmuirMaterial);

InputParameters
KokkosLangmuirMaterial::validParams()
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

KokkosLangmuirMaterial::KokkosLangmuirMaterial(const InputParameters & parameters)
  : Material(parameters),
    _one_over_de_time_const(kokkosCoupledValue("one_over_desorption_time_const")),
    _one_over_ad_time_const(kokkosCoupledValue("one_over_adsorption_time_const")),
    _langmuir_dens(getParam<Real>("langmuir_density")),
    _langmuir_p(getParam<Real>("langmuir_pressure")),
    _conc(kokkosCoupledValue("conc_var")),
    _pressure(kokkosCoupledValue("pressure_var")),
    _mass_rate_from_matrix(declareKokkosProperty<Real>("mass_rate_from_matrix")),
    _dmass_rate_from_matrix_dC(declareKokkosProperty<Real>("dmass_rate_from_matrix_dC")),
    _dmass_rate_from_matrix_dp(declareKokkosProperty<Real>("dmass_rate_from_matrix_dp"))
{
}
