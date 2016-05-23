/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/**********************************************2******************/

#include "PorousFlowBrine.h"

template<>
InputParameters validParams<PorousFlowBrine>()
{
  InputParameters params = validParams<PorousFlowWater>();
  params.addRequiredParam<Real>("xnacl", "The salt mass fraction in the brine (kg/kg)");
  params.addClassDescription("This Material calculates fluid properties for brine");
  return params;
}

PorousFlowBrine::PorousFlowBrine(const InputParameters & parameters) :
    PorousFlowWater(parameters),

    _Mnacl(58.443e-3),
    _xnacl(getParam<Real>("xnacl"))
{
}

void
PorousFlowBrine::initQpStatefulProperties()
{
  _density_nodal[_qp] = density(_porepressure_nodal[_qp][_phase_num], _temperature_nodal[_qp][_phase_num], _xnacl);
}

void
PorousFlowBrine::computeQpProperties()
{
  /// Density and derivatives wrt pressure and temperature at the nodes
  _density_nodal[_qp] = density(_porepressure_nodal[_qp][_phase_num], _temperature_nodal[_qp][_phase_num], _xnacl);
  _ddensity_nodal_dp[_qp] = dDensity_dP(_porepressure_nodal[_qp][_phase_num], _temperature_nodal[_qp][_phase_num], _xnacl);
  _ddensity_nodal_dt[_qp] = dDensity_dT(_porepressure_nodal[_qp][_phase_num], _temperature_nodal[_qp][_phase_num], _xnacl);

  /// Density and derivatives wrt pressure and temperature at the qps
  _density_qp[_qp] = density(_porepressure_qp[_qp][_phase_num], _temperature_qp[_qp][_phase_num], _xnacl);
  _ddensity_qp_dp[_qp] = dDensity_dP(_porepressure_qp[_qp][_phase_num], _temperature_qp[_qp][_phase_num], _xnacl);
  _ddensity_qp_dt[_qp] = dDensity_dT(_porepressure_qp[_qp][_phase_num], _temperature_qp[_qp][_phase_num], _xnacl);

  /// Viscosity and derivative wrt temperature at the nodes
  _viscosity_nodal[_qp] = viscosity(_temperature_nodal[_qp][_phase_num], _density_nodal[_qp], _xnacl);
  _dviscosity_nodal_dt[_qp] = dViscosity_dT(_temperature_nodal[_qp][_phase_num], _density_nodal[_qp], _xnacl);
}

Real
PorousFlowBrine::density(Real pressure, Real temperature, Real xnacl) const
{
  /*
   * From Driesner, The system H2o-NaCl. Part II: Correlations for molar volume,
   * enthalpy, and isobaric heat capacity from 0 to 1000 C, 1 to 500 bar, and 0
   * to 1 Xnacl, Geochimica et Cosmochimica Acta 71, 4902-4919 (2007).
   */
  Real n1, n2, n11, n12, n1x1, n20, n21, n22, n23, n2x1, Tv;
  Real water_density, density;

  // The correlation requires the pressure in bar, not Pa.
  Real pbar = pressure * 1.0e-5;
  Real pbar2 = pbar * pbar;
  Real pbar3 = pbar2 * pbar;

  // The correlation requires mole fraction. First calculate the average molar mass
  // from the mass fraction, then compute the mole fraction
  Real Mbrine = 1.0 / (xnacl / _Mnacl + (1.0 - xnacl) / _Mh2o);
  Real Xnacl = xnacl / _Mnacl * Mbrine;

  n11 = -54.2958 - 45.7623 * std::exp(-9.44785e-4 * pbar);
  n21 = -2.6142 - 2.39092e-4 * pbar;
  n22 = 0.0356828 + 4.37235e-6 * pbar + 2.0566e-9 * pbar2;
  n1x1 = 330.47 + 0.942876 * std::sqrt(pbar) + 0.0817193 * pbar - 2.47556e-8 * pbar2
         + 3.45052e-10 * pbar3;
  n2x1 = -0.0370751 + 0.00237723 * std::sqrt(pbar) + 5.42049e-5 * pbar
         + 5.84709e-9 * pbar2 - 5.99373e-13 * pbar3;
  n12 = - n1x1 - n11;
  n20 = 1.0 - n21 * std::sqrt(n22);
  n23 = n2x1 - n20 -n21 * std::sqrt(1.0 + n22);

  // The temperature Tv (C) where the brine has the same molar volume as pure water
  n1 = n1x1 + n11 * (1.0 - Xnacl) + n12 * (1.0 - Xnacl) * (1.0 - Xnacl);
  n2 = n20 + n21 * std::sqrt(Xnacl + n22) + n23 * Xnacl;
  Tv = n1 + n2 * temperature;

  /// The brine density is then given by the density of water at temperature Tv.
  Real psat = pSat(temperature, xnacl);

  if (Tv >= 0. && Tv <= 350.)
  {
    if (pressure > psat && pressure <= 100.e6)
    {
      /// Region 1: single phase liquid
      water_density = PorousFlowWater::densityRegion1(pressure, Tv);
    }

    else if (pressure <= psat)
    {
      /// Region 2: vapour phase
      water_density = PorousFlowWater::densityRegion2(pressure, Tv);
    }

    else
      mooseError("Pressure "<< pressure << " is out of range in BrineProperties::density");
  }
  else
    mooseError("Temperature " << temperature << " is out of range in BrineProperties::density");

  density = water_density * Mbrine / _Mh2o;

  return density;
}

Real
PorousFlowBrine::dDensity_dP(Real pressure, Real temperature, Real /*xnacl*/) const
{
  //TODO: implement brine density derivative wrt P
  return PorousFlowWater::dDensity_dP(pressure, temperature);
}

Real
PorousFlowBrine::dDensity_dT(Real /*pressure*/, Real /*temperature*/, Real /*xnacl*/) const
{
  //TODO: implement brine density derivative wrt T
  return 0.;}

Real
PorousFlowBrine::viscosity(Real temperature, Real water_density, Real xnacl) const
{
  /*
   * Viscosity of brine.
   * From Phillips et al, A technical databook for geothermal energy utilization,
   * LbL-12810 (1981).
   */
  // Correlation requires molar concentration (mol/kg)
  Real mol = xnacl / ((1.0 - xnacl) * _Mnacl);
  Real mol2 = mol * mol;
  Real mol3 = mol2 * mol;

  Real a = 1.0 + 0.0816 * mol + 0.0122 * mol2 + 0.128e-3 * mol3 + 0.629e-3 * temperature
           * (1.0 - std::exp(-0.7 * mol));

  return a * PorousFlowWater::viscosity(temperature, water_density);
}

Real
PorousFlowBrine::dViscosity_dT(Real /*temperature*/, Real /*density*/, Real /*xnacl*/) const
{
  return 0.; // TODO: not implemented yet
}

Real
PorousFlowBrine::pSat(Real temperature, Real xnacl) const
{
  /*
   * Brine vapour pressure
   * From Haas, Physical properties of the coexisting phases and thermochemical
   * properties of the H20 component in boiling NaCl solutions, Geological Survey
   * Bulletin, 1421-A (1976).
   */
  // Temperature in K
  Real tk = temperature + _t_c2k;

  // Correlation requires molar concentration (mol/kg)
  Real mol = xnacl / ((1.0 - xnacl) * _Mnacl);
  Real mol2 = mol * mol;
  Real mol3 = mol2 * mol;

  Real a = 1.0 + 5.93582e-6 * mol - 5.19386e-5 * mol2 + 1.23156e-5 * mol3;
  Real b = 1.1542e-6 * mol + 1.41254e-7 * mol2 - 1.92476e-8 * mol3 - 1.70717e-9 * mol * mol3
           + 1.0539e-10 * mol2 * mol3;

  // The temperature of pure water at the same pressure as the brine is given by
  // Note that this correlation uses temperature in K, not C
  Real th20 = std::exp(std::log(tk) / (a + b * tk)) - _t_c2k;

  // The brine vapour pressure is then found by evaluating the saturation pressure for pure water
  // using this effective temperature
  return PorousFlowWater::pSat(th20);
}

Real
PorousFlowBrine::dViscosity_dDensity(Real temperature, Real water_density, Real xnacl) const
{
  // Correlation requires molar concentration (mol/kg)
  Real mol = xnacl / ((1.0 - xnacl) * _Mnacl);
  Real mol2 = mol * mol;
  Real mol3 = mol2 * mol;

  Real a = 1.0 + 0.0816 * mol + 0.0122 * mol2 + 0.128e-3 * mol3 + 0.629e-3 * temperature
         * (1.0 - std::exp(-0.7 * mol));

  // The brine viscosity is then given by a multiplied by the viscosity of pure water. Note
  // that the density is the density of pure water.
  return a * PorousFlowWater::dViscosity_dDensity(temperature, water_density);
}

Real
PorousFlowBrine::haliteDensity(Real pressure, Real temperature) const
{
  /*
   * Density of halite (solid NaCl)
   * From Driesner, The system H2o-NaCl. Part II: Correlations for molar volume,
   * enthalpy, and isobaric heat capacity from 0 to 1000 C, 1 to 500 bar, and 0
   * to 1 Xnacl, Geochimica et Cosmochimica Acta 71, 4902-4919 (2007).
   */
  // Correlation needs pressure in bar
  Real pbar = pressure * 1.e-5;

  // Halite density at 0 Pa
  Real density_P0 = 2.17043e3 - 2.4599e-1 * temperature - 9.5797e-5 * temperature * temperature;

  // Halite density as a function of pressure
  Real l = 5.727e-3 + 2.715e-3 * std::exp(temperature / 733.4);

  return density_P0 + l * pbar;
}

Real
PorousFlowBrine::haliteSolubility(Real temperature) const
{
  /*
   * Halite solubility
   * Originally from Potter et al., A new method for determining the solubility
   * of salts in aqueous solutions at elevated temperatures, J. Res. U.S. Geol.
   * Surv., 5, 389-395 (1977). Equation describing halite solubility is repeated
   * in Chou, Phase relations in the system NaCI-KCI-H20. III: Solubilities of
   * halite in vapor-saturated liquids above 445°C and redetermination of phase
   * equilibrium properties in the system NaCI-HzO to 1000°C and 1500 bars,
   * Geochimica et Cosmochimica Acta 51, 1965-1975 (1987).
   */
  Real solubility = (26.18 + 7.2e-3 * temperature + 1.06e-4 * temperature * temperature) / 100.0;

  return solubility;
}
