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

InputParameters
BrineFluidProperties::validParams()
{
  InputParameters params = MultiComponentFluidProperties::validParams();
  params.addParam<UserObjectName>("water_fp",
                                  "The name of the FluidProperties UserObject for water");
  params.addClassDescription("Fluid properties for brine");
  return params;
}

BrineFluidProperties::BrineFluidProperties(const InputParameters & parameters)
  : MultiComponentFluidProperties(parameters), _water_fp_derivs(true)
{
  // There are two possibilities to consider:
  // 1) No water_fp has been supplied (in which case one is constructed)
  // 2) A water_fp hase been supplied (in which case it is used)
  // In both cases, though, a Water97FluidProperties UserObject must be added
  // Note: this UserObject is only used to gain access to the Henry's constant
  // formulation. All property calculations are performed using _water_fp
  const std::string water_name = name() + ":water";
  {
    const std::string class_name = "Water97FluidProperties";
    InputParameters params = _app.getFactory().getValidParams(class_name);
    if (_tid == 0)
      _fe_problem.addUserObject(class_name, water_name, params);
  }
  _water97_fp = &_fe_problem.getUserObject<Water97FluidProperties>(water_name);

  if (parameters.isParamSetByUser("water_fp"))
  {
    // SinglePhaseFluidPropertiesPT UserObject for water
    _water_fp = &getUserObject<SinglePhaseFluidProperties>("water_fp");

    // Check that a water userobject has actually been supplied
    if (_water_fp->fluidName() != "water")
      paramError("water_fp", "A water FluidProperties UserObject must be supplied");
  }
  else
  {
    // Construct a SinglePhaseFluidProperties UserObject for water
    _water_fp = &_fe_problem.getUserObject<SinglePhaseFluidProperties>(water_name);
  }

  // SinglePhaseFluidProperties UserObject for NaCl
  const std::string nacl_name = name() + ":nacl";
  {
    const std::string class_name = "NaClFluidProperties";
    InputParameters params = _app.getFactory().getValidParams(class_name);
    if (_tid == 0)
      _fe_problem.addUserObject(class_name, nacl_name, params);
  }
  _nacl_fp = &_fe_problem.getUserObject<SinglePhaseFluidProperties>(nacl_name);

  // Molar mass of NaCl and H20
  _Mnacl = _nacl_fp->molarMass();
  _Mh2o = _water_fp->molarMass();
}

BrineFluidProperties::~BrineFluidProperties() {}

const SinglePhaseFluidProperties &
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

FPDualReal
BrineFluidProperties::molarMass(const FPDualReal & xnacl) const
{
  return 1.0 / (xnacl / _Mnacl + (1.0 - xnacl) / _Mh2o);
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

FPDualReal
BrineFluidProperties::rho_from_p_T_X(const FPDualReal & pressure,
                                     const FPDualReal & temperature,
                                     const FPDualReal & xnacl) const
{
  // The correlation requires the pressure in bar, not Pa.
  FPDualReal pbar = pressure * 1.0e-5;
  FPDualReal pbar2 = pbar * pbar;
  FPDualReal pbar3 = pbar2 * pbar;

  // The correlation requires mole fraction
  const FPDualReal Xnacl = massFractionToMoleFraction(xnacl);

  const FPDualReal n11 = -54.2958 - 45.7623 * std::exp(-9.44785e-4 * pbar);
  const FPDualReal n21 = -2.6142 - 2.39092e-4 * pbar;
  const FPDualReal n22 = 0.0356828 + 4.37235e-6 * pbar + 2.0566e-9 * pbar2;
  const FPDualReal n1x1 = 330.47 + 0.942876 * std::sqrt(pbar) + 0.0817193 * pbar -
                          2.47556e-8 * pbar2 + 3.45052e-10 * pbar3;
  const FPDualReal n2x1 = -0.0370751 + 0.00237723 * std::sqrt(pbar) + 5.42049e-5 * pbar +
                          5.84709e-9 * pbar2 - 5.99373e-13 * pbar3;
  const FPDualReal n12 = -n1x1 - n11;
  const FPDualReal n20 = 1.0 - n21 * std::sqrt(n22);
  const FPDualReal n23 = n2x1 - n20 - n21 * std::sqrt(1.0 + n22);

  // The temperature Tv where the brine has the same molar volume as pure water
  // Note: correlation uses temperature in Celcius
  const FPDualReal n1 = n1x1 + n11 * (1.0 - Xnacl) + n12 * (1.0 - Xnacl) * (1.0 - Xnacl);
  const FPDualReal n2 = n20 + n21 * std::sqrt(Xnacl + n22) + n23 * Xnacl;
  const FPDualReal Tv = n1 + n2 * (temperature - _T_c2k);

  // The density of water at temperature Tv
  // Note: convert Tv to Kelvin to calculate water density
  FPDualReal water_density;
  if (_water_fp_derivs)
  {
    Real rho, drho_dp, drho_dT;
    _water_fp->rho_from_p_T(pressure.value(), Tv.value() + _T_c2k, rho, drho_dp, drho_dT);
    water_density = rho;

    water_density.derivatives() = pressure.derivatives() * drho_dp + Tv.derivatives() * drho_dT;
  }
  else
    water_density = _water_fp->rho_from_p_T(pressure.value(), Tv.value() + _T_c2k);

  // The brine density is given by the water density scaled by the ratio of
  // brine molar mass to pure water molar mass
  return water_density * molarMass(xnacl) / _Mh2o;
}

Real
BrineFluidProperties::rho_from_p_T_X(Real pressure, Real temperature, Real xnacl) const
{
  // Initialise the AD value (no derivatives required)
  FPDualReal p = pressure;
  FPDualReal T = temperature;
  FPDualReal x = xnacl;

  _water_fp_derivs = false;
  FPDualReal ad_rho = this->rho_from_p_T_X(p, T, x);

  return ad_rho.value();
}

void
BrineFluidProperties::rho_from_p_T_X(Real pressure,
                                     Real temperature,
                                     Real xnacl,
                                     Real & rho,
                                     Real & drho_dp,
                                     Real & drho_dT,
                                     Real & drho_dx) const
{
  // Initialise the AD value and derivatives
  FPDualReal p = pressure;
  Moose::derivInsert(p.derivatives(), 0, 1.0);
  FPDualReal T = temperature;
  Moose::derivInsert(T.derivatives(), 1, 1.0);
  FPDualReal x = xnacl;
  Moose::derivInsert(x.derivatives(), 2, 1.0);

  _water_fp_derivs = true;
  FPDualReal ad_rho = this->rho_from_p_T_X(p, T, x);

  rho = ad_rho.value();
  drho_dp = ad_rho.derivatives()[0];
  drho_dT = ad_rho.derivatives()[1];
  drho_dx = ad_rho.derivatives()[2];
}

Real
BrineFluidProperties::mu_from_p_T_X(Real pressure, Real temperature, Real xnacl) const
{
  // Correlation requires molal concentration (mol/kg)
  const Real mol = massFractionToMolalConc(xnacl);
  const Real mol2 = mol * mol;
  const Real mol3 = mol2 * mol;

  // Correlation requires temperature in C
  const Real Tc = temperature - _T_c2k;

  const Real a = 1.0 + 0.0816 * mol + 0.0122 * mol2 + 0.128e-3 * mol3 +
                 0.629e-3 * Tc * (1.0 - std::exp(-0.7 * mol));

  const Real water_viscosity = _water_fp->mu_from_p_T(pressure, temperature);

  return a * water_viscosity;
}

void
BrineFluidProperties::mu_from_p_T_X(Real pressure,
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

FPDualReal
BrineFluidProperties::h_from_p_T_X(const FPDualReal & pressure,
                                   const FPDualReal & temperature,
                                   const FPDualReal & xnacl) const
{
  FPDualReal q1, q2, q10, q11, q12, q20, q21, q22, q23, q1x1, q2x1, Th;

  // The correlation requires the pressure in bar, not Pa.
  const FPDualReal pbar = pressure * 1.0e-5;
  const FPDualReal pbar2 = pbar * pbar;

  // The correlation requires mole fraction
  const FPDualReal Xnacl = massFractionToMoleFraction(xnacl);

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
  FPDualReal enthalpy;
  if (_water_fp_derivs)
  {
    Real h, dh_dp, dh_dT;
    _water_fp->h_from_p_T(pressure.value(), Th.value() + _T_c2k, h, dh_dp, dh_dT);
    enthalpy = h;

    enthalpy.derivatives() = pressure.derivatives() * dh_dp + Th.derivatives() * dh_dT;
  }
  else
    enthalpy = _water_fp->h_from_p_T(pressure.value(), Th.value() + _T_c2k);

  return enthalpy;
}

Real
BrineFluidProperties::h_from_p_T_X(Real pressure, Real temperature, Real xnacl) const
{
  // Initialise the AD value (no derivatives required)
  FPDualReal p = pressure;
  FPDualReal T = temperature;
  FPDualReal x = xnacl;

  _water_fp_derivs = false;
  return h_from_p_T_X(p, T, x).value();
}

void
BrineFluidProperties::h_from_p_T_X(Real pressure,
                                   Real temperature,
                                   Real xnacl,
                                   Real & h,
                                   Real & dh_dp,
                                   Real & dh_dT,
                                   Real & dh_dx) const
{
  // Initialise the AD value and derivatives
  FPDualReal p = pressure;
  Moose::derivInsert(p.derivatives(), 0, 1.0);
  FPDualReal T = temperature;
  Moose::derivInsert(T.derivatives(), 1, 1.0);
  FPDualReal x = xnacl;
  Moose::derivInsert(x.derivatives(), 2, 1.0);

  _water_fp_derivs = true;
  FPDualReal ad_h = h_from_p_T_X(p, T, x);

  h = ad_h.value();
  dh_dp = ad_h.derivatives()[0];
  dh_dT = ad_h.derivatives()[1];
  dh_dx = ad_h.derivatives()[2];
}

Real
BrineFluidProperties::cp_from_p_T_X(Real pressure, Real temperature, Real xnacl) const
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

FPDualReal
BrineFluidProperties::e_from_p_T_X(const FPDualReal & pressure,
                                   const FPDualReal & temperature,
                                   const FPDualReal & xnacl) const
{
  FPDualReal enthalpy = h_from_p_T_X(pressure, temperature, xnacl);
  FPDualReal density = rho_from_p_T_X(pressure, temperature, xnacl);

  return enthalpy - pressure / density;
}

Real
BrineFluidProperties::e_from_p_T_X(Real pressure, Real temperature, Real xnacl) const
{
  Real enthalpy = h_from_p_T_X(pressure, temperature, xnacl);
  Real density = rho_from_p_T_X(pressure, temperature, xnacl);

  return enthalpy - pressure / density;
}

void
BrineFluidProperties::e_from_p_T_X(Real pressure,
                                   Real temperature,
                                   Real xnacl,
                                   Real & e,
                                   Real & de_dp,
                                   Real & de_dT,
                                   Real & de_dx) const
{
  // Initialise the AD value and derivatives
  FPDualReal p = pressure;
  Moose::derivInsert(p.derivatives(), 0, 1.0);
  FPDualReal T = temperature;
  Moose::derivInsert(T.derivatives(), 1, 1.0);
  FPDualReal x = xnacl;
  Moose::derivInsert(x.derivatives(), 2, 1.0);

  _water_fp_derivs = true;
  FPDualReal ad_e = e_from_p_T_X(p, T, x);

  e = ad_e.value();
  de_dp = ad_e.derivatives()[0];
  de_dT = ad_e.derivatives()[1];
  de_dx = ad_e.derivatives()[2];
}

Real
BrineFluidProperties::k_from_p_T_X(Real pressure, Real temperature, Real xnacl) const
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

FPDualReal
BrineFluidProperties::massFractionToMoleFraction(const FPDualReal & xnacl) const
{
  // The average molar mass of brine from the mass fraction
  FPDualReal Mbrine = molarMass(xnacl);
  // The mole fraction is then
  return xnacl * Mbrine / _Mnacl;
}

Real
BrineFluidProperties::henryConstant(Real temperature, const std::vector<Real> & coeffs) const
{
  return _water97_fp->henryConstant(temperature, coeffs);
}

void
BrineFluidProperties::henryConstant(Real temperature,
                                    const std::vector<Real> & coeffs,
                                    Real & Kh,
                                    Real & dKh_dT) const
{
  _water97_fp->henryConstant(temperature, coeffs, Kh, dKh_dT);
}

DualReal
BrineFluidProperties::henryConstant(const DualReal & temperature,
                                    const std::vector<Real> & coeffs) const
{
  Real Kh, dKh_dT;
  henryConstant(temperature.value(), coeffs, Kh, dKh_dT);

  DualReal henry = Kh;
  henry.derivatives() = temperature.derivatives() * dKh_dT;

  return henry;
}
