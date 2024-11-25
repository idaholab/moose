//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <string>
#include "MathUtils.h"
#include "MooseTypes.h"
#include "libmesh/vector_value.h"
#include "HeatConductionNames.h"

namespace NS
{
using namespace HeatConduction;

static const std::string directions[3] = {"x", "y", "z"};

// geometric quantities
static const std::string pebble_diameter = "pebble_diameter";
static const std::string infinite_porosity = "infinite_porosity";
static const std::string axis = "axis";
static const std::string center = "center";
static const std::string wall_porosity = "wall_porosity";
static const std::string wall_distance = "wall_distance";

// Names defined in Navier-Stokes
static const std::string density = "rho";
static const std::string superficial_density = "superficial_rho";
static const std::string momentum_x = "rhou";
static const std::string momentum_y = "rhov";
static const std::string momentum_z = "rhow";
static const std::string momentum_vector[3] = {momentum_x, momentum_y, momentum_z};
static const std::string superficial_momentum_x = "superficial_rhou";
static const std::string superficial_momentum_y = "superficial_rhov";
static const std::string superficial_momentum_z = "superficial_rhow";
static const std::string superficial_momentum_vector[3] = {
    superficial_momentum_x, superficial_momentum_y, superficial_momentum_z};

static const std::string velocity = "velocity";
static const std::string velocity_x = "vel_x";
static const std::string velocity_y = "vel_y";
static const std::string velocity_z = "vel_z";
const std::string velocity_vector[3] = {velocity_x, velocity_y, velocity_z};
static const std::string superficial_velocity_x = "superficial_vel_x";
static const std::string superficial_velocity_y = "superficial_vel_y";
static const std::string superficial_velocity_z = "superficial_vel_z";
static const std::string superficial_velocity = "superficial_velocity";
static const std::string superficial_velocity_vector[3] = {
    superficial_velocity_x, superficial_velocity_y, superficial_velocity_z};
static const std::string pressure = "pressure";
// Starting variable name with pressure so it is displayed next to pressure in outputs
static const std::string total_pressure = "pressure_total";
static const std::string temperature = "temperature";

static const std::string internal_energy = "internal_energy";
static const std::string specific_internal_energy = "e";
static const std::string specific_total_energy = "et";
static const std::string internal_energy_density = "rho_e";
static const std::string total_energy_density = "rho_et";
static const std::string superficial_total_energy_density = "superficial_rho_et";

static const std::string specific_enthalpy = "h";
static const std::string specific_total_enthalpy = "ht";
static const std::string enthalpy_density = "rho_h";
static const std::string total_enthalpy_density = "rho_ht";
static const std::string superficial_total_enthalpy_density = "superficial_rho_ht";

static const std::string mixing_length = "mixing_length";
static const std::string wall_shear_stress = "wall_shear_stress";
static const std::string wall_yplus = "wall_yplus";
static const std::string eddy_viscosity = "eddy_viscosity";
static const std::string total_viscosity = "total_viscosity";

static const std::string mach_number = "Mach";
static const std::string specific_volume = "specific_volume";

static const std::string momentum = "momentum";
static const std::string v = "v";
static const std::string acceleration = "acceleration";

static const std::string fluid = "fp";

// for Navier-Stokes material props representing gradients of nonlin+aux vars
inline std::string
grad(const std::string & var)
{
  return MathUtils::gradName(var);
}
// for Navier-Stokes material props representing time derivatives of nonlin+aux vars
inline std::string
time_deriv(const std::string & var)
{
  return MathUtils::timeDerivName(var);
}

// Navier-Stokes Variables
// Relating to porous media
static const std::string porosity = "porosity";
static const std::string smoothed_porosity = "smoothed_porosity";
static const std::string T_fluid = "T_fluid";
static const std::string T_solid = "T_solid";
static const std::string heat_source = "heat_source";

// Navier-Stokes Materials
static const std::string cL = "Darcy_coefficient";
static const std::string cQ = "Forchheimer_coefficient";
static const std::string alpha_boussinesq = "alpha_b";
static const std::string drhos_dTs = "drhos_dTs";
static const std::string dks_dTs = "dks_dTs";
static const std::string kappa = "kappa";
static const std::string kappa_s = "kappa_s";
static const std::string rho_s = "rho_s";
static const std::string cp_s = "cp_s";
static const std::string k_s = "k_s";
static const std::string cp = "cp";
static const std::string cv = "cv";
static const std::string mu = "mu";
// Turbulent dynamic viscosity
static const std::string mu_t = "mu_t";
// Turbulent dynamic scalar viscosity
static const std::string mu_t_passive_scalar = "mu_t_passive_scalar";
// Effective viscosity = sum of viscosities
static const std::string mu_eff = "mu_eff";
static const std::string k = "k";
// Turbulence 'conductivity'
static const std::string k_t = "k_t";
static const std::string thermal_diffusivity = "thermal_diffusivity";
static const std::string alpha = "alpha";
static const std::string alpha_wall = "alpha_wall";
static const std::string solid = "solid";
static const std::string Prandtl = "Pr";
static const std::string turbulent_Prandtl = "Pr_t";
static const std::string Reynolds = "Re";
static const std::string Reynolds_hydraulic = "Re_h";
static const std::string Reynolds_interstitial = "Re_i";
static const std::string c = "c";
static const std::string speed = "speed";
static const std::string sound_speed = "sound_speed";

// Two phase mixture materials
static const std::string latent_heat = "latent_heat";
static const std::string T_liquidus = "T_liquidus";
static const std::string T_solidus = "T_solidus";
static const std::string alpha_exchange = "alpha_exchange";

// other Navier-Stokes terms
static const std::string component = "component";
static const std::string source_scaling = "source_scaling";

// SUPG terms
static const std::string matrix_tau = "matrix_tau";
static const std::string vector_tau = "vector_tau";
static const std::string scalar_tau = "scalar_tau";
static const std::string diagonal_tau = "diagonal_tau";
static const std::string A = "A";
static const std::string R = "R";
static const std::string S = "S";
static const std::string dS_dTs = "dS_dTs";
static const std::string F = "F";
static const std::string G = "G";
static const std::string dUdt = "dUdt";
static const std::string C = "C";
static const std::string Z = "Z";
static const std::string K = "K";
static const std::string mass_flux = "mass_flux";

// Turbulence

// Turbulence variables
static const std::string TKE = "tke";
static const std::string TKED = "epsilon";

/**
 * Wall treatment options
 */
enum class WallTreatmentEnum
{
  EQ_NEWTON = 0,
  EQ_INCREMENTAL = 1,
  EQ_LINEARIZED = 2,
  NEQ = 3
};

// Turbulence constants
static constexpr Real von_karman_constant = 0.4187;
static constexpr Real E_turb_constant = 9.793;
// Lower limit for mu_t
static constexpr Real mu_t_low_limit = 1.0e-8;
// Lower limit for epsilon in the k-epsilon
static constexpr Real epsilon_low_limit = 1.0e-8;
// Lower limit for y_plus
static constexpr Real min_y_plus = 1e-10;
}

namespace NS_DEFAULT_VALUES
{
static const Real infinite_porosity = 0.4;
static const int bed_axis = 2;
static const Real wall_porosity = 1.0;

static const Real k_epsilon = 1e-6;
static const Real vel_epsilon = 1e-8;

static const RealVectorValue center(0.0, 0.0, 0.0);
static const RealVectorValue acceleration(0.0, 0.0, 0.0);

// assumed that the RZ geometry is not annular unless otherwise specified
static const Real inner_radius = 0.0;
}

namespace NS_CONSTANTS
{
using namespace HeatConduction::Constants;
}
