//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BrineFluidProperties.h"

registerMooseObject("FluidPropertiesApp", BrineFluidProperties);

template <>
InputParameters
validParams<BrineFluidProperties>()
{
  InputParameters params = validParams<MultiComponentFluidPropertiesPT>();
  params.addClassDescription("Fluid properties for brine");
  return params;
}

BrineFluidProperties::BrineFluidProperties(const InputParameters & parameters)
  : MultiComponentFluidPropertiesPT(parameters), _Mnacl(58.443e-3)
{
  // SinglePhaseFluidPropertiesPT UserObject for water to provide to getComponent
  std::string water_name = name() + ":water";
  {
    std::string class_name = "Water97FluidProperties";
    InputParameters params = _app.getFactory().getValidParams(class_name);
    _fe_problem.addUserObject(class_name, water_name, params);
  }
  _water_fp = &_fe_problem.getUserObject<SinglePhaseFluidPropertiesPT>(water_name);

  // SinglePhaseFluidPropertiesPT UserObject for NaCl to provide to getComponent
  std::string nacl_name = name() + ":nacl";
  {
    std::string class_name = "NaClFluidProperties";
    InputParameters params = _app.getFactory().getValidParams(class_name);
    _fe_problem.addUserObject(class_name, nacl_name, params);
  }
  _nacl_fp = &_fe_problem.getUserObject<SinglePhaseFluidPropertiesPT>(nacl_name);

  // Molar mass of NaCl and H20
  _Mnacl = _nacl_fp->molarMass();
  _Mh2o = _water_fp->molarMass();
}

BrineFluidProperties::~BrineFluidProperties() {}

const SinglePhaseFluidPropertiesPT &
BrineFluidProperties::getComponent(unsigned int component) const
{
  switch (component)
  {
    case WATER:
      return *_water_fp;

    case NACL:
      return *_nacl_fp;

    default:
      mooseError("BrineFluidProperties::getComponent has been provided an incorrect component");
  }
}

std::string
BrineFluidProperties::fluidName() const
{
  return "brine";
}

Real
BrineFluidProperties::molarMass(Real xnacl) const
{
  return 1.0 / (xnacl / _Mnacl + (1.0 - xnacl) / _Mh2o);
}

Real
BrineFluidProperties::molarMassNaCl() const
{
  return _Mnacl;
}

Real
BrineFluidProperties::molarMassH2O() const
{
  return _Mh2o;
}

Real
BrineFluidProperties::rho(Real pressure, Real temperature, Real xnacl) const
{
  Real n1, n2, n11, n12, n1x1, n20, n21, n22, n23, n2x1, Tv;
  Real water_density;

  // The correlation requires the pressure in bar, not Pa.
  Real pbar = pressure * 1.0e-5;
  Real pbar2 = pbar * pbar;
  Real pbar3 = pbar2 * pbar;

  // The correlation requires mole fraction
  Real Xnacl = massFractionToMoleFraction(xnacl);

  n11 = -54.2958 - 45.7623 * std::exp(-9.44785e-4 * pbar);
  n21 = -2.6142 - 2.39092e-4 * pbar;
  n22 = 0.0356828 + 4.37235e-6 * pbar + 2.0566e-9 * pbar2;
  n1x1 = 330.47 + 0.942876 * std::sqrt(pbar) + 0.0817193 * pbar - 2.47556e-8 * pbar2 +
         3.45052e-10 * pbar3;
  n2x1 = -0.0370751 + 0.00237723 * std::sqrt(pbar) + 5.42049e-5 * pbar + 5.84709e-9 * pbar2 -
         5.99373e-13 * pbar3;
  n12 = -n1x1 - n11;
  n20 = 1.0 - n21 * std::sqrt(n22);
  n23 = n2x1 - n20 - n21 * std::sqrt(1.0 + n22);

  // The temperature Tv where the brine has the same molar volume as pure water
  // Note: correlation uses temperature in Celcius
  n1 = n1x1 + n11 * (1.0 - Xnacl) + n12 * (1.0 - Xnacl) * (1.0 - Xnacl);
  n2 = n20 + n21 * std::sqrt(Xnacl + n22) + n23 * Xnacl;
  Tv = n1 + n2 * (temperature - _T_c2k);

  // The density of water at temperature Tv
  // Note: convert Tv to Kelvin to calculate water density
  water_density = _water_fp->rho_from_p_T(pressure, Tv + _T_c2k);

  // The brine density is given by the water density scaled by the ratio of
  // brine molar mass to pure water molar mass
  return water_density * molarMass(xnacl) / _Mh2o;
}

void
BrineFluidProperties::rho_dpTx(Real pressure,
                               Real temperature,
                               Real xnacl,
                               Real & rho,
                               Real & drho_dp,
                               Real & drho_dT,
                               Real & drho_dx) const
{
  rho = this->rho(pressure, temperature, xnacl);
  // Derivatives are calculated using finite differences due to complexity of correlation
  Real eps = 1.0e-8;
  Real peps = pressure * eps;
  Real Teps = temperature * eps;
  drho_dp = (this->rho(pressure + peps, temperature, xnacl) - rho) / peps;
  drho_dT = (this->rho(pressure, temperature + Teps, xnacl) - rho) / Teps;
  drho_dx = (this->rho(pressure, temperature, xnacl + eps) - rho) / eps;
}

Real
BrineFluidProperties::mu(Real pressure, Real temperature, Real xnacl) const
{
  // Correlation requires molal concentration (mol/kg)
  Real mol = massFractionToMolalConc(xnacl);
  Real mol2 = mol * mol;
  Real mol3 = mol2 * mol;

  // Correlation requires temperature in C
  Real Tc = temperature - _T_c2k;

  Real a = 1.0 + 0.0816 * mol + 0.0122 * mol2 + 0.128e-3 * mol3 +
           0.629e-3 * Tc * (1.0 - std::exp(-0.7 * mol));

  return a * _water_fp->mu_from_p_T(pressure, temperature);
}

void
BrineFluidProperties::mu_dpTx(Real pressure,
                              Real temperature,
                              Real xnacl,
                              Real & mu,
                              Real & dmu_dp,
                              Real & dmu_dT,
                              Real & dmu_dx) const
{
  // Viscosity of water and derivatives wrt pressure and temperature
  Real muw, dmuw_dp, dmuw_dT;
  _water_fp->mu_from_p_T(pressure, temperature, muw, dmuw_dp, dmuw_dT);

  // Correlation requires molal concentration (mol/kg)
  Real mol = massFractionToMolalConc(xnacl);
  Real dmol_dx = 1.0 / ((1.0 - xnacl) * (1.0 - xnacl) * _Mnacl);
  Real mol2 = mol * mol;
  Real mol3 = mol2 * mol;

  // Correlation requires temperature in C
  Real Tc = temperature - _T_c2k;

  Real a = 1.0 + 0.0816 * mol + 0.0122 * mol2 + 0.128e-3 * mol3 +
           0.629e-3 * Tc * (1.0 - std::exp(-0.7 * mol));
  Real da_dx =
      (0.0816 + 0.0244 * mol + 3.84e-4 * mol2 + 4.403e-4 * Tc * std::exp(-0.7 * mol)) * dmol_dx;
  Real da_dT = 0.629e-3 * (1.0 - std::exp(-0.7 * mol));

  mu = a * muw;
  dmu_dp = a * dmuw_dp;
  dmu_dx = da_dx * muw;
  dmu_dT = da_dT * muw + a * dmuw_dT;
}

void
BrineFluidProperties::rho_mu(
    Real pressure, Real temperature, Real xnacl, Real & rho, Real & mu) const
{
  rho = this->rho(pressure, temperature, xnacl);
  mu = this->mu(pressure, temperature, xnacl);
}

void
BrineFluidProperties::rho_mu_dpTx(Real pressure,
                                  Real temperature,
                                  Real xnacl,
                                  Real & rho,
                                  Real & drho_dp,
                                  Real & drho_dT,
                                  Real & drho_dx,
                                  Real & mu,
                                  Real & dmu_dp,
                                  Real & dmu_dT,
                                  Real & dmu_dx) const
{
  this->rho_dpTx(pressure, temperature, xnacl, rho, drho_dp, drho_dT, drho_dx);
  this->mu_dpTx(pressure, temperature, xnacl, mu, dmu_dp, dmu_dT, dmu_dx);
}

Real
BrineFluidProperties::h(Real pressure, Real temperature, Real xnacl) const
{
  Real q1, q2, q10, q11, q12, q20, q21, q22, q23, q1x1, q2x1, Th;

  // The correlation requires the pressure in bar, not Pa.
  Real pbar = pressure * 1.0e-5;
  Real pbar2 = pbar * pbar;

  // The correlation requires mole fraction
  Real Xnacl = massFractionToMoleFraction(xnacl);

  q11 = -32.1724 + 0.0621255 * pbar;
  q21 = -1.69513 - 4.52781e-4 * pbar - 6.04279e-8 * pbar2;
  q22 = 0.0612567 + 1.88082e-5 * pbar;

  q1x1 = 47.9048 - 9.36994e-3 * pbar + 6.51059e-6 * pbar2;
  q2x1 = 0.241022 + 3.45087e-5 * pbar - 4.28356e-9 * pbar2;

  q12 = -q11 - q1x1;
  q10 = q1x1;

  q20 = 1.0 - q21 * std::sqrt(q22);
  q23 = q2x1 - q20 - q21 * std::sqrt(1.0 + q22);

  q1 = q10 + q11 * (1.0 - Xnacl) + q12 * (1.0 - Xnacl) * (1.0 - Xnacl);
  q2 = q20 + q21 * std::sqrt(Xnacl + q22) + q23 * Xnacl;
  // The temperature Th where the brine has the same enthalpy as pure water
  // Note: correlation uses temperature in Celcius
  Th = q1 + q2 * (temperature - _T_c2k);

  // The brine enthalpy is then given by the enthalpy of water at temperature Th
  // Note: water enthalpy requires temperature in Kelvin
  return _water_fp->h_from_p_T(pressure, Th + _T_c2k);
}

void
BrineFluidProperties::h_dpTx(Real pressure,
                             Real temperature,
                             Real xnacl,
                             Real & h,
                             Real & dh_dp,
                             Real & dh_dT,
                             Real & dh_dx) const
{
  h = this->h(pressure, temperature, xnacl);
  // Derivatives are calculated using finite differences due to complexity of correlation
  Real eps = 1.0e-8;
  Real peps = pressure * eps;
  Real Teps = temperature * eps;
  dh_dp = (this->h(pressure + peps, temperature, xnacl) - h) / peps;
  dh_dT = (this->h(pressure, temperature + Teps, xnacl) - h) / Teps;
  dh_dx = (this->h(pressure, temperature, xnacl + eps) - h) / eps;
}

Real
BrineFluidProperties::cp(Real pressure, Real temperature, Real xnacl) const
{
  Real q1, q2, q10, q11, q12, q20, q21, q22, q23, q1x1, q2x1, Th;

  // The correlation requires the pressure in bar, not Pa.
  Real pbar = pressure * 1.0e-5;
  Real pbar2 = pbar * pbar;

  // The correlation requires mole fraction
  Real Xnacl = massFractionToMoleFraction(xnacl);

  q11 = -32.1724 + 0.0621255 * pbar;
  q21 = -1.69513 - 4.52781e-4 * pbar - 6.04279e-8 * pbar2;
  q22 = 0.0612567 + 1.88082e-5 * pbar;

  q1x1 = 47.9048 - 9.36994e-3 * pbar + 6.51059e-6 * pbar2;
  q2x1 = 0.241022 + 3.45087e-5 * pbar - 4.28356e-9 * pbar2;

  q12 = -q11 - q1x1;
  q10 = q1x1;

  q20 = 1.0 - q21 * std::sqrt(q22);
  q23 = q2x1 - q20 - q21 * std::sqrt(1.0 + q22);

  q1 = q10 + q11 * (1.0 - Xnacl) + q12 * (1.0 - Xnacl) * (1.0 - Xnacl);
  q2 = q20 + q21 * std::sqrt(Xnacl + q22) + q23 * Xnacl;
  // The temperature Th where the brine has the same isobaric heat capacity
  // as pure water. Note: correlation uses temperature in Celcius
  Th = q1 + q2 * (temperature - _T_c2k);

  // The brine isobaric heat capacity is then given by the isobaric heat
  // capacity of water at temperature Th multiplied by q2
  // Note: water isobaric heat capacity requires temperature in Kelvin
  return q2 * _water_fp->cp_from_p_T(pressure, Th + _T_c2k);
}

Real
BrineFluidProperties::e(Real pressure, Real temperature, Real xnacl) const
{
  Real enthalpy = h(pressure, temperature, xnacl);
  Real density = rho(pressure, temperature, xnacl);

  return enthalpy - pressure / density;
}

void
BrineFluidProperties::e_dpTx(Real pressure,
                             Real temperature,
                             Real xnacl,
                             Real & e,
                             Real & de_dp,
                             Real & de_dT,
                             Real & de_dx) const
{
  e = this->e(pressure, temperature, xnacl);
  // Derivatives are calculated using finite differences due to complexity of correlation
  Real eps = 1.0e-8;
  Real peps = pressure * eps;
  Real Teps = temperature * eps;
  de_dp = (this->e(pressure + peps, temperature, xnacl) - e) / peps;
  de_dT = (this->e(pressure, temperature + Teps, xnacl) - e) / Teps;
  de_dx = (this->e(pressure, temperature, xnacl + eps) - e) / eps;
}

Real
BrineFluidProperties::k(Real pressure, Real temperature, Real xnacl) const
{
  // Correlation requires molal concentration (mol/kg)
  Real mol = massFractionToMolalConc(xnacl);
  // Correlation requires temperature in C
  Real Tc = temperature - _T_c2k;

  Real S = 100.0 * _Mnacl * mol / (1.0 + _Mnacl * mol);
  Real lambdaw = _water_fp->k_from_p_T(pressure, temperature);
  Real lambda = 1.0 - (2.3434e-3 - 7.924e-6 * Tc + 3.924e-8 * Tc * Tc) * S +
                (1.06e-5 - 2.0e-8 * Tc - 1.2e-10 * Tc * Tc) * S * S;

  return lambda * lambdaw;
}

Real
BrineFluidProperties::vaporPressure(Real temperature, Real xnacl) const
{
  // Correlation requires molal concentration (mol/kg)
  Real mol = massFractionToMolalConc(xnacl);
  Real mol2 = mol * mol;
  Real mol3 = mol2 * mol;

  Real a = 1.0 + 5.93582e-6 * mol - 5.19386e-5 * mol2 + 1.23156e-5 * mol3;
  Real b = 1.1542e-6 * mol + 1.41254e-7 * mol2 - 1.92476e-8 * mol3 - 1.70717e-9 * mol * mol3 +
           1.0539e-10 * mol2 * mol3;

  // The temperature of pure water at the same pressure as the brine is given by
  Real th20 = std::exp(std::log(temperature) / (a + b * temperature));

  // The brine vapour pressure is then found by evaluating the saturation pressure for pure water
  // using this effective temperature
  return _water_fp->vaporPressure(th20);
}

Real
BrineFluidProperties::haliteSolubility(Real temperature) const
{
  // This correlation requires temperature in Celcius
  Real Tc = temperature - _T_c2k;

  return (26.18 + 7.2e-3 * Tc + 1.06e-4 * Tc * Tc) / 100.0;
}

Real
BrineFluidProperties::massFractionToMolalConc(Real xnacl) const
{
  return xnacl / ((1.0 - xnacl) * _Mnacl);
}

Real
BrineFluidProperties::massFractionToMoleFraction(Real xnacl) const
{
  // The average molar mass of brine from the mass fraction
  Real Mbrine = molarMass(xnacl);
  // The mole fraction is then
  return xnacl * Mbrine / _Mnacl;
}
