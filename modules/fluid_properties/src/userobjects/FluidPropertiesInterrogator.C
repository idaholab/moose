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
#include "SinglePhaseFluidProperties.h"
#include "VaporMixtureFluidProperties.h"
#include "TwoPhaseFluidProperties.h"
#include "TwoPhaseNCGFluidProperties.h"
#include "TwoPhaseNCGPartialPressureFluidProperties.h"

registerMooseObject("FluidPropertiesApp", FluidPropertiesInterrogator);

InputParameters
FluidPropertiesInterrogator::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addRequiredParam<UserObjectName>("fp",
                                          "The name of the fluid properties object to query.");
  params.addParam<bool>("json", false, "Output in JSON format");
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
    _json(getParam<bool>("json")),
    _fp(&getUserObject<FluidProperties>("fp")),
    _fp_1phase(dynamic_cast<const SinglePhaseFluidProperties * const>(_fp)),
    _fp_2phase(dynamic_cast<const TwoPhaseFluidProperties * const>(_fp)),
    _fp_2phase_ncg(dynamic_cast<const TwoPhaseNCGFluidProperties * const>(_fp)),
    _fp_2phase_ncg_partial_pressure(
        dynamic_cast<const TwoPhaseNCGPartialPressureFluidProperties * const>(_fp)),
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
    InputParameters pars_2phase = compute2Phase();
    InputParameters pars_liquid = compute1Phase(_fp_liquid, false);
    InputParameters pars_mixture = computeVaporMixture(false);

    if (_json)
    {
      nlohmann::json json;

      auto & json_2phase = json["2-phase"];
      buildJSON2Phase(json_2phase, pars_2phase);
      auto & json_liquid = json["liquid"];
      buildJSON1Phase(json_liquid, pars_liquid);
      auto & json_mixture = json["vapor-mixture"];
      buildJSONVaporMixture(json_mixture, pars_mixture);

      Moose::out << "**START JSON DATA**\n";
      Moose::out << json << "\n";
      Moose::out << "**END JSON DATA**" << std::endl;
    }
    else
    {
      outputASCII2Phase(pars_2phase);
      outputASCII1Phase("LIQUID phase", pars_liquid);
      outputASCIIVaporMixture(pars_mixture);
    }
  }
  else if (_has_2phase)
  {
    InputParameters pars_2phase = compute2Phase();
    InputParameters pars_liquid = compute1Phase(_fp_liquid, false);
    InputParameters pars_vapor = compute1Phase(_fp_vapor, false);

    if (_json)
    {
      nlohmann::json json;

      auto & json_2phase = json["2-phase"];
      buildJSON2Phase(json_2phase, pars_2phase);
      if (pars_liquid.have_parameter<bool>("specified"))
      {
        auto & json_liquid = json["liquid"];
        buildJSON1Phase(json_liquid, pars_liquid);
      }
      if (pars_vapor.have_parameter<bool>("specified"))
      {
        auto & json_vapor = json["vapor"];
        buildJSON1Phase(json_vapor, pars_vapor);
      }

      Moose::out << "**START JSON DATA**\n";
      Moose::out << json << "\n";
      Moose::out << "**END JSON DATA**" << std::endl;
    }
    else
    {
      outputASCII2Phase(pars_2phase);
      if (pars_liquid.have_parameter<bool>("specified"))
        outputASCII1Phase("LIQUID phase", pars_liquid);
      if (pars_vapor.have_parameter<bool>("specified"))
        outputASCII1Phase("VAPOR phase", pars_vapor);
    }
  }
  else if (_has_vapor_mixture)
  {
    InputParameters pars_mixture = computeVaporMixture(true);
    if (_json)
    {
      nlohmann::json json;
      buildJSONVaporMixture(json, pars_mixture);

      Moose::out << "**START JSON DATA**\n";
      Moose::out << json << "\n";
      Moose::out << "**END JSON DATA**" << std::endl;
    }
    else
      outputASCIIVaporMixture(pars_mixture);
  }
  else
  {
    InputParameters pars_1phase = compute1Phase(_fp_1phase, true);

    if (_json)
    {
      nlohmann::json json;

      buildJSON1Phase(json, pars_1phase);

      Moose::out << "**START JSON DATA**\n";
      Moose::out << json << "\n";
      Moose::out << "**END JSON DATA**" << std::endl;
    }
    else
      outputASCII1Phase("Single-phase", pars_1phase);
  }
}

void
FluidPropertiesInterrogator::finalize()
{
}

InputParameters
FluidPropertiesInterrogator::compute1Phase(const SinglePhaseFluidProperties * const fp_1phase,
                                           bool throw_error_if_no_match)
{
  InputParameters params = emptyInputParameters();

  // reset NaN flag
  _nan_encountered = false;

  // determine how state is specified
  std::vector<std::vector<std::string>> parameter_sets = {{"p", "T"}, {"rho", "e"}, {"rho", "p"}};
  if (_has_1phase)
    parameter_sets.push_back({"rho", "rhou", "rhoE"});
  auto specified = getSpecifiedSetMap(parameter_sets, "one-phase", throw_error_if_no_match);

  // compute/determine rho, e, p, T, vel

  Real rho, e, p, T, vel = 0;
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
    params.set<bool>("specified") = true;

    const Real v = 1.0 / rho;

    params.set<Real>("rho") = rho;
    params.set<Real>("e") = e;
    params.set<Real>("p") = p;
    params.set<Real>("T") = T;
    params.set<Real>("v") = v;
    params.set<Real>("h") = fp_1phase->h_from_p_T(p, T);
    params.set<Real>("s") = fp_1phase->s_from_v_e(v, e);
    params.set<Real>("c") = fp_1phase->c_from_v_e(v, e);
    params.set<Real>("mu") = fp_1phase->mu_from_v_e(v, e);
    params.set<Real>("cp") = fp_1phase->cp_from_v_e(v, e);
    params.set<Real>("cv") = fp_1phase->cv_from_v_e(v, e);
    params.set<Real>("k") = fp_1phase->k_from_v_e(v, e);
    params.set<Real>("beta") = fp_1phase->beta_from_p_T(p, T);

    if (isParamValid("vel") || specified["rho,rhou,rhoE"])
    {
      const Real s = fp_1phase->s_from_v_e(v, e);
      const Real s0 = s;
      const Real h = fp_1phase->h_from_p_T(p, T);
      const Real h0 = h + 0.5 * vel * vel;
      const Real p0 = fp_1phase->p_from_h_s(h0, s0);
      const Real rho0 = fp_1phase->rho_from_p_s(p0, s0);
      const Real e0 = fp_1phase->e_from_p_rho(p0, rho0);
      const Real v0 = 1.0 / rho0;
      const Real T0 = fp_1phase->T_from_v_e(v0, e0);
      const Real c0 = fp_1phase->c_from_v_e(v0, e0);
      const Real mu0 = fp_1phase->mu_from_v_e(v0, e0);
      const Real cp0 = fp_1phase->cp_from_v_e(v0, e0);
      const Real cv0 = fp_1phase->cv_from_v_e(v0, e0);
      const Real k0 = fp_1phase->k_from_v_e(v0, e0);
      const Real beta0 = fp_1phase->beta_from_p_T(p0, T0);

      params.set<Real>("vel") = vel;
      params.set<Real>("rho0") = rho0;
      params.set<Real>("s0") = s0;
      params.set<Real>("v0") = v0;
      params.set<Real>("e0") = e0;
      params.set<Real>("h0") = h0;
      params.set<Real>("p0") = p0;
      params.set<Real>("T0") = T0;
      params.set<Real>("c0") = c0;
      params.set<Real>("mu0") = mu0;
      params.set<Real>("cp0") = cp0;
      params.set<Real>("cv0") = cv0;
      params.set<Real>("k0") = k0;
      params.set<Real>("beta0") = beta0;
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

  return params;
}

InputParameters
FluidPropertiesInterrogator::compute2Phase()
{
  InputParameters params = emptyInputParameters();

  // reset NaN flag
  _nan_encountered = false;

  // determine how state is specified
  std::vector<std::vector<std::string>> parameter_sets = {{"p", "T"}, {"p"}, {"T"}};
  auto specified = getSpecifiedSetMap(parameter_sets, "two-phase", true);

  const Real p_critical = _fp_2phase->p_critical();
  params.set<Real>("p_critical") = p_critical;
  if (specified["p"])
  {
    const Real p = getParam<Real>("p");
    const Real T_sat = _fp_2phase->T_sat(p);
    const Real h_lat = _fp_2phase->h_lat(p, T_sat);

    params.set<Real>("p") = p;
    params.set<Real>("T_sat") = T_sat;
    params.set<Real>("h_lat") = h_lat;
  }
  else if (specified["T"])
  {
    const Real T = getParam<Real>("T");
    const Real p_sat = _fp_2phase->p_sat(T);
    const Real h_lat = _fp_2phase->h_lat(p_sat, T);
    const Real sigma = _fp_2phase->sigma_from_T(T);

    params.set<Real>("T") = T;
    params.set<Real>("p_sat") = p_sat;
    params.set<Real>("h_lat") = h_lat;
    params.set<Real>("sigma") = sigma;
  }
  else if (specified["p,T"])
  {
    const Real p = getParam<Real>("p");
    const Real T = getParam<Real>("T");
    const Real h_lat = _fp_2phase->h_lat(p, T);

    params.set<Real>("p") = p;
    params.set<Real>("T") = T;
    params.set<Real>("h_lat") = h_lat;
  }

  // warn if NaN encountered
  if (_nan_encountered)
    mooseWarning("At least one NaN was encountered.");

  return params;
}

InputParameters
FluidPropertiesInterrogator::computeVaporMixture(bool throw_error_if_no_match)
{
  InputParameters params = emptyInputParameters();

  // reset NaN flag
  _nan_encountered = false;

  // determine how state is specified
  std::vector<std::vector<std::string>> parameter_sets = {
      {"p", "T", "x_ncg"}, {"rho", "e", "x_ncg"}, {"rho", "rhou", "rhoE", "x_ncg"}, {"p", "T"}};
  auto specified = getSpecifiedSetMap(parameter_sets, "vapor mixture", throw_error_if_no_match);

  // compute/determine rho, e, p, T, vel

  Real rho, e, p, T, vel = 0;
  std::vector<Real> x_ncg;
  bool specified_a_set = false;
  if (specified["rho,e,x_ncg"])
  {
    rho = getParam<Real>("rho");
    e = getParam<Real>("e");
    x_ncg = getParam<std::vector<Real>>("x_ncg");
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
    x_ncg = getParam<std::vector<Real>>("x_ncg");
    rho = _fp_vapor_mixture->rho_from_p_T(p, T, x_ncg);
    e = _fp_vapor_mixture->e_from_p_T(p, T, x_ncg);
    if (isParamValid("vel"))
      vel = getParam<Real>("vel");

    specified_a_set = true;
  }
  else if (_fp_2phase_ncg_partial_pressure && specified["p,T"])
  {
    p = getParam<Real>("p");
    T = getParam<Real>("T");
    x_ncg = {_fp_2phase_ncg_partial_pressure->x_sat_ncg_from_p_T(p, T)};
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
    x_ncg = getParam<std::vector<Real>>("x_ncg");

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
    params.set<bool>("specified") = true;

    const Real v = 1.0 / rho;
    const Real h = e + p / rho;
    const Real c = _fp_vapor_mixture->c_from_p_T(p, T, x_ncg);
    const Real mu = _fp_vapor_mixture->mu_from_p_T(p, T, x_ncg);
    const Real cp = _fp_vapor_mixture->cp_from_p_T(p, T, x_ncg);
    const Real cv = _fp_vapor_mixture->cv_from_p_T(p, T, x_ncg);
    const Real k = _fp_vapor_mixture->k_from_p_T(p, T, x_ncg);

    params.set<std::vector<Real>>("x_ncg") = x_ncg;
    params.set<Real>("p") = p;
    params.set<Real>("T") = T;
    params.set<Real>("rho") = rho;
    params.set<Real>("e") = e;
    params.set<Real>("v") = v;
    params.set<Real>("h") = h;
    params.set<Real>("c") = c;
    params.set<Real>("mu") = mu;
    params.set<Real>("cp") = cp;
    params.set<Real>("cv") = cv;
    params.set<Real>("k") = k;

    if (isParamValid("vel") || specified["rho,rhou,rhoE,x_ncg"])
    {
      const Real h = e + p / rho;
      const Real h0 = h + 0.5 * vel * vel;

      params.set<Real>("h0") = h0;
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

  return params;
}

void
FluidPropertiesInterrogator::buildJSON1Phase(nlohmann::json & json, const InputParameters & params)
{
  for (auto p : {"rho", "e", "p", "T", "v", "h", "s", "c", "mu", "cp", "cv", "k", "beta"})
    json["static"][p] = params.get<Real>(p);

  if (params.have_parameter<Real>("vel"))
    for (auto p : {"vel",
                   "rho0",
                   "s0",
                   "v0",
                   "e0",
                   "h0",
                   "p0",
                   "T0",
                   "c0",
                   "mu0",
                   "cp0",
                   "cv0",
                   "k0",
                   "beta0"})
      json["stagnation"][p] = params.get<Real>(p);
}

void
FluidPropertiesInterrogator::buildJSON2Phase(nlohmann::json & json, const InputParameters & params)
{
  json["p_critical"] = params.get<Real>("p_critical");
  for (auto p : {"T_sat", "p_sat", "h_lat", "sigma"})
    if (params.have_parameter<Real>(p))
      json[p] = params.get<Real>(p);
}

void
FluidPropertiesInterrogator::buildJSONVaporMixture(nlohmann::json & json,
                                                   const InputParameters & params)
{
  for (auto p : {"p", "T", "rho", "e", "v", "h", "c", "mu", "cp", "cv", "k"})
    if (params.have_parameter<Real>(p))
      json["static"][p] = params.get<Real>(p);

  if (params.have_parameter<Real>("vel"))
    json["stagnation"]["h0"] = params.get<Real>("h0");
}

void
FluidPropertiesInterrogator::outputASCII1Phase(const std::string & description,
                                               const InputParameters & params)
{
  // output static property values
  outputHeader(description + " STATIC properties");
  outputStaticProperties(params);

  // output stagnation property values
  if (params.have_parameter<Real>("vel"))
  {
    outputHeader(description + " STAGNATION properties");
    outputStagnationProperties(params);
  }
}

void
FluidPropertiesInterrogator::outputASCII2Phase(const InputParameters & params)
{
  outputHeader("TWO-PHASE properties");
  outputProperty("Critical pressure", params.get<Real>("p_critical"), "Pa");
  if (params.have_parameter<Real>("T_sat"))
    outputProperty("Saturation temperature", params.get<Real>("T_sat"), "K");
  if (params.have_parameter<Real>("p_sat"))
    outputProperty("Saturation pressure", params.get<Real>("p_sat"), "Pa");
  if (params.have_parameter<Real>("h_lat"))
    outputProperty("Latent heat of vaporization", params.get<Real>("h_lat"), "J/kg");
  if (params.have_parameter<Real>("sigma"))
    outputProperty("Surface tension", params.get<Real>("sigma"), "N/m");
  _console << std::endl;
}

void
FluidPropertiesInterrogator::outputASCIIVaporMixture(const InputParameters & params)
{
  // output static property values
  outputHeader("Vapor mixture STATIC properties");
  outputVaporMixtureStaticProperties(params);

  // output stagnation property values
  if (params.have_parameter<Real>("vel"))
  {
    outputHeader("Vapor mixture STAGNATION properties");
    outputVaporMixtureStagnationProperties(params);
  }
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
            mooseError("(",
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
    ss << "For " << fp_type
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
FluidPropertiesInterrogator::outputStaticProperties(const InputParameters & params)
{
  outputProperty("Pressure", params.get<Real>("p"), "Pa");
  outputProperty("Temperature", params.get<Real>("T"), "K");
  outputProperty("Density", params.get<Real>("rho"), "kg/m^3");
  outputProperty("Specific volume", params.get<Real>("v"), "m^3/kg");
  outputProperty("Specific internal energy", params.get<Real>("e"), "J/kg");
  outputProperty("Specific enthalpy", params.get<Real>("h"), "J/kg");
  outputProperty("Specific entropy", params.get<Real>("s"), "J/(kg-K)");
  _console << std::endl;
  outputProperty("Sound speed", params.get<Real>("c"), "m/s");
  outputProperty("Dynamic viscosity", params.get<Real>("mu"), "Pa-s");
  outputProperty("Specific heat at constant pressure", params.get<Real>("cp"), "J/(kg-K)");
  outputProperty("Specific heat at constant volume", params.get<Real>("cv"), "J/(kg-K)");
  outputProperty("Thermal conductivity", params.get<Real>("k"), "W/(m-K)");
  outputProperty("Volumetric expansion coefficient", params.get<Real>("beta"), "1/K");
  _console << std::endl;
}

void
FluidPropertiesInterrogator::outputStagnationProperties(const InputParameters & params)
{
  outputProperty("Pressure", params.get<Real>("p0"), "Pa");
  outputProperty("Temperature", params.get<Real>("T0"), "K");
  outputProperty("Density", params.get<Real>("rho0"), "kg/m^3");
  outputProperty("Specific volume", params.get<Real>("v0"), "m^3/kg");
  outputProperty("Specific internal energy", params.get<Real>("e0"), "J/kg");
  outputProperty("Specific enthalpy", params.get<Real>("h0"), "J/kg");
  outputProperty("Specific entropy", params.get<Real>("s0"), "J/(kg-K)");
  _console << std::endl;
  outputProperty("Sound speed", params.get<Real>("c0"), "m/s");
  outputProperty("Dynamic viscosity", params.get<Real>("mu0"), "Pa-s");
  outputProperty("Specific heat at constant pressure", params.get<Real>("cp0"), "J/(kg-K)");
  outputProperty("Specific heat at constant volume", params.get<Real>("cv0"), "J/(kg-K)");
  outputProperty("Thermal conductivity", params.get<Real>("k0"), "W/(m-K)");
  outputProperty("Volumetric expansion coefficient", params.get<Real>("beta0"), "1/K");
  _console << std::endl;
}

void
FluidPropertiesInterrogator::outputVaporMixtureStaticProperties(const InputParameters & params)
{
  const auto x_ncg = params.get<std::vector<Real>>("x_ncg");
  for (unsigned int i = 0; i < x_ncg.size(); i++)
    outputProperty("Mass fraction " + std::to_string(i), x_ncg[i], "-");
  outputProperty("Pressure", params.get<Real>("p"), "Pa");
  outputProperty("Temperature", params.get<Real>("T"), "K");
  outputProperty("Density", params.get<Real>("rho"), "kg/m^3");
  outputProperty("Specific volume", params.get<Real>("v"), "m^3/kg");
  outputProperty("Specific internal energy", params.get<Real>("e"), "J/kg");
  outputProperty("Specific enthalpy", params.get<Real>("h"), "J/kg");
  _console << std::endl;
  outputProperty("Sound speed", params.get<Real>("c"), "m/s");
  outputProperty("Dynamic viscosity", params.get<Real>("mu"), "Pa-s");
  outputProperty("Specific heat at constant pressure", params.get<Real>("cp"), "J/(kg-K)");
  outputProperty("Specific heat at constant volume", params.get<Real>("cv"), "J/(kg-K)");
  outputProperty("Thermal conductivity", params.get<Real>("k"), "W/(m-K)");
  _console << std::endl;
}

void
FluidPropertiesInterrogator::outputVaporMixtureStagnationProperties(const InputParameters & params)
{
  outputProperty("Specific enthalpy", params.get<Real>("h0"), "J/kg");
  _console << std::endl;
}
