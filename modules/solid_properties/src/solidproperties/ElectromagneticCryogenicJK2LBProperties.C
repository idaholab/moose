//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElectromagneticCryogenicJK2LBProperties.h"
#include "libmesh/utility.h"

registerMooseObject("SolidPropertiesApp", ElectromagneticCryogenicJK2LBProperties);

InputParameters
ElectromagneticCryogenicJK2LBProperties::validParams()
{
  InputParameters params = ElectromagneticSolidProperties::validParams();

  params.addClassDescription(
      "Electromagnetic properties of JK2LB austenitic stainless steel for cryogenic "
      "applications. Electrical resistivity from Lu et al. (2009) quintic polynomial fit. "
      "Valid range: 2-300 K.");

  return params;
}

ElectromagneticCryogenicJK2LBProperties::ElectromagneticCryogenicJK2LBProperties(
    const InputParameters & parameters)
  : ElectromagneticSolidProperties(parameters),
    _T_min(2.0),
    _T_max(300.0),
    // Electrical resistivity coefficients from quintic polynomial fit
    // rho(T) = c0 + c1*T + c2*T^2 + c3*T^3 + c4*T^4 + c5*T^5 [Ohm·m]
    // Fitted via scipy.optimize.curve_fit to digitized data from Lu et al. (2009)
    // https://www.sciencedirect.com/science/article/pii/S0011227508001835
    // R^2 = 0.9987490979, RMSE = 8.03e-10 Ohm·m, Max Error = 2.25e-9 Ohm·m
    _rho_c0(9.128672625581926e-07),
    _rho_c1(-4.435542862339469e-10),
    _rho_c2(9.127900699537158e-12),
    _rho_c3(-5.183500588937069e-14),
    _rho_c4(1.137530802788486e-16),
    _rho_c5(-5.463764102978750e-20),
    // Magnetic permeability: mu_0 = 4*pi*10^-7 H/m
    // Austenitic stainless steel is non-magnetic (antiferromagnetic below 240K)
    _mu_const(1.25663706212e-06)
{
}

Real
ElectromagneticCryogenicJK2LBProperties::electrical_resistivity_from_T(const Real & T) const
{
  Real rho, drho_dT;
  electrical_resistivity_from_T(T, rho, drho_dT);
  return rho;
}

void
ElectromagneticCryogenicJK2LBProperties::electrical_resistivity_from_T(const Real & T,
                                                                       Real & rho,
                                                                       Real & drho_dT) const
{
  if ((T < _T_min) || (T > _T_max))
    flagInvalidSolution("Electrical resistivity evaluated outside valid range [" +
                        std::to_string(_T_min) + ", " + std::to_string(_T_max) + "] K");

  // Quintic polynomial: rho(T) = c0 + c1*T + c2*T^2 + c3*T^3 + c4*T^4 + c5*T^5
  const Real T2 = Utility::pow<2>(T);
  const Real T3 = Utility::pow<3>(T);
  const Real T4 = Utility::pow<4>(T);
  const Real T5 = Utility::pow<5>(T);

  rho = _rho_c0 + _rho_c1 * T + _rho_c2 * T2 + _rho_c3 * T3 + _rho_c4 * T4 + _rho_c5 * T5;

  // Derivative: drho/dT = c1 + 2*c2*T + 3*c3*T^2 + 4*c4*T^3 + 5*c5*T^4
  drho_dT =
      _rho_c1 + 2.0 * _rho_c2 * T + 3.0 * _rho_c3 * T2 + 4.0 * _rho_c4 * T3 + 5.0 * _rho_c5 * T4;
}

Real
ElectromagneticCryogenicJK2LBProperties::electrical_conductivity_from_T(const Real & T) const
{
  Real sigma, dsigma_dT;
  electrical_conductivity_from_T(T, sigma, dsigma_dT);
  return sigma;
}

void
ElectromagneticCryogenicJK2LBProperties::electrical_conductivity_from_T(const Real & T,
                                                                        Real & sigma,
                                                                        Real & dsigma_dT) const
{
  // Electrical conductivity is reciprocal of resistivity: sigma = 1/rho
  Real rho, drho_dT;
  electrical_resistivity_from_T(T, rho, drho_dT);

  sigma = 1.0 / rho;

  // Derivative: dsigma/dT = d(1/rho)/dT = -1/rho^2 * drho/dT (chain rule)
  dsigma_dT = -drho_dT / (rho * rho);
}

Real
ElectromagneticCryogenicJK2LBProperties::magnetic_permeability_from_T(const Real & T) const
{
  Real mu, dmu_dT;
  magnetic_permeability_from_T(T, mu, dmu_dT);
  return mu;
}

void
ElectromagneticCryogenicJK2LBProperties::magnetic_permeability_from_T(const Real & T,
                                                                      Real & mu,
                                                                      Real & dmu_dT) const
{
  if ((T < _T_min) || (T > _T_max))
    flagInvalidSolution("Magnetic permeability evaluated outside valid range [" +
                        std::to_string(_T_min) + ", " + std::to_string(_T_max) + "] K");

  // Constant: mu = mu_0 (austenitic steel is non-magnetic)
  mu = _mu_const;
  dmu_dT = 0.0;
}
