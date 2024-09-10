//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
//
#include "SalineMoltenSaltFluidProperties.h"

registerMooseObject("FluidPropertiesApp", SalineMoltenSaltFluidProperties);

InputParameters
SalineMoltenSaltFluidProperties::validParams()
{
  InputParameters params = SinglePhaseFluidProperties::validParams();
  const std::string description = "Molten salt fluid properties using Saline";
#ifdef THERMOCHIMICA_ENABLED
  params.addClassDescription(description);
#else
  params.addClassDescription(
      "To use this object, you need to have the `Saline` library installed. Refer to the "
      "documentation for guidance on how to enable it. (Original description: " +
      description + ")");
#endif
  params.addRequiredParam<std::vector<std::string>>("comp_name",
                                                    "The name of the components in the salt");
  params.addRequiredParam<std::vector<Real>>("comp_val",
                                             "The mole fraction of each salt component");
  params.addRequiredParam<std::string>(
      "prop_def_file",
      "Definition of a fluid property file, which must be a file path to the "
      "comma-separated data matching the Saline format.");
  return params;
}

SalineMoltenSaltFluidProperties::SalineMoltenSaltFluidProperties(const InputParameters & parameters)
  : SinglePhaseFluidProperties(parameters), _fd_size(1e-6)
{
#ifdef SALINE_ENABLED
  const auto & propDef = getParam<std::string>("prop_def_file");
  _d.load(propDef);
  bool success = _tp.initialize(&_d);
  if (!success)
    mooseError("The initialization of the Saline interface has failed");
  const auto & name = getParam<std::vector<std::string>>("comp_name");
  const auto & comp = getParam<std::vector<Real>>("comp_val");

  // Verify mole fractions
  Real mole_sum = 0.0;
  for (const auto val : comp)
    mole_sum += val;
  if (!MooseUtils::absoluteFuzzyEqual(mole_sum, 1.))
    mooseError("Mole fractions of defined salt compound do not sum to 1.0.");

  success = _tp.setComposition(name, comp);
  if (!success)
    mooseError("The composition set has failed");
  _fluid_name = MooseUtils::join(name, "-");
#else
  mooseError("Saline was not made available during the build and cannot be used. Make sure you "
             "have the 'modules/fluid_properties/contrib/saline' submodule checked out.");
#endif
}

#ifdef SALINE_ENABLED

std::string
SalineMoltenSaltFluidProperties::fluidName() const
{
  return _fluid_name;
}

Real
SalineMoltenSaltFluidProperties::rho_from_p_T(Real pressure, Real temperature) const
{
  return _tp.rho_kgm3(temperature, pressure * Pa_to_kPa);
}

void
SalineMoltenSaltFluidProperties::rho_from_p_T(
    Real pressure, Real temperature, Real & rho, Real & drho_dp, Real & drho_dT) const
{
  rho = rho_from_p_T(pressure, temperature);
  // Estimate derivatives for now because they are not provided by Saline
  drho_dp = 0.;
  drho_dT = (rho_from_p_T(pressure, temperature + 1.0) - rho) / 1.0;
}

Real
SalineMoltenSaltFluidProperties::cp_from_p_T(Real pressure, Real temperature) const
{
  return _tp.cp_kg(temperature, pressure * Pa_to_kPa); // J/kg/K
}

void
SalineMoltenSaltFluidProperties::cp_from_p_T(
    Real pressure, Real temperature, Real & cp, Real & dcp_dp, Real & dcp_dT) const
{
  cp = cp_from_p_T(pressure, temperature);
  dcp_dp = 0;
  dcp_dT = (cp_from_p_T(pressure, temperature + 1.0) - cp) / 1.0;
}

Real
SalineMoltenSaltFluidProperties::h_from_p_T(Real /*pressure*/, Real temperature) const
{
  return _tp.h_t_kg(temperature);
}

void
SalineMoltenSaltFluidProperties::h_from_p_T(
    Real /*pressure*/, Real temperature, Real & enthalpy, Real & dh_dp, Real & dh_dT) const
{
  enthalpy = h_from_p_T(0.0, temperature);
  dh_dp = 0.0;
  // finite difference approximation
  dh_dT = (h_from_p_T(0.0, temperature * (1 + _fd_size)) - enthalpy) / (temperature * _fd_size);
}

Real
SalineMoltenSaltFluidProperties::e_from_p_T(Real pressure, Real temperature) const
{
  return _tp.h_t_kg(temperature) - pressure / rho_from_p_T(pressure, temperature);
}

void
SalineMoltenSaltFluidProperties::e_from_p_T(
    Real pressure, Real temperature, Real & e, Real & de_dp, Real & de_dT) const
{
  const Real rho = rho_from_p_T(pressure, temperature);
  e = _tp.h_t_kg(temperature) - pressure / rho;
  de_dp = -1 / rho;
  // finite difference approximation
  de_dT = (e_from_p_T(pressure, temperature * (1 + _fd_size)) - e) / (temperature * _fd_size);
}

Real
SalineMoltenSaltFluidProperties::T_from_p_h(Real /*pressure*/, Real enthalpy) const
{
  return _tp.t_h_kg(enthalpy);
}

Real
SalineMoltenSaltFluidProperties::mu_from_p_T(Real pressure, Real temperature) const
{
  return _tp.mu(temperature, pressure * Pa_to_kPa) * mN_to_N; // Ns/m^2
}

void
SalineMoltenSaltFluidProperties::mu_from_p_T(
    Real pressure, Real temperature, Real & mu, Real & dmu_dp, Real & dmu_dT) const
{
  mu = _tp.mu(temperature, pressure * Pa_to_kPa) * mN_to_N;
  Real mu_p1 = _tp.mu(temperature, pressure * (1 + _fd_size) * Pa_to_kPa) * mN_to_N; // Ns/m^2
  Real mu_T1 = _tp.mu(temperature * (1 + _fd_size), pressure * Pa_to_kPa) * mN_to_N; // Ns/m^2
  dmu_dp = (mu_p1 - mu) / (_fd_size * pressure * Pa_to_kPa);
  dmu_dT = (mu_T1 - mu) / (_fd_size * temperature);
}

Real
SalineMoltenSaltFluidProperties::k_from_p_T(Real pressure, Real temperature) const
{
  return _tp.k(temperature, pressure * Pa_to_kPa); // W/m/K
}

void
SalineMoltenSaltFluidProperties::k_from_p_T(
    Real pressure, Real temperature, Real & k, Real & dk_dp, Real & dk_dT) const
{
  k = _tp.k(temperature, pressure * Pa_to_kPa);
  Real k_p1 = _tp.k(temperature, pressure * (1 + _fd_size) * Pa_to_kPa);
  Real k_T1 = _tp.k(temperature * (1 + _fd_size), pressure * Pa_to_kPa);
  dk_dp = (k_p1 - k) / (_fd_size * pressure * Pa_to_kPa);
  dk_dT = (k_T1 - k) / (_fd_size * temperature);
}

#endif
