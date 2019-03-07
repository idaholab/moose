//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FluidPropertiesInterrogator.h"
#include "FluidProperties.h"
#include "LiquidFluidPropertiesInterface.h"
#include "SinglePhaseFluidProperties.h"
#include "VaporMixtureFluidProperties.h"
#include "TwoPhaseFluidProperties.h"
#include "TwoPhaseNCGFluidProperties.h"

registerMooseObject("FluidPropertiesApp", FluidPropertiesInterrogator);

template <>
InputParameters
validParams<FluidPropertiesInterrogator>()
{
  InputParameters params = validParams<GeneralUserObject>();
  params.addRequiredParam<UserObjectName>("fp",
                                          "The name of the fluid properties object to query.");
  params.addParam<Real>("rho", "Density");
  params.addParam<Real>("rhou", "Momentum density; rho * u");
  params.addParam<Real>("rhoE", "Total energy density; rho * E");
  params.addParam<Real>("e", "Specific internal energy");
  params.addParam<Real>("p", "Pressure");
  params.addParam<Real>("T", "Temperature");
  params.addParam<Real>("vel", "Velocity");
  params.addParam<std::vector<Real>>("x_ncg", "Mass fractions of NCGs");
  params.addRequiredParam<unsigned int>("precision", "Precision for printing values");

  params.addClassDescription(
      "User object for querying a single-phase or two-phase fluid properties object");

  return params;
}

FluidPropertiesInterrogator::FluidPropertiesInterrogator(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _fp(&getUserObject<FluidProperties>("fp")),
    _fp_1phase(dynamic_cast<const SinglePhaseFluidProperties * const>(_fp)),
    _fp_2phase(dynamic_cast<const TwoPhaseFluidProperties * const>(_fp)),
    _fp_2phase_ncg(dynamic_cast<const TwoPhaseNCGFluidProperties * const>(_fp)),
    _has_1phase(_fp_1phase),
    _has_vapor_mixture(dynamic_cast<const VaporMixtureFluidProperties * const>(_fp)),
    _has_2phase(_fp_2phase),
    _has_2phase_ncg(_fp_2phase_ncg),
    _fp_liquid(_has_2phase
                   ? &getUserObjectByName<SinglePhaseFluidProperties>(_fp_2phase->getLiquidName())
                   : nullptr),
    _fp_vapor(_has_2phase
                  ? &getUserObjectByName<SinglePhaseFluidProperties>(_fp_2phase->getVaporName())
                  : nullptr),
    _fp_vapor_mixture(_has_vapor_mixture
                          ? dynamic_cast<const VaporMixtureFluidProperties * const>(_fp)
                          : (_has_2phase_ncg ? &getUserObjectByName<VaporMixtureFluidProperties>(
                                                   _fp_2phase_ncg->getVaporMixtureName())
                                             : nullptr)),
    _nan_encountered(false),
    _precision(getParam<unsigned int>("precision"))
{
  if (!(_has_1phase || _has_2phase || _has_vapor_mixture))
    mooseError(
        "The type of the parameter 'fp' must be derived from type 'SinglePhaseFluidProperties', "
        "'VaporMixtureFluidProperties', or 'TwoPhaseFluidProperties'.");
}

void
FluidPropertiesInterrogator::initialize()
{
}

void
FluidPropertiesInterrogator::execute()
{
  if (_has_2phase_ncg)
  {
    execute2Phase();
    execute1Phase(_fp_liquid, "LIQUID phase", false);
    executeVaporMixture(false);
  }
  else if (_has_2phase)
  {
    execute2Phase();
    execute1Phase(_fp_liquid, "LIQUID phase", false);
    execute1Phase(_fp_vapor, "VAPOR phase", false);
  }
  else if (_has_vapor_mixture)
    executeVaporMixture(true);
  else
    execute1Phase(_fp_1phase, "Single-phase", true);
}

void
FluidPropertiesInterrogator::finalize()
{
}

void
FluidPropertiesInterrogator::execute1Phase(const SinglePhaseFluidProperties * const fp_1phase,
                                           const std::string & description,
                                           bool throw_error_if_no_match)
{
  // reset NaN flag
  _nan_encountered = false;

  // determine how state is specified
  std::vector<std::vector<std::string>> parameter_sets = {{"p", "T"}, {"rho", "e"}, {"rho", "p"}};
  if (_has_1phase)
    parameter_sets.push_back({"rho", "rhou", "rhoE"});
  auto specified = getSpecifiedSetMap(parameter_sets, "one-phase", throw_error_if_no_match);

  // compute/determine rho, e, p, T, vel

  Real rho, e, p, T, vel;
  bool specified_a_set = false;
  if (specified["rho,e"])
  {
    rho = getParam<Real>("rho");
    e = getParam<Real>("e");
    const Real v = 1.0 / rho;
    p = fp_1phase->p_from_v_e(v, e);
    T = fp_1phase->T_from_v_e(v, e);
    if (isParamValid("vel"))
      vel = getParam<Real>("vel");

    specified_a_set = true;
  }
  else if (specified["rho,p"])
  {
    rho = getParam<Real>("rho");
    p = getParam<Real>("p");
    const Real v = 1.0 / rho;
    e = fp_1phase->e_from_p_rho(p, rho);
    T = fp_1phase->T_from_v_e(v, e);
    if (isParamValid("vel"))
      vel = getParam<Real>("vel");

    specified_a_set = true;
  }
  else if (specified["p,T"])
  {
    p = getParam<Real>("p");
    T = getParam<Real>("T");
    rho = fp_1phase->rho_from_p_T(p, T);
    e = fp_1phase->e_from_p_rho(p, rho);
    if (isParamValid("vel"))
      vel = getParam<Real>("vel");

    specified_a_set = true;
  }
  else if (specified["rho,rhou,rhoE"])
  {
    rho = getParam<Real>("rho");
    const Real rhou = getParam<Real>("rhou");
    const Real rhoE = getParam<Real>("rhoE");

    vel = rhou / rho;
    const Real E = rhoE / rho;
    e = E - 0.5 * vel * vel;

    const Real v = 1.0 / rho;
    p = fp_1phase->p_from_v_e(v, e);
    T = fp_1phase->T_from_v_e(v, e);

    specified_a_set = true;
  }

  if (specified_a_set)
  {
    // output static property values
    outputHeader(description + " STATIC properties");
    outputStaticProperties(fp_1phase, rho, e, p, T);

    // output liquid-only property values
    const LiquidFluidPropertiesInterface * const liquid_fp =
        dynamic_cast<const LiquidFluidPropertiesInterface * const>(fp_1phase);
    if (liquid_fp)
      outputLiquidSpecificProperties(liquid_fp, p, T);

    // output stagnation property values
    if (isParamValid("vel") || specified["rho,rhou,rhoE"])
    {
      outputHeader(description + " STAGNATION properties");
      outputStagnationProperties(fp_1phase, rho, e, p, T, vel);
    }

    // warn if NaN encountered
    if (_nan_encountered)
      mooseWarning(
          "At least one NaN was encountered. This implies one of the following:\n",
          "  1. The specified thermodynamic state is inconsistent with the equation of state\n",
          "     (for example, the state corresponds to a different phase of the fluid).\n",
          "  2. There is a problem with the equation of state package at this state\n",
          "     (for example, the supplied state is outside of the valid state space\n",
          "     that was programmed in the package).");
  }
}

void
FluidPropertiesInterrogator::executeVaporMixture(bool throw_error_if_no_match)
{
  // reset NaN flag
  _nan_encountered = false;

  // determine how state is specified
  std::vector<std::vector<std::string>> parameter_sets = {
      {"p", "T", "x_ncg"}, {"rho", "e", "x_ncg"}, {"rho", "rhou", "rhoE", "x_ncg"}};
  auto specified = getSpecifiedSetMap(parameter_sets, "vapor mixture", throw_error_if_no_match);

  const auto x_ncg = getParam<std::vector<Real>>("x_ncg");

  // compute/determine rho, e, p, T, vel

  Real rho, e, p, T, vel;
  bool specified_a_set = false;
  if (specified["rho,e,x_ncg"])
  {
    rho = getParam<Real>("rho");
    e = getParam<Real>("e");
    const Real v = 1.0 / rho;
    p = _fp_vapor_mixture->p_from_v_e(v, e, x_ncg);
    T = _fp_vapor_mixture->T_from_v_e(v, e, x_ncg);
    if (isParamValid("vel"))
      vel = getParam<Real>("vel");

    specified_a_set = true;
  }
  else if (specified["p,T,x_ncg"])
  {
    p = getParam<Real>("p");
    T = getParam<Real>("T");
    rho = _fp_vapor_mixture->rho_from_p_T(p, T, x_ncg);
    e = _fp_vapor_mixture->e_from_p_T(p, T, x_ncg);
    if (isParamValid("vel"))
      vel = getParam<Real>("vel");

    specified_a_set = true;
  }
  else if (specified["rho,rhou,rhoE,x_ncg"])
  {
    rho = getParam<Real>("rho");
    const Real rhou = getParam<Real>("rhou");
    const Real rhoE = getParam<Real>("rhoE");

    vel = rhou / rho;
    const Real E = rhoE / rho;
    e = E - 0.5 * vel * vel;

    const Real v = 1.0 / rho;
    p = _fp_vapor_mixture->p_from_v_e(v, e, x_ncg);
    T = _fp_vapor_mixture->T_from_v_e(v, e, x_ncg);

    specified_a_set = true;
  }

  if (specified_a_set)
  {
    // output static property values
    outputHeader("Vapor mixture STATIC properties");
    outputVaporMixtureStaticProperties(rho, e, p, T, x_ncg);

    // output stagnation property values
    if (isParamValid("vel") || specified["rho,rhou,rhoE,x_ncg"])
    {
      outputHeader("Vapor mixture STAGNATION properties");
      outputVaporMixtureStagnationProperties(rho, e, p, T, x_ncg, vel);
    }

    // warn if NaN encountered
    if (_nan_encountered)
      mooseWarning(
          "At least one NaN was encountered. This implies one of the following:\n",
          "  1. The specified thermodynamic state is inconsistent with the equation of state\n",
          "     (for example, the state corresponds to a different phase of the fluid).\n",
          "  2. There is a problem with the equation of state package at this state\n",
          "     (for example, the supplied state is outside of the valid state space\n",
          "     that was programmed in the package).");
  }
}

void
FluidPropertiesInterrogator::execute2Phase()
{
  // reset NaN flag
  _nan_encountered = false;

  // determine how state is specified
  std::vector<std::vector<std::string>> parameter_sets = {{"p", "T"}, {"p"}, {"T"}};
  auto specified = getSpecifiedSetMap(parameter_sets, "two-phase", true);

  // output the requested quantities

  outputHeader("TWO-PHASE properties");

  const Real p_critical = _fp_2phase->p_critical();
  outputProperty("Critical pressure", p_critical, "Pa");
  if (specified["p"])
  {
    const Real p = getParam<Real>("p");
    const Real T_sat = _fp_2phase->T_sat(p);
    const Real h_lat = _fp_2phase->h_lat(p, T_sat);
    outputProperty("Saturation temperature", T_sat, "K");
    outputProperty("Latent heat of vaporization", h_lat, "J/kg");
  }
  if (specified["T"])
  {
    const Real T = getParam<Real>("T");
    const Real p_sat = _fp_2phase->p_sat(T);
    const Real h_lat = _fp_2phase->h_lat(p_sat, T);
    outputProperty("Saturation pressure", p_sat, "Pa");
    outputProperty("Latent heat of vaporization", h_lat, "J/kg");
  }
  if (specified["p,T"])
  {
    const Real p = getParam<Real>("p");
    const Real T = getParam<Real>("T");
    const Real h_lat = _fp_2phase->h_lat(p, T);
    outputProperty("Latent heat of vaporization", h_lat, "J/kg");
  }
  _console << std::endl;

  // warn if NaN encountered
  if (_nan_encountered)
    mooseWarning("At least one NaN was encountered.");
}

std::map<std::string, bool>
FluidPropertiesInterrogator::getSpecifiedSetMap(
    const std::vector<std::vector<std::string>> & parameter_sets,
    const std::string & fp_type,
    bool throw_error_if_no_match) const
{
  // get union of parameters from all sets
  std::vector<std::string> parameter_union;
  for (auto & parameter_set : parameter_sets)
    parameter_union.insert(parameter_union.end(), parameter_set.begin(), parameter_set.end());
  std::sort(parameter_union.begin(), parameter_union.end());
  parameter_union.erase(std::unique(parameter_union.begin(), parameter_union.end()),
                        parameter_union.end());

  std::vector<std::string> parameter_set_names;
  std::map<std::string, bool> specified;
  bool specified_a_set = false;
  for (const auto & parameter_set : parameter_sets)
  {
    // create unique string to identify parameter set
    std::stringstream ss;
    for (unsigned int i = 0; i < parameter_set.size(); i++)
      if (i == 0)
        ss << parameter_set[i];
      else
        ss << "," << parameter_set[i];
    const std::string parameter_set_name = ss.str();
    parameter_set_names.push_back(parameter_set_name);

    // check if the set parameters were provided
    bool all_parameters_provided = true;
    for (const auto & parameter : parameter_set)
      if (!isParamValid(parameter))
        all_parameters_provided = false;

    if (all_parameters_provided)
    {
      // exclude set if a superset (assumed to be ordered before this set) was specified
      if (!specified_a_set)
      {
        specified[parameter_set_name] = true;

        // check that there are no extraneous parameters
        std::vector<std::string> parameter_set_sorted(parameter_set);
        std::sort(parameter_set_sorted.begin(), parameter_set_sorted.end());
        std::vector<std::string> extraneous_parameters;
        std::set_difference(parameter_union.begin(),
                            parameter_union.end(),
                            parameter_set_sorted.begin(),
                            parameter_set_sorted.end(),
                            std::inserter(extraneous_parameters, extraneous_parameters.end()));
        for (const auto & parameter : extraneous_parameters)
          if (isParamValid(parameter))
            mooseError(name(),
                       ": (",
                       parameter_set_name,
                       ") has been specified; ",
                       parameter,
                       " cannot be specified.");
      }

      specified_a_set = true;
    }
    else
      specified[parameter_set_name] = false;
  }

  if (!specified_a_set && throw_error_if_no_match)
  {
    std::stringstream ss;
    ss << name() << ": For " << fp_type
       << " fluid properties, you must provide one of the following\n"
          "combinations of thermodynamic properties:\n";
    for (const auto & parameter_set_name : parameter_set_names)
      ss << "  * (" << parameter_set_name << ")\n";
    mooseError(ss.str());
  }

  return specified;
}

void
FluidPropertiesInterrogator::outputHeader(const std::string & header) const
{
  _console << std::endl
           << std::endl
           << header << ":" << std::endl
           << std::setfill('-') << std::setw(80) << "-" << std::setfill(' ') << std::endl;
}

void
FluidPropertiesInterrogator::outputProperty(const std::string & name,
                                            const Real & value,
                                            const std::string & units)
{
  const bool use_scientific_notation = ((value < 0.001) || (value >= 10000.0));

  // check for NaN value
  const bool is_nan = value != value;
  if (is_nan)
    _nan_encountered = true;

  const std::string units_printed = is_nan ? "" : units;

  // The console output is not used directly because there is no way to reset
  // format flags. For example, if scientific format is used, there is no way
  // to restore the general format (not fixed format); for cout, there are
  // methods to save and restore format flags, but Console does not provide these.
  std::stringstream ss;

  if (use_scientific_notation)
    ss << std::setw(35) << std::left << name + ":" << std::setw(_precision + 10) << std::right
       << std::setprecision(_precision) << std::scientific << value << "  " << units_printed
       << std::endl;
  else
    ss << std::setw(35) << std::left << name + ":" << std::setw(_precision + 10) << std::right
       << std::setprecision(_precision) << value << "  " << units_printed << std::endl;

  _console << ss.str();
}

void
FluidPropertiesInterrogator::outputStaticProperties(const SinglePhaseFluidProperties * const fp,
                                                    const Real & rho,
                                                    const Real & e,
                                                    const Real & p,
                                                    const Real & T)
{
  const Real v = 1.0 / rho;
  const Real h = fp->h_from_p_T(p, T);
  const Real s = fp->s_from_v_e(v, e);
  const Real c = fp->c_from_v_e(v, e);
  const Real mu = fp->mu_from_v_e(v, e);
  const Real cp = fp->cp_from_v_e(v, e);
  const Real cv = fp->cv_from_v_e(v, e);
  const Real k = fp->k_from_v_e(v, e);
  const Real beta = fp->beta_from_p_T(p, T);

  outputProperty("Pressure", p, "Pa");
  outputProperty("Temperature", T, "K");
  outputProperty("Density", rho, "kg/m^3");
  outputProperty("Specific volume", v, "m^3/kg");
  outputProperty("Specific internal energy", e, "J/kg");
  outputProperty("Specific enthalpy", h, "J/kg");
  outputProperty("Specific entropy", s, "J/kg");
  _console << std::endl;
  outputProperty("Sound speed", c, "m/s");
  outputProperty("Dynamic viscosity", mu, "Pa-s");
  outputProperty("Specific heat at constant pressure", cp, "J/(kg-K)");
  outputProperty("Specific heat at constant volume", cv, "J/(kg-K)");
  outputProperty("Thermal conductivity", k, "W/(m-K)");
  outputProperty("Volumetric expansion coefficient", beta, "1/K");
  _console << std::endl;
}

void
FluidPropertiesInterrogator::outputLiquidSpecificProperties(
    const LiquidFluidPropertiesInterface * const liquid_fp, const Real & p, const Real & T)
{
  const Real sigma = liquid_fp->sigma_from_p_T(p, T);
  outputProperty("Surface tension", sigma, "N/m");
  _console << std::endl;
}

void
FluidPropertiesInterrogator::outputStagnationProperties(const SinglePhaseFluidProperties * const fp,
                                                        const Real & rho,
                                                        const Real & e,
                                                        const Real & p,
                                                        const Real & T,
                                                        const Real & vel)
{
  const Real v = 1.0 / rho;
  const Real s = fp->s_from_v_e(v, e);
  const Real s0 = s;
  const Real h = fp->h_from_p_T(p, T);
  const Real h0 = h + 0.5 * vel * vel;
  const Real p0 = fp->p_from_h_s(h0, s0);
  const Real rho0 = fp->rho_from_p_s(p0, s0);
  const Real e0 = fp->e_from_p_rho(p0, rho0);
  const Real v0 = 1.0 / rho0;
  const Real T0 = fp->T_from_v_e(v0, e0);
  const Real c0 = fp->c_from_v_e(v0, e0);
  const Real mu0 = fp->mu_from_v_e(v0, e0);
  const Real cp0 = fp->cp_from_v_e(v0, e0);
  const Real cv0 = fp->cv_from_v_e(v0, e0);
  const Real k0 = fp->k_from_v_e(v0, e0);
  const Real beta0 = fp->beta_from_p_T(p0, T0);

  outputProperty("Pressure", p0, "Pa");
  outputProperty("Temperature", T0, "K");
  outputProperty("Density", rho0, "kg/m^3");
  outputProperty("Specific volume", v0, "m^3/kg");
  outputProperty("Specific internal energy", e0, "J/kg");
  outputProperty("Specific enthalpy", h0, "J/kg");
  outputProperty("Specific entropy", s0, "J/kg");
  _console << std::endl;
  outputProperty("Sound speed", c0, "m/s");
  outputProperty("Dynamic viscosity", mu0, "Pa-s");
  outputProperty("Specific heat at constant pressure", cp0, "J/(kg-K)");
  outputProperty("Specific heat at constant volume", cv0, "J/(kg-K)");
  outputProperty("Thermal conductivity", k0, "W/(m-K)");
  outputProperty("Volumetric expansion coefficient", beta0, "1/K");
  _console << std::endl;
}

void
FluidPropertiesInterrogator::outputVaporMixtureStaticProperties(const Real & rho,
                                                                const Real & e,
                                                                const Real & p,
                                                                const Real & T,
                                                                const std::vector<Real> & x_ncg)
{
  const Real v = 1.0 / rho;
  const Real h = e + p / rho;
  const Real c = _fp_vapor_mixture->c_from_p_T(p, T, x_ncg);
  const Real mu = _fp_vapor_mixture->mu_from_p_T(p, T, x_ncg);
  const Real cp = _fp_vapor_mixture->cp_from_p_T(p, T, x_ncg);
  const Real cv = _fp_vapor_mixture->cv_from_p_T(p, T, x_ncg);
  const Real k = _fp_vapor_mixture->k_from_p_T(p, T, x_ncg);

  outputProperty("Pressure", p, "Pa");
  outputProperty("Temperature", T, "K");
  outputProperty("Density", rho, "kg/m^3");
  outputProperty("Specific volume", v, "m^3/kg");
  outputProperty("Specific internal energy", e, "J/kg");
  outputProperty("Specific enthalpy", h, "J/kg");
  _console << std::endl;
  outputProperty("Sound speed", c, "m/s");
  outputProperty("Dynamic viscosity", mu, "Pa-s");
  outputProperty("Specific heat at constant pressure", cp, "J/(kg-K)");
  outputProperty("Specific heat at constant volume", cv, "J/(kg-K)");
  outputProperty("Thermal conductivity", k, "W/(m-K)");
  _console << std::endl;
}

void
FluidPropertiesInterrogator::outputVaporMixtureStagnationProperties(
    const Real & rho,
    const Real & e,
    const Real & p,
    const Real & /*T*/,
    const std::vector<Real> & /*x_ncg*/,
    const Real & vel)
{
  const Real h = e + p / rho;
  const Real h0 = h + 0.5 * vel * vel;

  outputProperty("Specific enthalpy", h0, "J/kg");
  _console << std::endl;
}
