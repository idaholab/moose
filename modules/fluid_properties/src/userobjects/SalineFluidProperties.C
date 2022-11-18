//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
//
#include "SalineFluidProperties.h"

registerMooseObject("FluidPropertiesApp", SalineFluidProperties);

InputParameters
SalineFluidProperties::validParams()
{
  InputParameters params = SinglePhaseFluidProperties::validParams();
  params.addRequiredParam<std::string>(
      "comp_name", "The name of the components in the salt (e.g., 'LiF-NaF-KF')");
  params.addRequiredParam<std::string>(
      "comp_val", "The mole fraction of each salt component (e.g., '0.5-0.1-0.4)");
  params.addRequiredParam<std::string>(
      "prop_def",
      "Definition of a fluid property file, which must be a file path to the "
      "comma-separated data matching the Saline format.");
  return params;
}

//// Input parser for salt component names
// std::vector<std::string>
// splitString_str(std::string str)
//{
//   std::string delimiter = "-";
//   std::string tmpstr = str;
//
//   std::vector<std::string> list;
//   size_t pos = 0;
//   std::string token;
//   while ((pos = tmpstr.find(delimiter)) != std::string::npos)
//   {
//     token = tmpstr.substr(0, pos);
//     list.push_back(token);
//     tmpstr.erase(0, pos + delimiter.length());
//   }
//   list.push_back(tmpstr);
//   return list;
// }
//
//// Input parser for salt component mole fractions
// std::vector<Real>
// splitString_float(std::string str)
//{
//   std::vector<std::string> list = splitString_str(str);
//   std::vector<Real> res;
//   for (std::string s : list)
//     res.push_back(stof(s));
//   return res;
// }

SalineFluidProperties::SalineFluidProperties(const InputParameters & parameters)
  : SinglePhaseFluidProperties(parameters)
{

#ifdef SALINE_ENABLED

  std::string propDef = getParam<std::string>("prop_def");
  _d.load(propDef);
  bool success = _tp.initialize(&_d);
  if (!success)
    mooseError("The initialize of the Saline interface has failed");
  std::string name = getParam<std::string>("comp_name");
  std::string comp = getParam<std::string>("comp_val");
  std::vector<std::string> nameList;
  MooseUtils::tokenize<std::string>(name, nameList, 1, "-");
  std::vector<Real> valList;
  MooseUtils::tokenizeAndConvert<Real>(comp, valList, "-");
  Real mole_sum = 0.0;
  for (Real val : valList)
    mole_sum = mole_sum + val;
  if (std::abs(mole_sum - 1.0) > 1e-6)
    mooseError("Mole fractions of defined salt compound do not sum to 1.0.");
  success = _tp.setComposition(nameList, valList);
  if (!success)
    mooseError("The composition set has failed");
  _fluid_name = name;

#endif
#ifndef SALINE_ENABLED

  mooseError("Saline was not made available during the build and can not be used.");

#endif
}

// Unit conversion constants for communicating with Saline
const Real kPa_to_Pa = 1.0e3;
const Real Pa_to_kPa = 1.0 / kPa_to_Pa;
const Real kg_to_g = 1.0e3;
const Real g_to_kg = 1.0 / kg_to_g;
const Real m_to_cm = 1.0e2;
const Real cm_to_m = 1.0 / m_to_cm;
const Real N_to_mN = 1.0e3;
const Real mN_to_N = 1 / N_to_mN;

#ifdef SALINE_ENABLED

std::string
SalineFluidProperties::fluidName() const
{
  return _fluid_name;
}

Real
SalineFluidProperties::rho_from_p_T(Real pressure, Real temperature) const
{
  return _tp.rho_kgm3(temperature, pressure * Pa_to_kPa);
}

void
SalineFluidProperties::rho_from_p_T(
    Real pressure, Real temperature, Real & rho, Real & drho_dp, Real & drho_dT) const
{
  rho = rho_from_p_T(pressure, temperature);
  // Estimate derivatives for now because they are not provided by Saline
  drho_dp = 0.;
  drho_dT = (rho_from_p_T(pressure, temperature + 1.0) - rho) / 1.0;
}

Real
SalineFluidProperties::cp_from_p_T(Real pressure, Real temperature) const
{
  return _tp.cp_kg(temperature, pressure * Pa_to_kPa); // J/kg/K
}

void
SalineFluidProperties::cp_from_p_T(
    Real pressure, Real temperature, Real & cp, Real & dcp_dp, Real & dcp_dT) const
{
  cp = cp_from_p_T(pressure, temperature);
  dcp_dp = 0;
  dcp_dT = (cp_from_p_T(pressure, temperature + 1.0) - cp) / 1.0;
}

// Real
// SalineFluidProperties::enthalpy_from_T(Real temperature) const
//{
//   return _tp.h_t_kg(temperature);
// }
//
// Real
// SalineFluidProperties::T_from_enthalpy(Real enthalpy) const
//{
//   return _tp.t_h_kg(enthalpy);
// }

Real
SalineFluidProperties::mu_from_p_T(Real pressure, Real temperature) const
{
  return _tp.mu(temperature, pressure * Pa_to_kPa) * mN_to_N; // Ns/m^2
}

Real
SalineFluidProperties::k_from_p_T(Real pressure, Real temperature) const
{
  return _tp.k(temperature, pressure * Pa_to_kPa); // W/m/K
}

#endif
