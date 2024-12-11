//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TabulatedFluidProperties.h"
#include "MooseUtils.h"
#include "Conversion.h"
#include "KDTree.h"
#include "BidimensionalInterpolation.h"
#include "NewtonInversion.h"

// C++ includes
#include <fstream>
#include <ctime>
#include <cmath>
#include <regex>

InputParameters
TabulatedFluidProperties::validParams()
{
  InputParameters params = SinglePhaseFluidProperties::validParams();
  params.addClassDescription(
      "Single phase fluid properties computed using bi-dimensional interpolation of tabulated "
      "values.");

  // Which interpolations to create
  params.addParam<bool>("create_pT_interpolations",
                        true,
                        "Whether to load (from file) or create (from a fluid property object) "
                        "properties interpolations from pressure and temperature");
  params.addParam<bool>("create_ve_interpolations",
                        false,
                        "Whether to load (from file) or create (from a fluid property object) "
                        "properties interpolations from pressure and temperature");

  // Input / output
  params.addParam<UserObjectName>("fp", "The name of the FluidProperties UserObject");
  params.addParam<FileName>("fluid_property_file",
                            "Name of the csv file containing the tabulated fluid property data.");
  params.addParam<FileName>(
      "fluid_property_ve_file",
      "Name of the csv file containing the tabulated (v,e) fluid property data.");
  params.addParam<FileName>("fluid_property_output_file",
                            "Name of the CSV file which can be output with the tabulation. This "
                            "file can then be read as a 'fluid_property_file'");
  params.addParam<FileName>(
      "fluid_property_ve_output_file",
      "Name of the CSV file which can be output with the (v,e) tabulation. This "
      "file can then be read as a 'fluid_property_ve_file'");
  params.addDeprecatedParam<bool>(
      "save_file",
      "Whether to save the csv fluid properties file",
      "This parameter is no longer required. Whether to save a CSV tabulation file is controlled "
      "by specifying the 'fluid_property_output_file' parameter");

  // Data source on a per-property basis
  MultiMooseEnum properties(
      "density enthalpy internal_energy viscosity k c cv cp entropy pressure temperature",
      "density enthalpy internal_energy viscosity");
  params.addParam<MultiMooseEnum>("interpolated_properties",
                                  properties,
                                  "Properties to interpolate if no data file is provided");

  // (p,T) grid parameters
  params.addRangeCheckedParam<Real>(
      "temperature_min", 300, "temperature_min > 0", "Minimum temperature for tabulated data.");
  params.addParam<Real>("temperature_max", 500, "Maximum temperature for tabulated data.");
  params.addRangeCheckedParam<Real>(
      "pressure_min", 1e5, "pressure_min > 0", "Minimum pressure for tabulated data.");
  params.addParam<Real>("pressure_max", 50.0e6, "Maximum pressure for tabulated data.");
  params.addRangeCheckedParam<unsigned int>(
      "num_T", 100, "num_T > 0", "Number of points to divide temperature range.");
  params.addRangeCheckedParam<unsigned int>(
      "num_p", 100, "num_p > 0", "Number of points to divide pressure range.");

  // (v,e) grid parameters
  params.addParam<Real>("e_min", "Minimum specific internal energy for tabulated data.");
  params.addParam<Real>("e_max", "Maximum specific internal energy for tabulated data.");
  params.addRangeCheckedParam<Real>(
      "v_min", "v_min > 0", "Minimum specific volume for tabulated data.");
  params.addRangeCheckedParam<Real>(
      "v_max", "v_max > 0", "Maximum specific volume for tabulated data.");
  params.addParam<bool>("construct_pT_from_ve",
                        false,
                        "If the lookup table (p, T) as functions of (v, e) should be constructed.");
  params.addParam<bool>("construct_pT_from_vh",
                        false,
                        "If the lookup table (p, T) as functions of (v, h) should be constructed.");
  params.addRangeCheckedParam<unsigned int>(
      "num_v",
      100,
      "num_v > 0",
      "Number of points to divide specific volume range for (v,e) lookups.");
  params.addRangeCheckedParam<unsigned int>("num_e",
                                            100,
                                            "num_e > 0",
                                            "Number of points to divide specific internal energy "
                                            "range for (v,e) lookups.");
  params.addParam<bool>(
      "use_log_grid_v",
      false,
      "Option to use a base-10 logarithmically-spaced grid for specific volume instead of a "
      "linearly-spaced grid.");
  params.addParam<bool>(
      "use_log_grid_e",
      false,
      "Option to use a base-10 logarithmically-spaced grid for specific internal energy instead "
      "of a linearly-spaced grid.");
  params.addParam<bool>(
      "use_log_grid_h",
      false,
      "Option to use a base-10 logarithmically-spaced grid for specific enthalpy instead "
      "of a linearly-spaced grid.");

  // Out of bounds behavior
  params.addDeprecatedParam<bool>(
      "error_on_out_of_bounds",
      "Whether pressure or temperature from tabulation exceeding user-specified bounds leads to "
      "an error.",
      "This parameter has been replaced by the 'out_of_bounds_behavior' parameter which offers "
      "more flexibility. The option to error is called 'throw' in that parameter.");
  // NOTE: this enum must remain the same as OOBBehavior in the header
  MooseEnum OOBBehavior("ignore throw declare_invalid warn_invalid set_to_closest_bound", "throw");
  params.addParam<MooseEnum>("out_of_bounds_behavior",
                             OOBBehavior,
                             "Property evaluation behavior when evaluated outside the "
                             "user-specified or tabulation-specified bounds");

  // This is generally a bad idea. However, several properties have not been tabulated so several
  // tests are relying on the original fp object to provide the value (for example for the
  // vaporPressure())
  params.addParam<bool>(
      "allow_fp_and_tabulation", false, "Whether to allow the two sources of data concurrently");

  params.addParamNamesToGroup("fluid_property_file fluid_property_ve_file "
                              "fluid_property_output_file fluid_property_ve_output_file",
                              "Tabulation file read/write");
  params.addParamNamesToGroup("construct_pT_from_ve construct_pT_from_vh",
                              "Variable set conversion");
  params.addParamNamesToGroup("temperature_min temperature_max pressure_min pressure_max e_min "
                              "e_max v_min v_max error_on_out_of_bounds out_of_bounds_behavior",
                              "Tabulation and interpolation bounds");
  params.addParamNamesToGroup(
      "num_T num_p num_v num_e use_log_grid_v use_log_grid_e use_log_grid_h",
      "Tabulation and interpolation discretization");

  return params;
}

TabulatedFluidProperties::TabulatedFluidProperties(const InputParameters & parameters)
  : SinglePhaseFluidProperties(parameters),
    _file_name_in(isParamValid("fluid_property_file") ? getParam<FileName>("fluid_property_file")
                                                      : ""),
    _file_name_ve_in(
        isParamValid("fluid_property_ve_file") ? getParam<FileName>("fluid_property_ve_file") : ""),
    _file_name_out(isParamValid("fluid_property_output_file")
                       ? getParam<FileName>("fluid_property_output_file")
                       : ""),
    _file_name_ve_out(isParamValid("fluid_property_ve_output_file")
                          ? getParam<FileName>("fluid_property_ve_output_file")
                          : ""),
    _save_file(isParamValid("save_file") ? getParam<bool>("save_file")
                                         : isParamValid("fluid_property_output_file")),
    _create_direct_pT_interpolations(getParam<bool>("create_pT_interpolations")),
    _create_direct_ve_interpolations(getParam<bool>("create_ve_interpolations")),
    _temperature_min(getParam<Real>("temperature_min")),
    _temperature_max(getParam<Real>("temperature_max")),
    _pressure_min(getParam<Real>("pressure_min")),
    _pressure_max(getParam<Real>("pressure_max")),
    _num_T(getParam<unsigned int>("num_T")),
    _num_p(getParam<unsigned int>("num_p")),
    _fp(isParamValid("fp") ? &getUserObject<SinglePhaseFluidProperties>("fp") : nullptr),
    _allow_fp_and_tabulation(getParam<bool>("allow_fp_and_tabulation")),
    _interpolated_properties_enum(getParam<MultiMooseEnum>("interpolated_properties")),
    _interpolated_properties(),
    _interpolate_density(false),
    _interpolate_enthalpy(false),
    _interpolate_internal_energy(false),
    _interpolate_viscosity(false),
    _interpolate_k(false),
    _interpolate_c(false),
    _interpolate_cp(false),
    _interpolate_cv(false),
    _interpolate_entropy(false),
    _interpolate_pressure(false),
    _interpolate_temperature(false),
    _density_idx(libMesh::invalid_uint),
    _enthalpy_idx(libMesh::invalid_uint),
    _internal_energy_idx(libMesh::invalid_uint),
    _viscosity_idx(libMesh::invalid_uint),
    _k_idx(libMesh::invalid_uint),
    _c_idx(libMesh::invalid_uint),
    _cp_idx(libMesh::invalid_uint),
    _cv_idx(libMesh::invalid_uint),
    _entropy_idx(libMesh::invalid_uint),
    _p_idx(libMesh::invalid_uint),
    _T_idx(libMesh::invalid_uint),
    _csv_reader(_file_name_in, &_communicator),
    _construct_pT_from_ve(getParam<bool>("construct_pT_from_ve")),
    _construct_pT_from_vh(getParam<bool>("construct_pT_from_vh")),
    _initial_setup_done(false),
    _num_v(getParam<unsigned int>("num_v")),
    _num_e(getParam<unsigned int>("num_e")),
    _log_space_v(getParam<bool>("use_log_grid_v")),
    _log_space_e(getParam<bool>("use_log_grid_e")),
    _log_space_h(getParam<bool>("use_log_grid_h")),
    _OOBBehavior(getParam<MooseEnum>("out_of_bounds_behavior"))
{
  // Check that initial guess (used in Newton Method) is within min and max values
  checkInitialGuess();
  // Sanity check on minimum and maximum temperatures and pressures
  if (_temperature_max <= _temperature_min)
    mooseError("temperature_max must be greater than temperature_min");
  if (_pressure_max <= _pressure_min)
    mooseError("pressure_max must be greater than pressure_min");

  // Set (v,e) bounds if specified by the user
  if (isParamValid("e_min") && isParamValid("e_max"))
  {
    _e_min = getParam<Real>("e_min");
    _e_max = getParam<Real>("e_max");
    _e_bounds_specified = true;
  }
  else if (isParamValid("e_min") || isParamValid("e_max"))
    paramError("e_min",
               "Either both or none of the min and max values of the specific internal energy "
               "should be specified");
  else
    _e_bounds_specified = false;
  if (isParamValid("v_min") && isParamValid("v_max"))
  {
    _v_min = getParam<Real>("v_min");
    _v_max = getParam<Real>("v_max");
    _v_bounds_specified = true;
  }
  else if (isParamValid("v_min") || isParamValid("v_max"))
    paramError("v_min",
               "Either both or none of the min and max values of the specific volume "
               "should be specified");
  else
    _v_bounds_specified = false;

  // Handle out of bounds behavior parameters and deprecation
  if (isParamValid("error_on_out_of_bounds") && getParam<bool>("error_on_out_of_bounds") &&
      _OOBBehavior != Throw)
    paramError("out_of_bounds_behavior", "Inconsistent selection of out of bounds behavior.");
  else if (isParamValid("error_on_out_of_bounds") && !getParam<bool>("error_on_out_of_bounds"))
    _OOBBehavior = SetToClosestBound;

  // Lines starting with # in the data file are treated as comments
  _csv_reader.setComment("#");

  // Can only and must receive one source of data between fp and tabulations
  if (_fp && (!_file_name_in.empty() || !_file_name_ve_in.empty()) && !_allow_fp_and_tabulation)
    paramError("fluid_property_file",
               "Cannot supply both a fluid properties object with 'fp' and a source tabulation "
               "file with 'fluid_property_file', unless 'allow_fp_and_tabulation' is set to true");
  if (!_fp && _file_name_in.empty() && _file_name_ve_in.empty())
    paramError("fluid_property_file",
               "Either a fluid properties object with the parameter 'fp' and a source tabulation "
               "file with the parameter 'fluid_property_file' or 'fluid_property_ve_file' should "
               "be provided.");
  if (!_create_direct_pT_interpolations && !_create_direct_ve_interpolations)
    paramError("create_pT_interpolations", "Must create either (p,T) or (v,e) interpolations");

  // Some parameters are not used when reading a tabulation
  if (!_fp && !_file_name_in.empty() &&
      (isParamSetByUser("pressure_min") || isParamSetByUser("pressure_max") ||
       isParamSetByUser("temperature_min") || isParamSetByUser("temperature_max")))
    mooseWarning("User-specified bounds in pressure and temperature are ignored when reading a "
                 "'fluid_property_file'. The tabulation bounds are selected "
                 "from the bounds of the input tabulation.");
  if (!_fp && !_file_name_in.empty() && (isParamSetByUser("num_p") || isParamSetByUser("num_T")))
    mooseWarning("User-specified grid sizes in pressure and temperature are ignored when reading a "
                 "'fluid_property_file'. The tabulation bounds are selected "
                 "from the bounds of the input tabulation.");
  if (!_fp && !_file_name_ve_in.empty() &&
      (isParamSetByUser("v_min") || isParamSetByUser("v_max") || isParamSetByUser("e_min") ||
       isParamSetByUser("e_max")))
    mooseWarning(
        "User-specified bounds in specific volume and internal energy are ignored when reading a "
        "'fluid_property_ve_file'. The tabulation bounds are selected "
        "from the bounds of the input tabulation.");
  if (!_fp && !_file_name_ve_in.empty() && (isParamSetByUser("num_e") || isParamSetByUser("num_v")))
    mooseWarning("User-specified grid sizes in specific volume and internal energy are ignored "
                 "when reading a 'fluid_property_ve_file'. The tabulation widths are read "
                 "from the input tabulation.");
  if (!_file_name_ve_in.empty() && (_log_space_v || _log_space_e))
    mooseWarning(
        "User specfied logarithmic grids in specific volume and energy are ignored when reading a "
        "'fluid_properties_ve_file'. The tabulation grid is read from the input tabulation");
}

void
TabulatedFluidProperties::initialSetup()
{
  if (_initial_setup_done)
    return;
  _initial_setup_done = true;

  if (_create_direct_pT_interpolations)
  {
    // If the user specified a (p, T) tabulation to read, use that
    if (!_file_name_in.empty())
      readFileTabulationData(true);
    else
    {
      if (!_fp)
        paramError(
            "create_pT_interpolations",
            "No FluidProperties (specified with 'fp' parameter) exists. Either specify a 'fp' or "
            "specify a (p, T) tabulation file with the 'fluid_property_file' parameter");
      _console << name() + ": Generating (p, T) tabulated data\n";
      _console << std::flush;

      generateTabulatedData();
    }
  }

  if (_create_direct_ve_interpolations)
  {
    // If the user specified a (v, e) tabulation to read, use that
    if (!_file_name_ve_in.empty())
      readFileTabulationData(false);
    else
    {
      if (!_fp)
        paramError(
            "create_ve_interpolations",
            "No FluidProperties (specified with 'fp' parameter) exists. Either specify a 'fp' or "
            "specify a (v, e) tabulation file with the 'fluid_property_ve_file' parameter");
      _console << name() + ": Generating (v, e) tabulated data\n";
      _console << std::flush;

      generateVETabulatedData();
    }
  }

  computePropertyIndicesInInterpolationVectors();
  constructInterpolation();

  // Write tabulated data to file
  if (_save_file)
  {
    _console << name() + ": Writing tabulated data to " << _file_name_out << "\n";
    writeTabulatedData(_file_name_out);
  }
}

std::string
TabulatedFluidProperties::fluidName() const
{
  if (_fp)
    return _fp->fluidName();
  else
    return "TabulationFromFile";
}

Real
TabulatedFluidProperties::molarMass() const
{
  if (_fp)
    return _fp->molarMass();
  else
    FluidPropertiesForwardError("molarMass");
}

Real
TabulatedFluidProperties::v_from_p_T(Real pressure, Real temperature) const
{
  if (_interpolate_density && _create_direct_pT_interpolations)
  {
    checkInputVariables(pressure, temperature);
    return 1.0 / _property_ipol[_density_idx]->sample(pressure, temperature);
  }
  else
  {
    if (_fp)
      return 1.0 / _fp->rho_from_p_T(pressure, temperature);
    else
      paramError("fp", "No fluid properties or csv data provided for density.");
  }
}

void
TabulatedFluidProperties::v_from_p_T(
    Real pressure, Real temperature, Real & v, Real & dv_dp, Real & dv_dT) const
{
  Real rho = 0, drho_dp = 0, drho_dT = 0;
  if (_interpolate_density && _create_direct_pT_interpolations)
  {
    checkInputVariables(pressure, temperature);
    _property_ipol[_density_idx]->sampleValueAndDerivatives(
        pressure, temperature, rho, drho_dp, drho_dT);
  }
  else
  {
    if (_fp)
      _fp->rho_from_p_T(pressure, temperature, rho, drho_dp, drho_dT);
    else
      paramError("fp", "No fluid properties or csv data provided for density.");
  }
  // convert from rho to v
  v = 1.0 / rho;
  dv_dp = -drho_dp / (rho * rho);
  dv_dT = -drho_dT / (rho * rho);
}

Real
TabulatedFluidProperties::rho_from_p_T(Real pressure, Real temperature) const
{
  if (_interpolate_density && _create_direct_pT_interpolations)
  {
    checkInputVariables(pressure, temperature);
    return _property_ipol[_density_idx]->sample(pressure, temperature);
  }
  else
  {
    if (_fp)
      return _fp->rho_from_p_T(pressure, temperature);
    else
      paramError("fp", "No fluid properties or csv data provided for density.");
  }
}

void
TabulatedFluidProperties::rho_from_p_T(
    Real pressure, Real temperature, Real & rho, Real & drho_dp, Real & drho_dT) const
{
  if (_interpolate_density && _create_direct_pT_interpolations)
  {
    checkInputVariables(pressure, temperature);
    _property_ipol[_density_idx]->sampleValueAndDerivatives(
        pressure, temperature, rho, drho_dp, drho_dT);
  }
  else
  {
    if (_fp)
      _fp->rho_from_p_T(pressure, temperature, rho, drho_dp, drho_dT);
    else
      paramError("fp", "No fluid properties or csv data provided for density.");
  }
}

void
TabulatedFluidProperties::rho_from_p_T(const ADReal & pressure,
                                       const ADReal & temperature,
                                       ADReal & rho,
                                       ADReal & drho_dp,
                                       ADReal & drho_dT) const
{
  if (_interpolate_density && _create_direct_pT_interpolations)
  {
    ADReal p = pressure, T = temperature;
    checkInputVariables(p, T);
    _property_ipol[_density_idx]->sampleValueAndDerivatives(p, T, rho, drho_dp, drho_dT);
  }
  else
  {
    if (_fp)
      _fp->rho_from_p_T(pressure, temperature, rho, drho_dp, drho_dT);
    else
      paramError("fp", "No fluid properties or csv data provided for density.");
  }
}

Real
TabulatedFluidProperties::rho_from_p_s(Real p, Real s) const
{
  Real T = T_from_p_s(p, s);
  return rho_from_p_T(p, T);
}

void
TabulatedFluidProperties::rho_from_p_s(
    Real p, Real s, Real & rho, Real & drho_dp, Real & drho_ds) const
{
  Real T, dT_dp, dT_ds;
  T_from_p_s(p, s, T, dT_dp, dT_ds);
  Real drho_dp_T, drho_dT;
  rho_from_p_T(p, T, rho, drho_dp_T, drho_dT);
  drho_dp = drho_dT * dT_dp + drho_dp_T;
  drho_ds = drho_dT * dT_ds;
}

Real
TabulatedFluidProperties::e_from_p_T(Real pressure, Real temperature) const
{
  if (_interpolate_internal_energy && _create_direct_pT_interpolations)
  {
    checkInputVariables(pressure, temperature);
    return _property_ipol[_internal_energy_idx]->sample(pressure, temperature);
  }
  else
  {
    if (_fp)
      return _fp->e_from_p_T(pressure, temperature);
    else
      paramError("fp", "No fluid properties or csv data provided for internal energy.");
  }
}

void
TabulatedFluidProperties::e_from_p_T(
    Real pressure, Real temperature, Real & e, Real & de_dp, Real & de_dT) const
{
  if (_interpolate_internal_energy && _create_direct_pT_interpolations)
  {
    checkInputVariables(pressure, temperature);
    _property_ipol[_internal_energy_idx]->sampleValueAndDerivatives(
        pressure, temperature, e, de_dp, de_dT);
  }
  else
  {
    if (_fp)
      _fp->e_from_p_T(pressure, temperature, e, de_dp, de_dT);
    else
      paramError("fp", "No fluid properties or csv data provided for internal energy.");
  }
}

Real
TabulatedFluidProperties::e_from_p_rho(Real pressure, Real rho) const
{
  Real T = T_from_p_rho(pressure, rho);
  Real e = e_from_p_T(pressure, T);
  return e;
}

void
TabulatedFluidProperties::e_from_p_rho(
    Real pressure, Real rho, Real & e, Real & de_dp, Real & de_drho) const
{
  // get derivatives of T wrt to pressure and density
  Real T, dT_dp, dT_drho;
  T_from_p_rho(pressure, rho, T, dT_dp, dT_drho);

  // Get e, then derivatives of e wrt pressure and temperature
  Real de_dp_at_const_T, de_dT;
  e_from_p_T(pressure, T, e, de_dp_at_const_T, de_dT);

  // Get the derivatives of density wrt pressure and temperature
  Real rho_pT, drho_dp, drho_dT;
  rho_from_p_T(pressure, T, rho_pT, drho_dp, drho_dT);

  // derivatives of e wrt pressure and rho (what we want from e_from_p_rho)
  de_drho = de_dT * dT_drho;
  de_dp = de_dp_at_const_T - (de_drho * drho_dp);
}

Real
TabulatedFluidProperties::T_from_p_rho(Real pressure, Real rho) const
{
  auto lambda = [&](Real p, Real current_T, Real & new_rho, Real & drho_dp, Real & drho_dT)
  { rho_from_p_T(p, current_T, new_rho, drho_dp, drho_dT); };
  Real T = FluidPropertiesUtils::NewtonSolve(pressure,
                                             rho,
                                             _T_initial_guess,
                                             _tolerance,
                                             lambda,
                                             name() + "::T_from_p_rho",
                                             _max_newton_its)
               .first;
  // check for nans
  if (std::isnan(T))
    mooseError("Conversion from pressure (p = ",
               pressure,
               ") and density (rho = ",
               rho,
               ") to temperature failed to converge.");
  return T;
}

void
TabulatedFluidProperties::T_from_p_rho(
    Real pressure, Real rho, Real & T, Real & dT_dp, Real & dT_drho) const
{
  T = T_from_p_rho(pressure, rho);
  Real eps = 1e-8;
  dT_dp = (T_from_p_rho(pressure * (1 + eps), rho) - T) / (eps * pressure);
  dT_drho = (T_from_p_rho(pressure, rho * (1 + eps)) - T) / (eps * rho);
}

Real
TabulatedFluidProperties::T_from_p_s(Real pressure, Real s) const
{
  auto lambda = [&](Real p, Real current_T, Real & new_s, Real & ds_dp, Real & ds_dT)
  { s_from_p_T(p, current_T, new_s, ds_dp, ds_dT); };
  Real T = FluidPropertiesUtils::NewtonSolve(pressure,
                                             s,
                                             _T_initial_guess,
                                             _tolerance,
                                             lambda,
                                             name() + "::T_from_p_s",
                                             _max_newton_its)
               .first;
  // check for nans
  if (std::isnan(T))
    mooseError("Conversion from pressure (p = ",
               pressure,
               ") and entropy (s = ",
               s,
               ") to temperature failed to converge.");
  return T;
}

void
TabulatedFluidProperties::T_from_p_s(
    Real pressure, Real s, Real & T, Real & dT_dp, Real & dT_ds) const
{
  T = T_from_p_s(pressure, s);
  Real eps = 1e-8;
  dT_dp = (T_from_p_s(pressure * (1 + eps), s) - T) / (eps * pressure);
  dT_ds = (T_from_p_s(pressure, s * (1 + eps)) - T) / (eps * s);
}

Real
TabulatedFluidProperties::h_from_p_T(Real pressure, Real temperature) const
{
  if (_interpolate_enthalpy && _create_direct_pT_interpolations)
  {
    checkInputVariables(pressure, temperature);
    return _property_ipol[_enthalpy_idx]->sample(pressure, temperature);
  }
  else
  {
    if (_fp)
      return _fp->h_from_p_T(pressure, temperature);
    else
      paramError("fp", "No fluid properties or csv data provided for enthalpy.");
  }
}

ADReal
TabulatedFluidProperties::h_from_p_T(const ADReal & pressure, const ADReal & temperature) const
{
  if (_fp) // Assuming _fp can handle ADReal types
    return _fp->h_from_p_T(pressure, temperature);
  else
    FluidPropertiesForwardError("h_from_p_T");
}

void
TabulatedFluidProperties::h_from_p_T(
    Real pressure, Real temperature, Real & h, Real & dh_dp, Real & dh_dT) const
{
  if (_interpolate_enthalpy && _create_direct_pT_interpolations)
  {
    checkInputVariables(pressure, temperature);
    _property_ipol[_enthalpy_idx]->sampleValueAndDerivatives(
        pressure, temperature, h, dh_dp, dh_dT);
  }
  else
  {
    if (_fp)
      _fp->h_from_p_T(pressure, temperature, h, dh_dp, dh_dT);
    else
      paramError("fp", "No fluid properties or csv data provided for enthalpy.");
  }
}

Real
TabulatedFluidProperties::mu_from_p_T(Real pressure, Real temperature) const
{
  if (_interpolate_viscosity && _create_direct_pT_interpolations)
  {
    checkInputVariables(pressure, temperature);
    return _property_ipol[_viscosity_idx]->sample(pressure, temperature);
  }
  else
  {
    if (_fp)
      return _fp->mu_from_p_T(pressure, temperature);
    else
      paramError("fp", "No fluid properties or csv data provided for viscosity.");
  }
}

void
TabulatedFluidProperties::mu_from_p_T(
    Real pressure, Real temperature, Real & mu, Real & dmu_dp, Real & dmu_dT) const
{
  if (_interpolate_viscosity && _create_direct_pT_interpolations)
  {
    checkInputVariables(pressure, temperature);
    _property_ipol[_viscosity_idx]->sampleValueAndDerivatives(
        pressure, temperature, mu, dmu_dp, dmu_dT);
  }
  else
  {
    if (_fp)
      _fp->mu_from_p_T(pressure, temperature, mu, dmu_dp, dmu_dT);
    else
      paramError("fp", "No fluid properties or csv data provided for viscosity.");
  }
}

Real
TabulatedFluidProperties::c_from_p_T(Real pressure, Real temperature) const
{
  if (_interpolate_c && _create_direct_pT_interpolations)
  {
    checkInputVariables(pressure, temperature);
    return _property_ipol[_c_idx]->sample(pressure, temperature);
  }
  else
  {
    if (_fp)
      return _fp->c_from_p_T(pressure, temperature);
    else
      paramError("interpolated_properties", "No data to interpolate for speed of sound.");
  }
}

void
TabulatedFluidProperties::c_from_p_T(
    Real pressure, Real temperature, Real & c, Real & dc_dp, Real & dc_dT) const
{
  if (_interpolate_c && _create_direct_pT_interpolations)
  {
    checkInputVariables(pressure, temperature);
    _property_ipol[_c_idx]->sampleValueAndDerivatives(pressure, temperature, c, dc_dp, dc_dT);
  }
  else
  {
    if (_fp)
      _fp->c_from_p_T(pressure, temperature, c, dc_dp, dc_dT);
    else
      paramError("interpolated_properties", "No data to interpolate for speed of sound.");
  }
}

Real
TabulatedFluidProperties::cp_from_p_T(Real pressure, Real temperature) const
{
  if (_interpolate_cp && _create_direct_pT_interpolations)
  {
    checkInputVariables(pressure, temperature);
    return _property_ipol[_cp_idx]->sample(pressure, temperature);
  }
  else
  {
    if (_fp)
      return _fp->cp_from_p_T(pressure, temperature);
    else
      paramError("interpolated_properties",
                 "No data to interpolate for specific heat capacity at constant pressure.");
  }
}

void
TabulatedFluidProperties::cp_from_p_T(
    Real pressure, Real temperature, Real & cp, Real & dcp_dp, Real & dcp_dT) const
{
  if (_interpolate_cp && _create_direct_pT_interpolations)
  {
    checkInputVariables(pressure, temperature);
    _property_ipol[_cp_idx]->sampleValueAndDerivatives(pressure, temperature, cp, dcp_dp, dcp_dT);
  }
  else
  {
    if (_fp)
      _fp->cp_from_p_T(pressure, temperature, cp, dcp_dp, dcp_dT);
    else
      paramError("interpolated_properties",
                 "No data to interpolate for specific heat capacity at constant pressure.");
  }
}

Real
TabulatedFluidProperties::cv_from_p_T(Real pressure, Real temperature) const
{
  if (_interpolate_cv && _create_direct_pT_interpolations)
  {
    checkInputVariables(pressure, temperature);
    return _property_ipol[_cv_idx]->sample(pressure, temperature);
  }
  else
  {
    if (_fp)
      return _fp->cv_from_p_T(pressure, temperature);
    else
      paramError("interpolated_properties",
                 "No data to interpolate for specific heat capacity at constant volume.");
  }
}

void
TabulatedFluidProperties::cv_from_p_T(
    Real pressure, Real temperature, Real & cv, Real & dcv_dp, Real & dcv_dT) const
{
  if (_interpolate_cv)
  {
    checkInputVariables(pressure, temperature);
    _property_ipol[_cv_idx]->sampleValueAndDerivatives(pressure, temperature, cv, dcv_dp, dcv_dT);
  }
  else
  {
    if (_fp)
      _fp->cv_from_p_T(pressure, temperature, cv, dcv_dp, dcv_dT);
    else
      paramError("interpolated_properties",
                 "No data to interpolate for specific heat capacity at constant volume.");
  }
}

Real
TabulatedFluidProperties::k_from_p_T(Real pressure, Real temperature) const
{
  if (_interpolate_k && _create_direct_pT_interpolations)
  {
    checkInputVariables(pressure, temperature);
    return _property_ipol[_k_idx]->sample(pressure, temperature);
  }
  else
  {
    if (_fp)
      return _fp->k_from_p_T(pressure, temperature);
    else
      paramError("interpolated_properties", "No data to interpolate for thermal conductivity.");
  }
}

void
TabulatedFluidProperties::k_from_p_T(
    Real pressure, Real temperature, Real & k, Real & dk_dp, Real & dk_dT) const
{
  if (_interpolate_k && _create_direct_pT_interpolations)
  {
    checkInputVariables(pressure, temperature);
    return _property_ipol[_k_idx]->sampleValueAndDerivatives(
        pressure, temperature, k, dk_dp, dk_dT);
  }
  else
  {
    if (_fp)
      return _fp->k_from_p_T(pressure, temperature, k, dk_dp, dk_dT);
    else
      paramError("interpolated_properties", "No data to interpolate for thermal conductivity.");
  }
}

Real
TabulatedFluidProperties::s_from_p_T(Real pressure, Real temperature) const
{
  if (_interpolate_entropy && _create_direct_pT_interpolations)
  {
    checkInputVariables(pressure, temperature);
    return _property_ipol[_entropy_idx]->sample(pressure, temperature);
  }
  else
  {
    if (_fp)
      return _fp->s_from_p_T(pressure, temperature);
    else
      paramError("interpolated_properties", "No data to interpolate for entropy.");
  }
}

void
TabulatedFluidProperties::s_from_p_T(Real p, Real T, Real & s, Real & ds_dp, Real & ds_dT) const
{
  if (_interpolate_entropy && _create_direct_pT_interpolations)
  {
    checkInputVariables(p, T);
    _property_ipol[_entropy_idx]->sampleValueAndDerivatives(p, T, s, ds_dp, ds_dT);
  }
  else
  {
    if (_fp)
      _fp->s_from_p_T(p, T, s, ds_dp, ds_dT);
    else
      paramError("interpolated_properties", "No data to interpolate for entropy.");
  }
}

Real
TabulatedFluidProperties::e_from_v_h(Real v, Real h) const
{
  if (_construct_pT_from_vh)
  {
    const Real p = _p_from_v_h_ipol->sample(v, h);
    const Real T = _T_from_v_h_ipol->sample(v, h);
    return e_from_p_T(p, T);
  }
  else if (_create_direct_ve_interpolations)
  {
    // Lambda computes h from v and the current_e
    auto lambda = [&](Real v, Real current_e, Real & new_h, Real & dh_dv, Real & dh_de)
    { h_from_v_e(v, current_e, new_h, dh_dv, dh_de); };
    Real e = FluidPropertiesUtils::NewtonSolve(v,
                                               h,
                                               /*e initial guess*/ h - _p_initial_guess * v,
                                               _tolerance,
                                               lambda,
                                               name() + "::e_from_v_h",
                                               _max_newton_its)
                 .first;
    return e;
  }
  else if (_fp)
    return _fp->e_from_v_h(v, h);
  else
    mooseError(__PRETTY_FUNCTION__,
               "\nNo tabulation or fluid property 'fp' object to compute value");
}

void
TabulatedFluidProperties::e_from_v_h(Real v, Real h, Real & e, Real & de_dv, Real & de_dh) const
{
  if (_construct_pT_from_vh)
  {
    Real p = 0, dp_dv = 0, dp_dh = 0;
    _p_from_v_h_ipol->sampleValueAndDerivatives(v, h, p, dp_dv, dp_dh);
    Real T = 0, dT_dv = 0, dT_dh = 0;
    _T_from_v_h_ipol->sampleValueAndDerivatives(v, h, T, dT_dv, dT_dh);
    Real de_dp, de_dT;
    e_from_p_T(p, T, e, de_dp, de_dT);
    de_dv = de_dp * dp_dv + de_dT * dT_dv;
    de_dh = de_dp * dp_dh + de_dT * dT_dh;
  }
  else if (_create_direct_ve_interpolations)
  {
    // Lambda computes h from v and the current_e
    auto lambda = [&](Real v, Real current_e, Real & new_h, Real & dh_dv, Real & dh_de)
    { h_from_v_e(v, current_e, new_h, dh_dv, dh_de); };
    const auto e_data =
        FluidPropertiesUtils::NewtonSolve(v,
                                          h,
                                          /*e initial guess*/ h - _p_initial_guess * v,
                                          _tolerance,
                                          lambda,
                                          name() + "::e_from_v_h",
                                          _max_newton_its);
    e = e_data.first;
    // Finite difference approximation
    const auto e2 = e_from_v_h(v * (1 + TOLERANCE), h);
    de_dv = (e2 - e) / (TOLERANCE * v);
    de_dh = 1. / e_data.second;
  }
  else if (_fp)
    _fp->e_from_v_h(v, h, e, de_dv, de_dh);
  else
    mooseError(__PRETTY_FUNCTION__,
               "\nNo tabulation or fluid property 'fp' object to compute value");
}

std::vector<Real>
TabulatedFluidProperties::henryCoefficients() const
{
  if (_fp)
    return _fp->henryCoefficients();
  else
    FluidPropertiesForwardError("henryCoefficients");
}

Real
TabulatedFluidProperties::vaporPressure(Real temperature) const
{
  if (_fp)
    return _fp->vaporPressure(temperature);
  else
    FluidPropertiesForwardError("vaporPressure");
}

void
TabulatedFluidProperties::vaporPressure(Real temperature, Real & psat, Real & dpsat_dT) const
{
  if (_fp)
    _fp->vaporPressure(temperature, psat, dpsat_dT);
  else
    FluidPropertiesForwardError("vaporPressure");
}

Real
TabulatedFluidProperties::vaporTemperature(Real pressure) const
{
  if (_fp)
    return _fp->vaporTemperature(pressure);
  else
    FluidPropertiesForwardError("vaporTemperature");
}

void
TabulatedFluidProperties::vaporTemperature(Real pressure, Real & Tsat, Real & dTsat_dp) const
{

  if (_fp)
    _fp->vaporTemperature(pressure, Tsat, dTsat_dp);
  else
    FluidPropertiesForwardError("vaporTemperature");
}

Real
TabulatedFluidProperties::triplePointPressure() const
{

  if (_fp)
    return _fp->triplePointPressure();
  else
    FluidPropertiesForwardError("triplePointPressure");
}

Real
TabulatedFluidProperties::triplePointTemperature() const
{

  if (_fp)
    return _fp->triplePointTemperature();
  else
    FluidPropertiesForwardError("triplePointTemperature");
}

Real
TabulatedFluidProperties::criticalPressure() const
{

  if (_fp)
    return _fp->criticalPressure();
  else
    FluidPropertiesForwardError("criticalPressure");
}

Real
TabulatedFluidProperties::criticalTemperature() const
{

  if (_fp)
    return _fp->criticalTemperature();
  else
    FluidPropertiesForwardError("criticalTemperature");
}

Real
TabulatedFluidProperties::criticalDensity() const
{
  if (_fp)
    return _fp->criticalDensity();
  else
    FluidPropertiesForwardError("criticalDensity");
}

Real
TabulatedFluidProperties::p_from_v_e(Real v, Real e) const
{
  if (_interpolate_pressure && !_construct_pT_from_ve && !_create_direct_ve_interpolations)
    missingVEInterpolationError(__PRETTY_FUNCTION__);
  checkInputVariablesVE(v, e);

  if (_create_direct_ve_interpolations && _interpolate_pressure)
    return _property_ve_ipol[_p_idx]->sample(v, e);
  else if (_construct_pT_from_ve)
    return _p_from_v_e_ipol->sample(v, e);
  else if (_fp)
    return _fp->p_from_v_e(v, e);
  else
    mooseError(__PRETTY_FUNCTION__,
               "\nNo tabulation or fluid property 'fp' object to compute value");
}

void
TabulatedFluidProperties::p_from_v_e(Real v, Real e, Real & p, Real & dp_dv, Real & dp_de) const
{
  if (_interpolate_pressure && !_construct_pT_from_ve && !_create_direct_ve_interpolations)
    missingVEInterpolationError(__PRETTY_FUNCTION__);
  checkInputVariablesVE(v, e);

  if (_create_direct_ve_interpolations && _interpolate_pressure)
    _property_ve_ipol[_p_idx]->sampleValueAndDerivatives(v, e, p, dp_dv, dp_de);
  else if (_construct_pT_from_ve)
    _p_from_v_e_ipol->sampleValueAndDerivatives(v, e, p, dp_dv, dp_de);
  else if (_fp)
    _fp->p_from_v_e(v, e, p, dp_dv, dp_de);
  else
    mooseError(__PRETTY_FUNCTION__,
               "\nNo tabulation or fluid property 'fp' object to compute value");
}

Real
TabulatedFluidProperties::T_from_v_e(Real v, Real e) const
{
  if (_interpolate_temperature && !_construct_pT_from_ve && !_create_direct_ve_interpolations)
    missingVEInterpolationError(__PRETTY_FUNCTION__);
  checkInputVariablesVE(v, e);

  if (_create_direct_ve_interpolations && _interpolate_temperature)
    return _property_ve_ipol[_T_idx]->sample(v, e);
  else if (_construct_pT_from_ve)
    return _T_from_v_e_ipol->sample(v, e);
  else if (_fp)
    return _fp->T_from_v_e(v, e);
  else
    mooseError(__PRETTY_FUNCTION__,
               "\nNo tabulation or fluid property 'fp' object to compute value");
}

void
TabulatedFluidProperties::T_from_v_e(Real v, Real e, Real & T, Real & dT_dv, Real & dT_de) const
{
  if (_interpolate_temperature && !_construct_pT_from_ve && !_create_direct_ve_interpolations)
    missingVEInterpolationError(__PRETTY_FUNCTION__);
  checkInputVariablesVE(v, e);

  if (_create_direct_ve_interpolations && _interpolate_temperature)
    _property_ve_ipol[_T_idx]->sampleValueAndDerivatives(v, e, T, dT_dv, dT_de);
  else if (_construct_pT_from_ve)
    _T_from_v_e_ipol->sampleValueAndDerivatives(v, e, T, dT_dv, dT_de);
  else if (_fp)
    _fp->T_from_v_e(v, e, T, dT_dv, dT_de);
  else
    mooseError(__PRETTY_FUNCTION__,
               "\nNo tabulation or fluid property 'fp' object to compute value");
}

Real
TabulatedFluidProperties::c_from_v_e(Real v, Real e) const
{
  if (_interpolate_c && !_construct_pT_from_ve && !_create_direct_ve_interpolations)
    missingVEInterpolationError(__PRETTY_FUNCTION__);
  checkInputVariablesVE(v, e);

  if (_create_direct_ve_interpolations && _interpolate_c)
    return _property_ve_ipol[_c_idx]->sample(v, e);
  else if (_construct_pT_from_ve)
  {
    Real p = _p_from_v_e_ipol->sample(v, e);
    Real T = _T_from_v_e_ipol->sample(v, e);
    return c_from_p_T(p, T);
  }
  else if (_fp)
    return _fp->c_from_v_e(v, e);
  else
    mooseError(__PRETTY_FUNCTION__,
               "\nNo tabulation or fluid property 'fp' object to compute value");
}

void
TabulatedFluidProperties::c_from_v_e(Real v, Real e, Real & c, Real & dc_dv, Real & dc_de) const
{
  if (_interpolate_c && !_construct_pT_from_ve && !_create_direct_ve_interpolations)
    missingVEInterpolationError(__PRETTY_FUNCTION__);
  checkInputVariablesVE(v, e);

  if (_create_direct_ve_interpolations && _interpolate_c)
    _property_ve_ipol[_c_idx]->sampleValueAndDerivatives(v, e, c, dc_dv, dc_de);
  else if (_construct_pT_from_ve)
  {
    Real p, dp_dv, dp_de;
    _p_from_v_e_ipol->sampleValueAndDerivatives(v, e, p, dp_dv, dp_de);
    Real T, dT_dv, dT_de;
    _T_from_v_e_ipol->sampleValueAndDerivatives(v, e, T, dT_dv, dT_de);
    Real dc_dp, dc_dT;
    c_from_p_T(p, T, c, dc_dp, dc_dT);
    dc_dv = dc_dp * dp_dv + dc_dT * dT_dv;
    dc_de = dc_dp * dp_de + dc_dT * dT_de;
  }
  else if (_fp)
    _fp->c_from_v_e(v, e, c, dc_dv, dc_de);
  else
    mooseError(__PRETTY_FUNCTION__,
               "\nNo tabulation or fluid property 'fp' object to compute value");
}

Real
TabulatedFluidProperties::cp_from_v_e(Real v, Real e) const
{
  if (_interpolate_cp && !_construct_pT_from_ve && !_create_direct_ve_interpolations)
    missingVEInterpolationError(__PRETTY_FUNCTION__);
  checkInputVariablesVE(v, e);

  if (_create_direct_ve_interpolations && _interpolate_cp)
    return _property_ve_ipol[_cp_idx]->sample(v, e);
  else if (_construct_pT_from_ve)
  {
    Real p = _p_from_v_e_ipol->sample(v, e);
    Real T = _T_from_v_e_ipol->sample(v, e);
    return cp_from_p_T(p, T);
  }
  else if (_fp)
    return _fp->cp_from_v_e(v, e);
  else
    mooseError(__PRETTY_FUNCTION__,
               "\nNo tabulation or fluid property 'fp' object to compute value");
}

void
TabulatedFluidProperties::cp_from_v_e(Real v, Real e, Real & cp, Real & dcp_dv, Real & dcp_de) const
{
  if (_interpolate_cp && !_construct_pT_from_ve && !_create_direct_ve_interpolations)
    missingVEInterpolationError(__PRETTY_FUNCTION__);
  checkInputVariablesVE(v, e);

  if (_create_direct_ve_interpolations && _interpolate_cp)
    _property_ve_ipol[_cp_idx]->sampleValueAndDerivatives(v, e, cp, dcp_dv, dcp_de);
  else if (_construct_pT_from_ve)
  {
    Real p, dp_dv, dp_de;
    _p_from_v_e_ipol->sampleValueAndDerivatives(v, e, p, dp_dv, dp_de);
    Real T, dT_dv, dT_de;
    _T_from_v_e_ipol->sampleValueAndDerivatives(v, e, T, dT_dv, dT_de);
    Real dcp_dp, dcp_dT;
    cp_from_p_T(p, T, cp, dcp_dp, dcp_dT);
    dcp_dv = dcp_dp * dp_dv + dcp_dT * dT_dv;
    dcp_de = dcp_dp * dp_de + dcp_dT * dT_de;
  }
  else if (_fp)
    _fp->cp_from_v_e(v, e, cp, dcp_dv, dcp_de);
  else
    mooseError(__PRETTY_FUNCTION__,
               "\nNo tabulation or fluid property 'fp' object to compute value");
}

Real
TabulatedFluidProperties::cv_from_v_e(Real v, Real e) const
{
  if (_interpolate_cv && !_construct_pT_from_ve && !_create_direct_ve_interpolations)
    missingVEInterpolationError(__PRETTY_FUNCTION__);
  checkInputVariablesVE(v, e);

  if (_create_direct_ve_interpolations && _interpolate_cv)
    return _property_ve_ipol[_cv_idx]->sample(v, e);
  else if (_construct_pT_from_ve)
  {
    Real p = _p_from_v_e_ipol->sample(v, e);
    Real T = _T_from_v_e_ipol->sample(v, e);
    return cv_from_p_T(p, T);
  }
  else if (_fp)
    return _fp->cv_from_v_e(v, e);
  else
    mooseError(__PRETTY_FUNCTION__,
               "\nNo tabulation or fluid property 'fp' object to compute value");
}

void
TabulatedFluidProperties::cv_from_v_e(Real v, Real e, Real & cv, Real & dcv_dv, Real & dcv_de) const
{
  if (_interpolate_cv && !_construct_pT_from_ve && !_create_direct_ve_interpolations)
    missingVEInterpolationError(__PRETTY_FUNCTION__);
  checkInputVariablesVE(v, e);

  if (_create_direct_ve_interpolations && _interpolate_cv)
    _property_ve_ipol[_cv_idx]->sampleValueAndDerivatives(v, e, cv, dcv_dv, dcv_de);
  else if (_construct_pT_from_ve)
  {
    Real p, dp_dv, dp_de;
    _p_from_v_e_ipol->sampleValueAndDerivatives(v, e, p, dp_dv, dp_de);
    Real T, dT_dv, dT_de;
    _T_from_v_e_ipol->sampleValueAndDerivatives(v, e, T, dT_dv, dT_de);
    Real dcv_dp, dcv_dT;
    cv_from_p_T(p, T, cv, dcv_dp, dcv_dT);
    dcv_dv = dcv_dp * dp_dv + dcv_dT * dT_dv;
    dcv_de = dcv_dp * dp_de + dcv_dT * dT_de;
  }
  else if (_fp)
    _fp->cv_from_v_e(v, e, cv, dcv_dv, dcv_de);
  else
    mooseError(__PRETTY_FUNCTION__,
               "\nNo tabulation or fluid property 'fp' object to compute value");
}

Real
TabulatedFluidProperties::mu_from_v_e(Real v, Real e) const
{
  if (_interpolate_viscosity && !_construct_pT_from_ve && !_create_direct_ve_interpolations)
    missingVEInterpolationError(__PRETTY_FUNCTION__);
  checkInputVariablesVE(v, e);

  if (_create_direct_ve_interpolations && _interpolate_viscosity)
    return _property_ve_ipol[_viscosity_idx]->sample(v, e);
  else if (_construct_pT_from_ve)
  {
    Real p = _p_from_v_e_ipol->sample(v, e);
    Real T = _T_from_v_e_ipol->sample(v, e);
    return mu_from_p_T(p, T);
  }
  else if (_fp)
    return _fp->mu_from_v_e(v, e);
  else
    mooseError(__PRETTY_FUNCTION__,
               "\nNo tabulation or fluid property 'fp' object to compute value");
}

void
TabulatedFluidProperties::mu_from_v_e(Real v, Real e, Real & mu, Real & dmu_dv, Real & dmu_de) const
{
  if (_interpolate_viscosity & !_construct_pT_from_ve && !_create_direct_ve_interpolations)
    missingVEInterpolationError(__PRETTY_FUNCTION__);
  checkInputVariablesVE(v, e);

  if (_create_direct_ve_interpolations && _interpolate_viscosity)
    _property_ve_ipol[_viscosity_idx]->sampleValueAndDerivatives(v, e, mu, dmu_dv, dmu_de);
  else if (_construct_pT_from_ve)
  {
    Real p, dp_dv, dp_de;
    _p_from_v_e_ipol->sampleValueAndDerivatives(v, e, p, dp_dv, dp_de);
    Real T, dT_dv, dT_de;
    _T_from_v_e_ipol->sampleValueAndDerivatives(v, e, T, dT_dv, dT_de);
    Real dmu_dp, dmu_dT;
    mu_from_p_T(p, T, mu, dmu_dp, dmu_dT);
    dmu_dv = dmu_dp * dp_dv + dmu_dT * dT_dv;
    dmu_de = dmu_dp * dp_de + dmu_dT * dT_de;
  }
  else if (_fp)
    _fp->mu_from_v_e(v, e, mu, dmu_dv, dmu_de);
  else
    mooseError(__PRETTY_FUNCTION__,
               "\nNo tabulation or fluid property 'fp' object to compute value");
}

Real
TabulatedFluidProperties::k_from_v_e(Real v, Real e) const
{
  if (_interpolate_k && !_construct_pT_from_ve && !_create_direct_ve_interpolations)
    missingVEInterpolationError(__PRETTY_FUNCTION__);
  checkInputVariablesVE(v, e);

  if (_create_direct_ve_interpolations && _interpolate_k)
    return _property_ve_ipol[_k_idx]->sample(v, e);
  else if (_construct_pT_from_ve)
  {
    Real T = _T_from_v_e_ipol->sample(v, e);
    Real p = _p_from_v_e_ipol->sample(v, e);
    return k_from_p_T(p, T);
  }
  else if (_fp)
    return _fp->k_from_v_e(v, e);
  else
    mooseError(__PRETTY_FUNCTION__,
               "\nNo tabulation or fluid property 'fp' object to compute value");
}

void
TabulatedFluidProperties::k_from_v_e(Real v, Real e, Real & k, Real & dk_dv, Real & dk_de) const
{
  if (_interpolate_k && !_construct_pT_from_ve && !_create_direct_ve_interpolations)
    missingVEInterpolationError(__PRETTY_FUNCTION__);
  checkInputVariablesVE(v, e);

  if (_create_direct_ve_interpolations && _interpolate_k)
    _property_ve_ipol[_k_idx]->sampleValueAndDerivatives(v, e, k, dk_dv, dk_de);
  else if (_construct_pT_from_ve)
  {
    Real p, dp_dv, dp_de;
    _p_from_v_e_ipol->sampleValueAndDerivatives(v, e, p, dp_dv, dp_de);
    Real T, dT_dv, dT_de;
    _T_from_v_e_ipol->sampleValueAndDerivatives(v, e, T, dT_dv, dT_de);
    Real dk_dp, dk_dT;
    k_from_p_T(p, T, k, dk_dp, dk_dT);
    dk_dv = dk_dp * dp_dv + dk_dT * dT_dv;
    dk_de = dk_dp * dp_de + dk_dT * dT_de;
  }
  else if (_fp)
    _fp->k_from_v_e(v, e, k, dk_dv, dk_de);
  else
    mooseError(__PRETTY_FUNCTION__,
               "\nNo tabulation or fluid property 'fp' object to compute value");
}

Real
TabulatedFluidProperties::s_from_v_e(Real v, Real e) const
{
  if (_interpolate_entropy && !_construct_pT_from_ve && !_create_direct_ve_interpolations)
    missingVEInterpolationError(__PRETTY_FUNCTION__);
  checkInputVariablesVE(v, e);

  if (_create_direct_ve_interpolations && _interpolate_entropy)
    return _property_ve_ipol[_entropy_idx]->sample(v, e);
  else if (_construct_pT_from_ve)
  {
    Real T = _T_from_v_e_ipol->sample(v, e);
    Real p = _p_from_v_e_ipol->sample(v, e);
    return s_from_p_T(p, T);
  }
  else if (_fp)
    return _fp->s_from_v_e(v, e);
  else
    mooseError(__PRETTY_FUNCTION__,
               "\nNo tabulation or fluid property 'fp' object to compute value");
}

Real
TabulatedFluidProperties::g_from_v_e(Real v, Real e) const
{
  if (!_construct_pT_from_ve &&
      (!_create_direct_ve_interpolations || _entropy_idx == libMesh::invalid_uint ||
       _enthalpy_idx == libMesh::invalid_uint || _T_idx == libMesh::invalid_uint))
    missingVEInterpolationError(__PRETTY_FUNCTION__);
  checkInputVariablesVE(v, e);

  Real h, T = 0, s;
  if (_create_direct_ve_interpolations)
  {
    s = _property_ve_ipol[_entropy_idx]->sample(v, e);
    h = _property_ve_ipol[_enthalpy_idx]->sample(v, e);
    T = _property_ve_ipol[_T_idx]->sample(v, e);
  }
  else if (_fp || _create_direct_pT_interpolations)
  {
    Real p0 = _p_initial_guess;
    Real T0 = _T_initial_guess;
    Real p = 0;
    bool conversion_succeeded;
    p_T_from_v_e(v, e, p0, T0, p, T, conversion_succeeded);
    s = s_from_p_T(p, T);
    h = h_from_p_T(p, T);
  }
  else
    mooseError(__PRETTY_FUNCTION__,
               "\nNo tabulation or fluid property 'fp' object to compute value");
  return h - T * s;
}

Real
TabulatedFluidProperties::T_from_h_s(Real h, Real s) const
{
  Real p0 = _p_initial_guess;
  Real T0 = _T_initial_guess;
  Real p, T;
  bool conversion_succeeded;
  p_T_from_h_s(h, s, p0, T0, p, T, conversion_succeeded);
  return T;
}

Real
TabulatedFluidProperties::T_from_p_h(Real pressure, Real enthalpy) const
{
  if (_fp)
    return _fp->T_from_p_h(pressure, enthalpy);
  else
  {
    auto lambda = [&](Real pressure, Real current_T, Real & new_h, Real & dh_dp, Real & dh_dT)
    { h_from_p_T(pressure, current_T, new_h, dh_dp, dh_dT); };
    Real T = FluidPropertiesUtils::NewtonSolve(
                 pressure, enthalpy, _T_initial_guess, _tolerance, lambda, name() + "::T_from_p_h")
                 .first;
    // check for nans
    if (std::isnan(T))
      mooseError("Conversion from enthalpy (h = ",
                 enthalpy,
                 ") and pressure (p = ",
                 pressure,
                 ") to temperature failed to converge.");
    return T;
  }
}

ADReal
TabulatedFluidProperties::T_from_p_h(const ADReal & pressure, const ADReal & enthalpy) const
{
  if (_fp)
    return _fp->T_from_p_h(pressure, enthalpy);
  else
  {
    auto lambda =
        [&](ADReal pressure, ADReal current_T, ADReal & new_h, ADReal & dh_dp, ADReal & dh_dT)
    {
      h_from_p_T(pressure.value(), current_T.value(), new_h.value(), dh_dp.value(), dh_dT.value());
      // Reconstruct derivatives
      new_h.derivatives() =
          dh_dp.value() * pressure.derivatives() + dh_dT.value() * current_T.derivatives();
    };
    ADReal T =
        FluidPropertiesUtils::NewtonSolve(
            pressure, enthalpy, _T_initial_guess, _tolerance, lambda, name() + "::T_from_p_h")
            .first;
    // check for nans
    if (std::isnan(T))
      mooseError("Conversion from enthalpy (h = ",
                 enthalpy,
                 ") and pressure (p = ",
                 pressure,
                 ") to temperature failed to converge.");
    return T;
  }
}

Real
TabulatedFluidProperties::s_from_h_p(Real enthalpy, Real pressure) const
{
  Real T = T_from_p_h(pressure, enthalpy);
  return s_from_p_T(pressure, T);
}

void
TabulatedFluidProperties::s_from_h_p(
    Real h, Real pressure, Real & s, Real & ds_dh, Real & ds_dp) const
{

  if (_fp)
    _fp->s_from_h_p(h, pressure, s, ds_dh, ds_dp);
  else
    mooseError("fp", "s_from_h_p derivatives not implemented.");
}

[[noreturn]] void
TabulatedFluidProperties::FluidPropertiesForwardError(const std::string & desired_routine) const
{
  mooseError("TabulatedFluidProperties can only call the function '" + desired_routine +
             "' when the 'fp' parameter is provided. It is currently not implemented using "
             "tabulations, and this property is simply forwarded to the FluidProperties specified "
             "in the 'fp' parameter");
}

void
TabulatedFluidProperties::writeTabulatedData(std::string file_name)
{
  file_name = file_name.empty() ? "fluid_properties_" + name() + "_out.csv" : file_name;
  if (processor_id() == 0)
  {
    {
      MooseUtils::checkFileWriteable(file_name);

      std::ofstream file_out(file_name.c_str());

      // Write out date and fluid type
      time_t now = std::time(&now);
      if (_fp)
        file_out << "# " << _fp->fluidName()
                 << " properties created by TabulatedFluidProperties on " << ctime(&now) << "\n";
      else
        file_out << "# tabulated properties created by TabulatedFluidProperties on " << ctime(&now)
                 << "\n";

      // Write out column names
      file_out << "pressure, temperature";
      for (std::size_t i = 0; i < _interpolated_properties.size(); ++i)
        file_out << ", " << _interpolated_properties[i];
      file_out << "\n";

      // Write out the fluid property data
      for (unsigned int p = 0; p < _num_p; ++p)
        for (unsigned int t = 0; t < _num_T; ++t)
        {
          file_out << _pressure[p] << ", " << _temperature[t];
          for (std::size_t i = 0; i < _properties.size(); ++i)
            file_out << ", " << _properties[i][p * _num_T + t];
          file_out << "\n";
        }

      file_out << std::flush;
      file_out.close();
    }

    // Write out the (v,e) to (p,T) conversions
    if (_construct_pT_from_ve)
    {
      const auto file_name_ve = (_file_name_ve_out == "")
                                    ? std::regex_replace(file_name, std::regex("\\.csv"), "_ve.csv")
                                    : _file_name_ve_out;
      MooseUtils::checkFileWriteable(file_name_ve);
      std::ofstream file_out(file_name_ve.c_str());

      // Write out column names
      file_out << "specific_volume, internal_energy, pressure, temperature";
      for (const auto i : index_range(_properties))
        if (_interpolated_properties[i] != "internal_energy")
          file_out << ", " << _interpolated_properties[i];
      file_out << "\n";

      // Write out the fluid property data
      for (const auto v : make_range(_num_v))
        for (const auto e : make_range(_num_e))
        {
          const auto v_val = _specific_volume[v];
          const auto e_val = _internal_energy[e];
          const auto pressure = _p_from_v_e_ipol->sample(v_val, e_val);
          const auto temperature = _T_from_v_e_ipol->sample(v_val, e_val);
          file_out << v_val << ", " << e_val << ", " << pressure << ", " << temperature << ", ";
          for (const auto i : index_range(_properties))
          {
            bool add_comma = true;
            if (i == _density_idx)
              file_out << 1 / v_val;
            else if (i == _enthalpy_idx)
              file_out << h_from_p_T(pressure, temperature);
            // Note that we could use (p,T) routine to generate this instead of (v,e)
            // Or could use the _properties_ve array
            else if (i == _viscosity_idx)
              file_out << mu_from_v_e(v_val, e_val);
            else if (i == _k_idx)
              file_out << k_from_v_e(v_val, e_val);
            else if (i == _c_idx)
              file_out << c_from_v_e(v_val, e_val);
            else if (i == _cv_idx)
              file_out << cv_from_v_e(v_val, e_val);
            else if (i == _cp_idx)
              file_out << cp_from_v_e(v_val, e_val);
            else if (i == _entropy_idx)
              file_out << s_from_v_e(v_val, e_val);
            else
              add_comma = false;
            if (i != _properties.size() - 1 && add_comma)
              file_out << ", ";
          }

          file_out << "\n";
        }

      file_out << std::flush;
      file_out.close();
    }
  }
}

void
TabulatedFluidProperties::generateTabulatedData()
{
  mooseAssert(_fp, "We should not try to generate (p,T) tabulated data without a _fp user object");
  _pressure.resize(_num_p);
  _temperature.resize(_num_T);

  // Generate data for all properties entered in input file
  _properties.resize(_interpolated_properties_enum.size());
  _interpolated_properties.resize(_interpolated_properties_enum.size());

  for (std::size_t i = 0; i < _interpolated_properties_enum.size(); ++i)
    _interpolated_properties[i] = _interpolated_properties_enum[i];

  for (const auto i : index_range(_properties))
    _properties[i].resize(_num_p * _num_T);

  // Temperature is divided equally into _num_T segments
  Real delta_T = (_temperature_max - _temperature_min) / static_cast<Real>(_num_T - 1);

  for (unsigned int j = 0; j < _num_T; ++j)
    _temperature[j] = _temperature_min + j * delta_T;

  // Divide the pressure into _num_p equal segments
  Real delta_p = (_pressure_max - _pressure_min) / static_cast<Real>(_num_p - 1);

  for (unsigned int i = 0; i < _num_p; ++i)
    _pressure[i] = _pressure_min + i * delta_p;

  // Generate the tabulated data at the pressure and temperature points
  for (const auto i : index_range(_properties))
  {
    if (_interpolated_properties[i] == "density")
      for (unsigned int p = 0; p < _num_p; ++p)
        for (unsigned int t = 0; t < _num_T; ++t)
          _properties[i][p * _num_T + t] = _fp->rho_from_p_T(_pressure[p], _temperature[t]);

    if (_interpolated_properties[i] == "enthalpy")
      for (unsigned int p = 0; p < _num_p; ++p)
        for (unsigned int t = 0; t < _num_T; ++t)
          _properties[i][p * _num_T + t] = _fp->h_from_p_T(_pressure[p], _temperature[t]);

    if (_interpolated_properties[i] == "internal_energy")
      for (unsigned int p = 0; p < _num_p; ++p)
        for (unsigned int t = 0; t < _num_T; ++t)
          _properties[i][p * _num_T + t] = _fp->e_from_p_T(_pressure[p], _temperature[t]);

    if (_interpolated_properties[i] == "viscosity")
      for (unsigned int p = 0; p < _num_p; ++p)
        for (unsigned int t = 0; t < _num_T; ++t)
          _properties[i][p * _num_T + t] = _fp->mu_from_p_T(_pressure[p], _temperature[t]);

    if (_interpolated_properties[i] == "k")
      for (unsigned int p = 0; p < _num_p; ++p)
        for (unsigned int t = 0; t < _num_T; ++t)
          _properties[i][p * _num_T + t] = _fp->k_from_p_T(_pressure[p], _temperature[t]);

    if (_interpolated_properties[i] == "c")
      for (unsigned int p = 0; p < _num_p; ++p)
        for (unsigned int t = 0; t < _num_T; ++t)
          _properties[i][p * _num_T + t] = _fp->c_from_p_T(_pressure[p], _temperature[t]);

    if (_interpolated_properties[i] == "cv")
      for (unsigned int p = 0; p < _num_p; ++p)
        for (unsigned int t = 0; t < _num_T; ++t)
          _properties[i][p * _num_T + t] = _fp->cv_from_p_T(_pressure[p], _temperature[t]);

    if (_interpolated_properties[i] == "cp")
      for (unsigned int p = 0; p < _num_p; ++p)
        for (unsigned int t = 0; t < _num_T; ++t)
          _properties[i][p * _num_T + t] = _fp->cp_from_p_T(_pressure[p], _temperature[t]);

    if (_interpolated_properties[i] == "entropy")
      for (unsigned int p = 0; p < _num_p; ++p)
        for (unsigned int t = 0; t < _num_T; ++t)
          _properties[i][p * _num_T + t] = _fp->s_from_p_T(_pressure[p], _temperature[t]);
  }
}

void
TabulatedFluidProperties::generateVETabulatedData()
{
  mooseAssert(_fp, "We should not try to generate (v,e) tabulated data without a _fp user object");
  _specific_volume.resize(_num_v);
  _internal_energy.resize(_num_e);

  // Generate data for all properties entered in input file
  _properties_ve.resize(_interpolated_properties_enum.size());
  _interpolated_properties.resize(_interpolated_properties_enum.size());

  // This is filled from the user input, so it does not matter than this operation is performed
  // for both the (p,T) and (v,e) tabulated data generation
  for (std::size_t i = 0; i < _interpolated_properties_enum.size(); ++i)
    _interpolated_properties[i] = _interpolated_properties_enum[i];

  for (const auto i : index_range(_properties_ve))
    _properties_ve[i].resize(_num_v * _num_e);

  // Grids in (v,e) are not read, so we either use user input or rely on (p,T) data
  createVEGridVectors();

  // Generate the tabulated data at the pressure and temperature points
  for (const auto i : index_range(_properties_ve))
  {
    if (_interpolated_properties[i] == "density")
      for (unsigned int v = 0; v < _num_v; ++v)
        for (unsigned int e = 0; e < _num_e; ++e)
          _properties_ve[i][v * _num_e + e] = 1. / _specific_volume[v];

    if (_interpolated_properties[i] == "enthalpy")
      for (unsigned int v = 0; v < _num_v; ++v)
        for (unsigned int e = 0; e < _num_e; ++e)
          _properties_ve[i][v * _num_e + e] =
              _fp->h_from_v_e(_specific_volume[v], _internal_energy[e]);

    if (_interpolated_properties[i] == "internal_energy")
      for (unsigned int v = 0; v < _num_v; ++v)
        for (unsigned int e = 0; e < _num_e; ++e)
          _properties_ve[i][v * _num_e + e] = _internal_energy[e];

    if (_interpolated_properties[i] == "viscosity")
      for (unsigned int v = 0; v < _num_v; ++v)
        for (unsigned int e = 0; e < _num_e; ++e)
          _properties_ve[i][v * _num_e + e] =
              _fp->mu_from_v_e(_specific_volume[v], _internal_energy[e]);

    if (_interpolated_properties[i] == "k")
      for (unsigned int v = 0; v < _num_v; ++v)
        for (unsigned int e = 0; e < _num_e; ++e)
          _properties_ve[i][v * _num_e + e] =
              _fp->k_from_v_e(_specific_volume[v], _internal_energy[e]);

    if (_interpolated_properties[i] == "c")
      for (unsigned int v = 0; v < _num_v; ++v)
        for (unsigned int e = 0; e < _num_e; ++e)
          _properties_ve[i][v * _num_e + e] =
              _fp->c_from_v_e(_specific_volume[v], _internal_energy[e]);

    if (_interpolated_properties[i] == "cv")
      for (unsigned int v = 0; v < _num_v; ++v)
        for (unsigned int e = 0; e < _num_e; ++e)
          _properties_ve[i][v * _num_e + e] =
              _fp->cv_from_v_e(_specific_volume[v], _internal_energy[e]);

    if (_interpolated_properties[i] == "cp")
      for (unsigned int v = 0; v < _num_v; ++v)
        for (unsigned int e = 0; e < _num_e; ++e)
          _properties_ve[i][v * _num_e + e] =
              _fp->cp_from_v_e(_specific_volume[v], _internal_energy[e]);

    if (_interpolated_properties[i] == "entropy")
      for (unsigned int v = 0; v < _num_v; ++v)
        for (unsigned int e = 0; e < _num_e; ++e)
          _properties_ve[i][v * _num_e + e] =
              _fp->s_from_v_e(_specific_volume[v], _internal_energy[e]);

    if (_interpolated_properties[i] == "pressure")
      for (unsigned int v = 0; v < _num_v; ++v)
        for (unsigned int e = 0; e < _num_e; ++e)
          _properties_ve[i][v * _num_e + e] =
              _fp->p_from_v_e(_specific_volume[v], _internal_energy[e]);

    if (_interpolated_properties[i] == "temperature")
      for (unsigned int v = 0; v < _num_v; ++v)
        for (unsigned int e = 0; e < _num_e; ++e)
          _properties_ve[i][v * _num_e + e] =
              _fp->T_from_v_e(_specific_volume[v], _internal_energy[e]);
  }
}

template <typename T>
void
TabulatedFluidProperties::checkInputVariables(T & pressure, T & temperature) const
{
  if (_OOBBehavior == Ignore)
    return;
  else if (MooseUtils::absoluteFuzzyGreaterThan(_pressure_min, pressure, libMesh::TOLERANCE) ||
           MooseUtils::absoluteFuzzyGreaterThan(pressure, _pressure_max, libMesh::TOLERANCE))
  {
    if (_OOBBehavior == Throw)
      throw MooseException("Pressure " + Moose::stringify(pressure) +
                           " is outside the range of tabulated pressure (" +
                           Moose::stringify(_pressure_min) + ", " +
                           Moose::stringify(_pressure_max) + ").");

    else
    {
      pressure = std::max(_pressure_min, std::min(pressure, _pressure_max));
      if (_OOBBehavior == DeclareInvalid)
        flagInvalidSolution("Pressure out of bounds");
      else if (_OOBBehavior == WarnInvalid)
        flagSolutionWarning("Pressure out of bounds");
    }
  }

  if (MooseUtils::absoluteFuzzyGreaterThan(_temperature_min, temperature, libMesh::TOLERANCE) ||
      MooseUtils::absoluteFuzzyGreaterThan(temperature, _temperature_max, libMesh::TOLERANCE))
  {
    if (_OOBBehavior == Throw)
      mooseError("Temperature " + Moose::stringify(temperature) +
                 " is outside the range of tabulated temperature (" +
                 Moose::stringify(_temperature_min) + ", " + Moose::stringify(_temperature_max) +
                 ").");
    else
    {
      temperature = std::max(T(_temperature_min), std::min(temperature, T(_temperature_max)));
      if (_OOBBehavior == DeclareInvalid)
        flagInvalidSolution("Temperature out of bounds");
      else if (_OOBBehavior == WarnInvalid)
        flagSolutionWarning("Temperature out of bounds");
    }
  }
}

template <typename T>
void
TabulatedFluidProperties::checkInputVariablesVE(T & v, T & e) const
{
  if (_OOBBehavior == Ignore)
    return;
  else if (e < _e_min || e > _e_max)
  {
    if (_OOBBehavior == Throw)
      throw MooseException("Specific internal energy " + Moose::stringify(e) +
                           " is outside the range of tabulated specific internal energies (" +
                           Moose::stringify(_e_min) + ", " + Moose::stringify(_e_max) + ").");
    else
    {
      e = std::max(_e_min, std::min(e, _e_max));
      if (_OOBBehavior == DeclareInvalid)
        flagInvalidSolution("Specific internal energy out of bounds");
      else if (_OOBBehavior == WarnInvalid)
        flagSolutionWarning("Specific internal energy out of bounds");
    }
  }

  if (v < _v_min || v > _v_max)
  {
    if (_OOBBehavior == Throw)
      mooseError("Specific volume " + Moose::stringify(v) +
                 " is outside the range of tabulated specific volumes (" +
                 Moose::stringify(_v_min) + ", " + Moose::stringify(_v_max) + ").");
    else
    {
      v = std::max(T(_v_min), std::min(v, T(_v_max)));
      if (_OOBBehavior == DeclareInvalid)
        flagInvalidSolution("Specific volume out of bounds");
      else if (_OOBBehavior == WarnInvalid)
        flagSolutionWarning("Specific volume out of bounds");
    }
  }
}

void
TabulatedFluidProperties::checkInitialGuess() const
{
  if (_fp && (_construct_pT_from_ve || _construct_pT_from_vh))
  {
    if (_p_initial_guess < _pressure_min || _p_initial_guess > _pressure_max)
      mooseWarning("Pressure initial guess for (p,T), (v,e) conversions " +
                   Moose::stringify(_p_initial_guess) +
                   " is outside the range of tabulated "
                   "pressure (" +
                   Moose::stringify(_pressure_min) + ", " + Moose::stringify(_pressure_max) + ").");

    if (_T_initial_guess < _temperature_min || _T_initial_guess > _temperature_max)
      mooseWarning("Temperature initial guess for (p,T), (v,e) conversions " +
                   Moose::stringify(_T_initial_guess) +
                   " is outside the range of tabulated "
                   "temperature (" +
                   Moose::stringify(_temperature_min) + ", " + Moose::stringify(_temperature_max) +
                   ").");
  }
}

void
TabulatedFluidProperties::readFileTabulationData(const bool use_pT)
{
  std::string file_name;
  if (use_pT)
  {
    _console << name() + ": Reading tabulated properties from " << _file_name_in << std::endl;
    _csv_reader.read();
    file_name = _file_name_in;
  }
  else
  {
    _console << name() + ": Reading tabulated properties from " << _file_name_ve_in << std::endl;
    _csv_reader.setFileName(_file_name_ve_in);
    _csv_reader.read();
    file_name = _file_name_ve_in;
  }

  const std::vector<std::string> & column_names = _csv_reader.getNames();

  // These columns form the grid and must be present in the file
  std::vector<std::string> required_columns;
  if (use_pT)
    required_columns = {"pressure", "temperature"};
  else
    required_columns = {"specific_volume", "internal_energy"};

  // Check that all required columns are present
  for (std::size_t i = 0; i < required_columns.size(); ++i)
  {
    if (std::find(column_names.begin(), column_names.end(), required_columns[i]) ==
        column_names.end())
      mooseError("No ",
                 required_columns[i],
                 " data read in ",
                 file_name,
                 ". A column named ",
                 required_columns[i],
                 " must be present");
  }

  // These columns can be present in the file
  std::vector<std::string> property_columns = {
      "density", "enthalpy", "viscosity", "k", "c", "cv", "cp", "entropy"};
  if (use_pT)
    property_columns.push_back("internal_energy");
  else
  {
    property_columns.push_back("pressure");
    property_columns.push_back("temperature");
  }

  // Check that any property names read from the file are present in the list of possible
  // properties, and if they are, add them to the list of read properties
  for (std::size_t i = 0; i < column_names.size(); ++i)
  {
    // Only check properties not in _required_columns
    if (std::find(required_columns.begin(), required_columns.end(), column_names[i]) ==
        required_columns.end())
    {
      if (std::find(property_columns.begin(), property_columns.end(), column_names[i]) ==
          property_columns.end())
        mooseWarning(column_names[i],
                     " read in ",
                     file_name,
                     " tabulation file is not one of the properties that TabulatedFluidProperties "
                     "understands. It will be ignored.");
      // We could be reading a (v,e) tabulation after having read a (p,T) tabulation, do not
      // insert twice
      else if (std::find(_interpolated_properties.begin(),
                         _interpolated_properties.end(),
                         column_names[i]) == _interpolated_properties.end())
        _interpolated_properties.push_back(column_names[i]);
    }
  }

  std::map<std::string, unsigned int> data_index;
  for (std::size_t i = 0; i < column_names.size(); ++i)
  {
    auto it = std::find(column_names.begin(), column_names.end(), column_names[i]);
    data_index[column_names[i]] = std::distance(column_names.begin(), it);
  }

  const std::vector<std::vector<Real>> & column_data = _csv_reader.getData();

  // Extract the pressure and temperature data vectors
  if (use_pT)
  {
    _pressure = column_data[data_index.find("pressure")->second];
    _temperature = column_data[data_index.find("temperature")->second];
  }
  else
  {
    _specific_volume = column_data[data_index.find("specific_volume")->second];
    _internal_energy = column_data[data_index.find("internal_energy")->second];
  }

  if (use_pT)
    checkFileTabulationGrids(_pressure, _temperature, file_name, "pressure", "temperature");
  else
    checkFileTabulationGrids(_specific_volume,
                             _internal_energy,
                             file_name,
                             "specific volume",
                             "specific internal energy");

  if (use_pT)
  {
    _num_p = _pressure.size();
    _num_T = _temperature.size();

    // Minimum and maximum pressure and temperature. Note that _pressure and
    // _temperature are sorted
    _pressure_min = _pressure.front();
    _pressure_max = _pressure.back();
    _temperature_min = _temperature.front();
    _temperature_max = _temperature.back();

    // Extract the fluid property data from the file
    for (std::size_t i = 0; i < _interpolated_properties.size(); ++i)
      _properties.push_back(column_data[data_index.find(_interpolated_properties[i])->second]);
  }
  else
  {
    _num_v = _specific_volume.size();
    _num_e = _internal_energy.size();

    // Minimum and maximum specific internal energy and specific volume
    _v_min = _specific_volume.front();
    _v_max = _specific_volume.back();
    _e_min = _internal_energy.front();
    _e_max = _internal_energy.back();

    // We cannot overwrite the tabulated data grid with a grid generated from user-input for the
    // purpose of creating (p,T) to (v,e) interpolations
    if (_construct_pT_from_ve)
      paramError("construct_pT_from_ve",
                 "Reading a (v,e) tabulation and generating (p,T) to (v,e) interpolation tables is "
                 "not supported at this time.");

    // Make sure we use the tabulation bounds
    _e_bounds_specified = true;
    _v_bounds_specified = true;

    // Extract the fluid property data from the file
    _properties_ve.reserve(_interpolated_properties.size());
    for (std::size_t i = 0; i < _interpolated_properties.size(); ++i)
      _properties_ve.push_back(column_data[data_index.find(_interpolated_properties[i])->second]);
  }
}

void
TabulatedFluidProperties::checkFileTabulationGrids(std::vector<Real> & v1,
                                                   std::vector<Real> & v2,
                                                   const std::string & file_name,
                                                   const std::string & v1_name,
                                                   const std::string & v2_name)
{
  // NOTE: We kept the comments in terms of pressure & temperature for clarity
  // Pressure (v1) and temperature (v2) data contains duplicates due to the csv format.
  // First, check that pressure (v1) is monotonically increasing
  if (!std::is_sorted(v1.begin(), v1.end()))
    mooseError("The column data for ", v1_name, " is not monotonically increasing in ", file_name);

  // The first pressure (v1) value is repeated for each temperature (v2) value. Counting the
  // number of repeats provides the number of temperature (v2) values
  auto num_v2 = std::count(v1.begin(), v1.end(), v1.front());

  // Now remove the duplicates in the pressure (v1) vector
  auto last_unique = std::unique(v1.begin(), v1.end());
  v1.erase(last_unique, v1.end());

  // Check that the number of rows in the csv file is equal to _num_v1 * _num_v2
  // v2 is currently the same size as the column_data (will get trimmed at the end)
  if (v2.size() != v1.size() * libMesh::cast_int<unsigned int>(num_v2))
    mooseError("The number of rows in ",
               file_name,
               " is not equal to the number of unique ",
               v1_name,
               " values ",
               v1.size(),
               " multiplied by the number of unique ",
               v2_name,
               " values ",
               num_v2);

  // Need to make sure that the temperature (v2) values are provided in ascending order
  std::vector<Real> base_v2(v2.begin(), v2.begin() + num_v2);
  if (!std::is_sorted(base_v2.begin(), base_v2.end()))
    mooseError("The column data for ", v2_name, " is not monotonically increasing in ", file_name);

  // Need to make sure that the temperature (v2) are repeated for each pressure (v1) grid point
  auto it_v2 = v2.begin() + num_v2;
  for (const auto i : make_range(v1.size() - 1))
  {
    std::vector<Real> repeated_v2(it_v2, it_v2 + num_v2);
    if (repeated_v2 != base_v2)
      mooseError(v2_name,
                 " values for ",
                 v1_name,
                 " ",
                 v1[i + 1],
                 " are not identical to values for ",
                 v1[0]);

    std::advance(it_v2, num_v2);
  }

  // At this point, all temperature (v2) data has been provided in ascending order
  // identically for each pressure (v1) value, so we can just keep the first range
  v2.erase(v2.begin() + num_v2, v2.end());
}

void
TabulatedFluidProperties::computePropertyIndicesInInterpolationVectors()
{
  // At this point, all properties read or generated are able to be used by
  // TabulatedFluidProperties. Now set flags and indexes for each property in
  //_interpolated_properties to use in property calculations
  for (std::size_t i = 0; i < _interpolated_properties.size(); ++i)
  {
    if (_interpolated_properties[i] == "density")
    {
      _interpolate_density = true;
      _density_idx = i;
    }
    else if (_interpolated_properties[i] == "enthalpy")
    {
      _interpolate_enthalpy = true;
      _enthalpy_idx = i;
    }
    else if (_interpolated_properties[i] == "internal_energy")
    {
      _interpolate_internal_energy = true;
      _internal_energy_idx = i;
    }
    else if (_interpolated_properties[i] == "viscosity")
    {
      _interpolate_viscosity = true;
      _viscosity_idx = i;
    }
    else if (_interpolated_properties[i] == "k")
    {
      _interpolate_k = true;
      _k_idx = i;
    }
    else if (_interpolated_properties[i] == "c")
    {
      _interpolate_c = true;
      _c_idx = i;
    }
    else if (_interpolated_properties[i] == "cp")
    {
      _interpolate_cp = true;
      _cp_idx = i;
    }
    else if (_interpolated_properties[i] == "cv")
    {
      _interpolate_cv = true;
      _cv_idx = i;
    }
    else if (_interpolated_properties[i] == "entropy")
    {
      _interpolate_entropy = true;
      _entropy_idx = i;
    }
    else if (_interpolated_properties[i] == "pressure")
    {
      _interpolate_pressure = true;
      _p_idx = i;
    }
    else if (_interpolated_properties[i] == "temperature")
    {
      _interpolate_temperature = true;
      _T_idx = i;
    }
    else
      mooseError("Specified property '" + _interpolated_properties[i] +
                 "' is present in the tabulation but is not currently leveraged by the code in the "
                 "TabulatedFluidProperties. If it is spelled correctly, then please contact a "
                 "MOOSE or fluid properties module developer.");
  }
}

void
TabulatedFluidProperties::createVGridVector()
{
  mooseAssert(_file_name_ve_in.empty(), "We should be reading the specific volume grid from file");
  if (!_v_bounds_specified)
  {
    if (_fp)
    {
      // extreme values of specific volume for the grid bounds
      Real v1 = v_from_p_T(_pressure_min, _temperature_min);
      Real v2 = v_from_p_T(_pressure_max, _temperature_min);
      Real v3 = v_from_p_T(_pressure_min, _temperature_max);
      Real v4 = v_from_p_T(_pressure_max, _temperature_max);
      _v_min = std::min({v1, v2, v3, v4});
      _v_max = std::max({v1, v2, v3, v4});
    }
    // if csv exists, get max and min values from csv file
    else
    {
      Real rho_max =
          *std::max_element(_properties[_density_idx].begin(), _properties[_density_idx].end());
      Real rho_min =
          *std::min_element(_properties[_density_idx].begin(), _properties[_density_idx].end());
      _v_max = 1 / rho_min;
      _v_min = 1 / rho_max;
    }
    // Prevent changing the bounds of the grid
    _v_bounds_specified = true;
  }

  // Create v grid for interpolation
  _specific_volume.resize(_num_v);
  if (_log_space_v)
  {
    // incrementing the exponent linearly will yield a log-spaced grid after taking the value to
    // the power of 10
    Real dv = (std::log10(_v_max) - std::log10(_v_min)) / ((Real)_num_v - 1);
    Real log_v_min = std::log10(_v_min);
    for (unsigned int j = 0; j < _num_v; ++j)
      _specific_volume[j] = std::pow(10, log_v_min + j * dv);
  }
  else
  {
    Real dv = (_v_max - _v_min) / ((Real)_num_v - 1);
    for (unsigned int j = 0; j < _num_v; ++j)
      _specific_volume[j] = _v_min + j * dv;
  }
}

void
TabulatedFluidProperties::createVEGridVectors()
{
  createVGridVector();
  if (!_e_bounds_specified)
  {
    if (_fp)
    {
      // extreme values of internal energy for the grid bounds
      Real e1 = e_from_p_T(_pressure_min, _temperature_min);
      Real e2 = e_from_p_T(_pressure_max, _temperature_min);
      Real e3 = e_from_p_T(_pressure_min, _temperature_max);
      Real e4 = e_from_p_T(_pressure_max, _temperature_max);
      _e_min = std::min({e1, e2, e3, e4});
      _e_max = std::max({e1, e2, e3, e4});
    }
    // if csv exists, get max and min values from csv file
    else
    {
      _e_min = *std::min_element(_properties[_internal_energy_idx].begin(),
                                 _properties[_internal_energy_idx].end());
      _e_max = *std::max_element(_properties[_internal_energy_idx].begin(),
                                 _properties[_internal_energy_idx].end());
    }
  }

  // Create e grid for interpolation
  _internal_energy.resize(_num_e);
  if (_log_space_e)
  {
    // incrementing the exponent linearly will yield a log-spaced grid after taking the value to
    // the power of 10
    if (_e_min < 0)
      mooseError("Logarithmic grid in specific energy can only be used with a positive specific "
                 "energy. Current minimum: " +
                 std::to_string(_e_min));
    Real de = (std::log10(_e_max) - std::log10(_e_min)) / ((Real)_num_e - 1);
    Real log_e_min = std::log10(_e_min);
    for (const auto j : make_range(_num_e))
      _internal_energy[j] = std::pow(10, log_e_min + j * de);
  }
  else
  {
    Real de = (_e_max - _e_min) / ((Real)_num_e - 1);
    for (const auto j : make_range(_num_e))
      _internal_energy[j] = _e_min + j * de;
  }
}

void
TabulatedFluidProperties::createVHGridVectors()
{
  if (_file_name_ve_in.empty())
    createVGridVector();
  if (_fp)
  {
    // extreme values of enthalpy for the grid bounds
    Real h1 = h_from_p_T(_pressure_min, _temperature_min);
    Real h2 = h_from_p_T(_pressure_max, _temperature_min);
    Real h3 = h_from_p_T(_pressure_min, _temperature_max);
    Real h4 = h_from_p_T(_pressure_max, _temperature_max);
    _h_min = std::min({h1, h2, h3, h4});
    _h_max = std::max({h1, h2, h3, h4});
  }
  // if csv exists, get max and min values from csv file
  else if (_properties.size())
  {
    _h_max = *max_element(_properties[_enthalpy_idx].begin(), _properties[_enthalpy_idx].end());
    _h_min = *min_element(_properties[_enthalpy_idx].begin(), _properties[_enthalpy_idx].end());
  }
  else if (_properties_ve.size())
  {
    _h_max = *max_element(_properties[_enthalpy_idx].begin(), _properties[_enthalpy_idx].end());
    _h_min = *min_element(_properties[_enthalpy_idx].begin(), _properties[_enthalpy_idx].end());
  }
  else
    mooseError("Need a source to compute the enthalpy grid bounds: either a FP object, or a (p,T) "
               "tabulation file or a (v,e) tabulation file");

  // Create h grid for interpolation
  // enthalpy & internal energy use same # grid points
  _enthalpy.resize(_num_e);
  if (_log_space_h)
  {
    // incrementing the exponent linearly will yield a log-spaced grid after taking the value to
    // the power of 10
    if (_h_min < 0)
      mooseError("Logarithmic grid in specific energy can only be used with a positive enthalpy. "
                 "Current minimum: " +
                 std::to_string(_h_min));
    Real dh = (std::log10(_h_max) - std::log10(_h_min)) / ((Real)_num_e - 1);
    Real log_h_min = std::log10(_h_min);
    for (const auto j : make_range(_num_e))
      _enthalpy[j] = std::pow(10, log_h_min + j * dh);
  }
  else
  {
    Real dh = (_h_max - _h_min) / ((Real)_num_e - 1);
    for (const auto j : make_range(_num_e))
      _enthalpy[j] = _h_min + j * dh;
  }
}

void
TabulatedFluidProperties::missingVEInterpolationError(const std::string & function_name) const
{
  mooseError(function_name +
             ": to call this function you must:\n-add this property to the list to the list of "
             "'interpolated_properties'\n and then either:\n-construct (p, T) from (v, e) "
             "tabulations using the 'construct_pT_from_ve' parameter\n-load (v,e) interpolation "
             "tables using the 'fluid_property_ve_file' parameter");
}

template void TabulatedFluidProperties::checkInputVariables(Real & pressure,
                                                            Real & temperature) const;
template void TabulatedFluidProperties::checkInputVariables(ADReal & pressure,
                                                            ADReal & temperature) const;
template void TabulatedFluidProperties::checkInputVariablesVE(Real & v, Real & e) const;
template void TabulatedFluidProperties::checkInputVariablesVE(ADReal & v, ADReal & e) const;
