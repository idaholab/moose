#include "LeadLithiumFluidProperties.h"

registerMooseObject("FluidPropertiesApp", LeadLithiumFluidProperties);

InputParameters
LeadLithiumFluidProperties::validParams()
{
  InputParameters params = SinglePhaseFluidProperties::validParams();
  params.addClassDescription("Fluid properties for Lead Lithium eutectic (83% Pb, 17% Li)");
  return params;
}

LeadLithiumFluidProperties::LeadLithiumFluidProperties(const InputParameters & parameters)
  : SinglePhaseFluidProperties(parameters)
{
}

std::string
LeadLithiumFluidProperties::fluidName() const
{
  return "LeadLithium";
}

Real
LeadLithiumFluidProperties::molarMass() const
{
  // Approximate molar mass for 83% Pb (207.2 g/mol) and 17% Li (6.94 g/mol) by atomic fraction
  return 1.73e-1; // in kg/mol
}

//------------------------------------------------------
// Density & Volume (based on a linear fit)
// ρ(T) = 10520.35 - 1.19051*T    [kg/m³]
//------------------------------------------------------
Real
LeadLithiumFluidProperties::rho_from_p_T(Real /*p*/, Real T) const
{
  return 10520.35 - 1.19051 * T;
}

void
LeadLithiumFluidProperties::rho_from_p_T(
    Real p, Real T, Real & rho, Real & drho_dp, Real & drho_dT) const
{
  rho = rho_from_p_T(p, T);
  drho_dp = 0;
  drho_dT = -1.19051;
}

void
LeadLithiumFluidProperties::rho_from_p_T(
    const ADReal & p, const ADReal & T, ADReal & rho, ADReal & drho_dp, ADReal & drho_dT) const
{
  rho = SinglePhaseFluidProperties::rho_from_p_T(p, T);
  drho_dp = 0;
  drho_dT = -1.19051;
}

Real
LeadLithiumFluidProperties::v_from_p_T(Real p, Real T) const
{
  return 1.0 / rho_from_p_T(p, T);
}

void
LeadLithiumFluidProperties::v_from_p_T(Real p, Real T, Real & v, Real & dv_dp, Real & dv_dT) const
{
  v = v_from_p_T(p, T);
  dv_dp = 0;
  dv_dT = 1.19051 / std::pow(10520.35 - 1.19051 * T, 2);
}

//------------------------------------------------------
// Temperature from Specific Volume and Energy
// We invert ρ = 10520.35 - 1.19051*T  (with v = 1/ρ)
//   => T = (10520.35 - 1/v)/1.19051
//------------------------------------------------------
Real
LeadLithiumFluidProperties::T_from_v_e(Real v, Real /*e*/) const
{
  return (10520.35 - 1.0 / v) / 1.19051;
}

void
LeadLithiumFluidProperties::T_from_v_e(Real v, Real e, Real & T, Real & dT_dv, Real & dT_de) const
{
  T = T_from_v_e(v, e);
  dT_de = 0;
  dT_dv = 1.0 / (1.19051 * v * v);
}

//------------------------------------------------------
// Bulk modulus (isentropic) [Pa]
// A quadratic fit was derived to give ~33 GPa at 500K, ~31 GPa at 600K,
// and ~16 GPa by 1800K.
//------------------------------------------------------
Real
LeadLithiumFluidProperties::bulk_modulus_from_p_T(Real /*p*/, Real T) const
{
  return (44.73077 - 0.02634615 * T + 5.76923e-6 * T * T) * 1e9;
}

//------------------------------------------------------
// Speed of sound (c)
// Using a linear fit from ultrasonic measurements:
//   c(T) ≈ 1959.63 - 0.306 * T   [m/s]  (with T in K)
//------------------------------------------------------
Real
LeadLithiumFluidProperties::c_from_v_e(Real v, Real e) const
{
  Real T = T_from_v_e(v, e);
  return 1959.63 - 0.306 * T;
}

ADReal
LeadLithiumFluidProperties::c_from_v_e(const ADReal & v, const ADReal & e) const
{
  ADReal T = SinglePhaseFluidProperties::T_from_v_e(v, e);
  return 1959.63 - 0.306 * T;
}

//------------------------------------------------------
// Pressure from specific volume and energy:
//   p = (h - e) / v
//------------------------------------------------------
Real
LeadLithiumFluidProperties::p_from_v_e(Real v, Real e) const
{
  Real h = h_from_v_e(v, e);
  return (h - e) / v;
}

void
LeadLithiumFluidProperties::p_from_v_e(Real v, Real e, Real & p, Real & dp_dv, Real & dp_de) const
{
  p = p_from_v_e(v, e);
  Real h, dh_dv, dh_de;
  h_from_v_e(v, e, h, dh_dv, dh_de);
  dp_dv = (v * dh_dv - h + e) / (v * v);
  dp_de = (dh_de - 1) / v;
}

//------------------------------------------------------
// Specific heat at constant pressure (cp)
// Using a linear correlation: cp(T) = 195 - 9.116e-3 * T  [J/(kg·K)]
//------------------------------------------------------
Real
LeadLithiumFluidProperties::cp_from_v_e(Real v, Real e) const
{
  Real T = T_from_v_e(v, e);
  return 195.0 - 9.116e-3 * T;
}

void
LeadLithiumFluidProperties::cp_from_v_e(
    Real v, Real e, Real & cp, Real & dcp_dv, Real & dcp_de) const
{
  Real T, dT_dv, dT_de;
  T_from_v_e(v, e, T, dT_dv, dT_de);
  cp = cp_from_v_e(v, e);
  const Real dcp_dT = -9.116e-3;
  dcp_dv = dcp_dT * dT_dv;
  dcp_de = dcp_dT * dT_de;
}

//------------------------------------------------------
// Specific heat at constant volume (cv)
// Calculated via the thermodynamic relation:
//   cv = cp / (1 + alpha² * bulk_modulus * T/(rho * cp))
// where the thermal expansion coefficient is α = 1.19051/(10520.35 - 1.19051*T).
//------------------------------------------------------
Real
LeadLithiumFluidProperties::cv_from_p_T(Real p, Real T) const
{
  Real rho, drho_dT, drho_dp;
  rho_from_p_T(p, T, rho, drho_dp, drho_dT);
  Real alpha = 1.19051 / (10520.35 - 1.19051 * T);
  Real bulk_modulus = bulk_modulus_from_p_T(p, T);
  Real cp = cp_from_p_T(p, T);
  return cp / (1.0 + alpha * alpha * bulk_modulus * T / (rho * cp));
}

void
LeadLithiumFluidProperties::cv_from_p_T(
    Real p, Real T, Real & cv, Real & dcv_dp, Real & dcv_dT) const
{
  // A full analytical derivative is complex; here we assume minimal pressure dependence.
  Real cp, dcp_dp, dcp_dT;
  cp_from_p_T(p, T, cp, dcp_dp, dcp_dT);
  cv = cv_from_p_T(p, T);
  dcv_dp = 0;
  const Real dT = 1e-3;
  Real cv_plus = cv_from_p_T(p, T + dT);
  dcv_dT = (cv_plus - cv) / dT;
}

Real
LeadLithiumFluidProperties::cv_from_v_e(Real v, Real e) const
{
  Real p = p_from_v_e(v, e);
  Real T = T_from_v_e(v, e);
  return cv_from_p_T(p, T);
}

void
LeadLithiumFluidProperties::cv_from_v_e(
    Real v, Real e, Real & cv, Real & dcv_dv, Real & dcv_de) const
{
  Real p, dp_dv, dp_de;
  p_from_v_e(v, e, p, dp_dv, dp_de);
  Real T, dT_dv, dT_de;
  T_from_v_e(v, e, T, dT_dv, dT_de);
  Real dcv_dp, dcv_dT;
  cv_from_p_T(p, T, cv, dcv_dp, dcv_dT);
  dcv_dv = dcv_dp * dp_dv + dcv_dT * dT_dv;
  dcv_de = dcv_dp * dp_de + dcv_dT * dT_de;
}

//------------------------------------------------------
// Dynamic viscosity (mu)
// Using an Arrhenius-type correlation:
//   mu(T) = 1.87e-4 * exp(11640/(8.314*T))   [Pa·s]
//------------------------------------------------------
Real
LeadLithiumFluidProperties::mu_from_v_e(Real v, Real e) const
{
  Real T = T_from_v_e(v, e);
  return 1.87e-4 * std::exp(11640.0 / (8.314 * T));
}

void
LeadLithiumFluidProperties::mu_from_v_e(
    Real v, Real e, Real & mu, Real & dmu_dv, Real & dmu_de) const
{
  Real T, dT_dv, dT_de;
  T_from_v_e(v, e, T, dT_dv, dT_de);
  mu = mu_from_v_e(v, e);
  Real factor = -11640.0 / (8.314 * T * T);
  dmu_dv = factor * mu * dT_dv;
  dmu_de = factor * mu * dT_de;
}

//------------------------------------------------------
// Thermal conductivity (k)
// Using a linear fit: k(T) = 9.144 + 0.019631 * T   [W/(m·K)]
// (Derived by converting literature data from °C to K.)
//------------------------------------------------------
Real
LeadLithiumFluidProperties::k_from_v_e(Real v, Real e) const
{
  Real T = T_from_v_e(v, e);
  return 9.144 + 0.019631 * T;
}

void
LeadLithiumFluidProperties::k_from_v_e(Real v, Real e, Real & k, Real & dk_dv, Real & dk_de) const
{
  Real T, dT_dv, dT_de;
  T_from_v_e(v, e, T, dT_dv, dT_de);
  k = k_from_v_e(v, e);
  dk_dv = 0.019631 * dT_dv;
  dk_de = 0.019631 * dT_de;
}

//------------------------------------------------------
// Enthalpy (h)
// We integrate cp from the melting point (_T_mo) to T.
// Here, with cp(T) = 195 - 9.116e-3 * T, the integral gives:
//   h = 195*(T - _T_mo) - 0.5*9.116e-3*(T² - _T_mo²)
// (Pressure effects are negligible.)
//------------------------------------------------------
Real
LeadLithiumFluidProperties::h_from_p_T(Real /*p*/, Real T) const
{
  return 195.0 * (T - _T_mo) - 0.5 * 9.116e-3 * (T * T - _T_mo * _T_mo);
}

void
LeadLithiumFluidProperties::h_from_p_T(Real p, Real T, Real & h, Real & dh_dp, Real & dh_dT) const
{
  h = h_from_p_T(p, T);
  dh_dp = 0;
  dh_dT = cp_from_p_T(p, T);
}

Real
LeadLithiumFluidProperties::h_from_v_e(Real v, Real e) const
{
  Real T = T_from_v_e(v, e);
  return h_from_p_T(0, T);
}

void
LeadLithiumFluidProperties::h_from_v_e(Real v, Real e, Real & h, Real & dh_dv, Real & dh_de) const
{
  Real T, dT_dv, dT_de;
  T_from_v_e(v, e, T, dT_dv, dT_de);
  h = h_from_v_e(v, e);
  Real cp = cp_from_v_e(v, e);
  dh_dv = cp * dT_dv;
  dh_de = cp * dT_de;
}

//------------------------------------------------------
// Internal energy (e)
// Using the definition: h = e + p*v  =>  e = h - p*v
//------------------------------------------------------
Real
LeadLithiumFluidProperties::e_from_p_T(Real p, Real T) const
{
  Real v = v_from_p_T(p, T);
  Real h = h_from_p_T(p, T);
  return h - p * v;
}

void
LeadLithiumFluidProperties::e_from_p_T(Real p, Real T, Real & e, Real & de_dp, Real & de_dT) const
{
  Real dh_dp, dv_dp, dh_dT, dv_dT, v, h;
  h_from_p_T(p, T, h, dh_dp, dh_dT);
  v_from_p_T(p, T, v, dv_dp, dv_dT);
  e = e_from_p_T(p, T);
  de_dp = dh_dp - v - dv_dp * p;
  de_dT = dh_dT - dv_dT * p;
}

Real
LeadLithiumFluidProperties::e_from_p_rho(Real p, Real rho) const
{
  return e_from_p_T(p, T_from_p_rho(p, rho));
}

void
LeadLithiumFluidProperties::e_from_p_rho(
    Real p, Real rho, Real & e, Real & de_dp, Real & de_drho) const
{
  Real T, dT_dp, dT_drho;
  T_from_p_rho(p, rho, T, dT_dp, dT_drho);
  Real de_dp_T, de_dT;
  e_from_p_T(p, T, e, de_dp_T, de_dT);
  de_dp = de_dp_T + de_dT * dT_dp;
  de_drho = de_dT * dT_drho;
}

//------------------------------------------------------
// Temperature from pressure and density
// Inverting ρ = 10520.35 - 1.19051*T  =>  T = (10520.35 - ρ)/1.19051
//------------------------------------------------------
Real
LeadLithiumFluidProperties::T_from_p_rho(Real /*p*/, Real rho) const
{
  return (10520.35 - rho) / 1.19051;
}

void
LeadLithiumFluidProperties::T_from_p_rho(
    Real p, Real rho, Real & T, Real & dT_dp, Real & dT_drho) const
{
  T = T_from_p_rho(p, rho);
  dT_dp = 0;
  dT_drho = -1.0 / 1.19051;
}

//------------------------------------------------------
// Temperature from pressure and enthalpy
// (Uses an iterative Newton solver; see FluidPropertiesUtils.)
//------------------------------------------------------
Real
LeadLithiumFluidProperties::T_from_p_h(Real p, Real h) const
{
  auto lambda = [&](Real p, Real current_T, Real & new_h, Real & dh_dp, Real & dh_dT)
  { h_from_p_T(p, current_T, new_h, dh_dp, dh_dT); };
  Real T = FluidPropertiesUtils::NewtonSolve(
               p, h, _T_initial_guess, _tolerance, lambda, name() + "::T_from_p_h")
               .first;
  if (std::isnan(T))
    mooseError("Conversion from pressure and enthalpy to temperature failed to converge.");
  return T;
}

void
LeadLithiumFluidProperties::T_from_p_h(Real p, Real h, Real & T, Real & dT_dp, Real & dT_dh) const
{
  T = T_from_p_h(p, h);
  dT_dp = 0;
  Real h1, dh_dp, dh_dT;
  h_from_p_T(p, T, h1, dh_dp, dh_dT);
  dT_dh = 1.0 / dh_dT;
}

//------------------------------------------------------
// cp, mu, and k from pressure and temperature (wrappers)
//------------------------------------------------------
Real
LeadLithiumFluidProperties::cp_from_p_T(Real /*p*/, Real T) const
{
  return 195.0 - 9.116e-3 * T;
}

void
LeadLithiumFluidProperties::cp_from_p_T(
    Real p, Real T, Real & cp, Real & dcp_dp, Real & dcp_dT) const
{
  cp = cp_from_p_T(p, T);
  dcp_dp = 0;
  dcp_dT = -9.116e-3;
}

Real
LeadLithiumFluidProperties::mu_from_p_T(Real /*p*/, Real T) const
{
  return 1.87e-4 * std::exp(11640.0 / (8.314 * T));
}

void
LeadLithiumFluidProperties::mu_from_p_T(
    Real p, Real T, Real & mu, Real & dmu_dp, Real & dmu_dT) const
{
  mu = mu_from_p_T(p, T);
  dmu_dp = 0;
  dmu_dT = -11640.0 / (8.314 * T * T) * mu;
}

Real
LeadLithiumFluidProperties::k_from_p_T(Real /*p*/, Real T) const
{
  return 9.144 + 0.019631 * T;
}

void
LeadLithiumFluidProperties::k_from_p_T(Real p, Real T, Real & k, Real & dk_dp, Real & dk_dT) const
{
  k = k_from_p_T(p, T);
  dk_dp = 0;
  dk_dT = 0.019631;
}
