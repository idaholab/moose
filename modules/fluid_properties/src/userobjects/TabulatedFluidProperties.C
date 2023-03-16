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

InputParameters
TabulatedFluidProperties::validParams()
{
  InputParameters params = SinglePhaseFluidProperties::validParams();
  params.addClassDescription(
      "Single phase fluid properties computed using bi-dimensional interpolation of tabulated "
      "values.");

  params.addParam<FileName>(
      "fluid_property_file",
      "fluid_properties.csv",
      "Name of the csv file containing the tabulated fluid property data. If "
      "no file exists and save_file = true, then one will be written to "
      "fluid_properties.csv using the temperature and pressure range specified.");
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
  params.addParam<UserObjectName>("fp", "The name of the FluidProperties UserObject");
  MultiMooseEnum properties("density enthalpy internal_energy viscosity k c cv cp entropy",
                            "density enthalpy internal_energy viscosity");
  params.addParam<MultiMooseEnum>("interpolated_properties",
                                  properties,
                                  "Properties to interpolate if no data file is provided");
  params.addParam<bool>("save_file", true, "Whether to save the csv fluid properties file");
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
      "error_on_out_of_bounds",
      true,
      "Whether pressure or temperature from tabulation exceeding user-specified bounds leads to "
      "an error.");
  params.addParam<bool>(
      "use_log_grid_v",
      false,
      "Option to use a base-10 logarithmically-spaced grid for specific volume instead of a "
      "linearly-spaced grid.");

  params.addParamNamesToGroup("fluid_property_file save_file", "Tabulation file read/write");
  params.addParamNamesToGroup("construct_pT_from_ve construct_pT_from_vh",
                              "Variable set conversion");
  params.addParamNamesToGroup(
      "temperature_min temperature_max pressure_min pressure_max error_on_out_of_bounds",
      "Tabulation and interpolation bounds");
  params.addParamNamesToGroup("num_T num_p num_v num_e use_log_grid_v",
                              "Tabulation and interpolation discretization");

  return params;
}

TabulatedFluidProperties::TabulatedFluidProperties(const InputParameters & parameters)
  : SinglePhaseFluidProperties(parameters),
    _file_name(getParam<FileName>("fluid_property_file")),
    _temperature_min(getParam<Real>("temperature_min")),
    _temperature_max(getParam<Real>("temperature_max")),
    _pressure_min(getParam<Real>("pressure_min")),
    _pressure_max(getParam<Real>("pressure_max")),
    _num_T(getParam<unsigned int>("num_T")),
    _num_p(getParam<unsigned int>("num_p")),
    _save_file(getParam<bool>("save_file")),
    _fp(isParamValid("fp") ? &getUserObject<SinglePhaseFluidProperties>("fp") : nullptr),
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
    _density_idx(0),
    _enthalpy_idx(0),
    _internal_energy_idx(0),
    _viscosity_idx(0),
    _k_idx(0),
    _c_idx(0),
    _cp_idx(0),
    _cv_idx(0),
    _entropy_idx(0),
    _csv_reader(_file_name, &_communicator),
    _construct_pT_from_ve(getParam<bool>("construct_pT_from_ve")),
    _construct_pT_from_vh(getParam<bool>("construct_pT_from_vh")),
    _initial_setup_done(false),
    _num_v(getParam<unsigned int>("num_v")),
    _num_e(getParam<unsigned int>("num_e")),
    _error_on_out_of_bounds(getParam<bool>("error_on_out_of_bounds")),
    _log_space_v(getParam<bool>("use_log_grid_v"))
{
  // Check that initial guess (used in Newton Method) is within min and max values
  checkInitialGuess();
  // Sanity check on minimum and maximum temperatures and pressures
  if (_temperature_max <= _temperature_min)
    mooseError("temperature_max must be greater than temperature_min");
  if (_pressure_max <= _pressure_min)
    mooseError("pressure_max must be greater than pressure_min");

  // Lines starting with # in the data file are treated as comments
  _csv_reader.setComment("#");
}

void
TabulatedFluidProperties::initialSetup()
{
  if (_initial_setup_done)
    return;
  _initial_setup_done = true;

  // Check to see if _file_name supplied exists. If it does, that data
  // will be used. If it does not exist, data will be generated and then
  // written to _file_name.
  std::ifstream file(_file_name.c_str());
  if (file.good())
  {
    _console << name() + ": Reading tabulated properties from " << _file_name << std::endl;
    _csv_reader.read();

    const std::vector<std::string> & column_names = _csv_reader.getNames();

    // Check that all required columns are present
    for (std::size_t i = 0; i < _required_columns.size(); ++i)
    {
      if (std::find(column_names.begin(), column_names.end(), _required_columns[i]) ==
          column_names.end())
        mooseError("No ",
                   _required_columns[i],
                   " data read in ",
                   _file_name,
                   ". A column named ",
                   _required_columns[i],
                   " must be present");
    }

    // Check that any property names read from the file are present in the list of possible
    // properties, and if they are, add them to the list of read properties
    for (std::size_t i = 0; i < column_names.size(); ++i)
    {
      // Only check properties not in _required_columns
      if (std::find(_required_columns.begin(), _required_columns.end(), column_names[i]) ==
          _required_columns.end())
      {
        if (std::find(_property_columns.begin(), _property_columns.end(), column_names[i]) ==
            _property_columns.end())
          mooseError(column_names[i],
                     " read in ",
                     _file_name,
                     " is not one of the properties that TabulatedFluidProperties understands");
        else
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
    _pressure = column_data[data_index.find("pressure")->second];
    _temperature = column_data[data_index.find("temperature")->second];

    // Pressure and temperature data contains duplicates due to the csv format.
    // First, check that pressure is monotonically increasing
    if (!std::is_sorted(_pressure.begin(), _pressure.end()))
      mooseError("The column data for pressure is not monotonically increasing in ", _file_name);

    // The first pressure value is repeated for each temperature value. Counting the
    // number of repeats provides the number of temperature values
    auto num_T = std::count(_pressure.begin(), _pressure.end(), _pressure.front());

    // Now remove the duplicates in the pressure vector
    auto last_unique = std::unique(_pressure.begin(), _pressure.end());
    _pressure.erase(last_unique, _pressure.end());
    _num_p = _pressure.size();

    // Check that the number of rows in the csv file is equal to _num_p * _num_T
    if (column_data[0].size() != _num_p * static_cast<unsigned int>(num_T))
      mooseError("The number of rows in ",
                 _file_name,
                 " is not equal to the number of unique pressure values ",
                 _num_p,
                 " multiplied by the number of unique temperature values ",
                 num_T);

    // Need to make sure that the temperature values are provided in ascending order
    // as well as duplicated for each pressure value
    std::vector<Real> temp0(_temperature.begin(), _temperature.begin() + num_T);
    if (!std::is_sorted(temp0.begin(), temp0.end()))
      mooseError("The column data for temperature is not monotonically increasing in ", _file_name);

    auto it_temp = _temperature.begin() + num_T;
    for (std::size_t i = 1; i < _pressure.size(); ++i)
    {
      std::vector<Real> temp(it_temp, it_temp + num_T);
      if (temp != temp0)
        mooseError("Temperature values for pressure ",
                   _pressure[i],
                   " are not identical to values for ",
                   _pressure[0]);

      std::advance(it_temp, num_T);
    }

    // At this point, all temperature data has been provided in ascending order
    // identically for each pressure value, so we can just keep the first range
    _temperature.erase(_temperature.begin() + num_T, _temperature.end());
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
    if (!_fp)
      mooseError("No csv file or fp object exist.");
    _console << name() + ": No tabulated properties file named " << _file_name << " exists.\n";
    _console << name() + ": Generating tabulated data\n";
    if (_save_file)
      _console << name() + ": Writing tabulated data to " << _file_name << "\n";

    _console << std::flush;

    generateTabulatedData();

    // Write tabulated data to file
    if (_save_file)
      writeTabulatedData(_file_name);
  }

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
    if (_interpolated_properties[i] == "enthalpy")
    {
      _interpolate_enthalpy = true;
      _enthalpy_idx = i;
    }
    if (_interpolated_properties[i] == "internal_energy")
    {
      _interpolate_internal_energy = true;
      _internal_energy_idx = i;
    }
    if (_interpolated_properties[i] == "viscosity")
    {
      _interpolate_viscosity = true;
      _viscosity_idx = i;
    }
    if (_interpolated_properties[i] == "k")
    {
      _interpolate_k = true;
      _k_idx = i;
    }
    if (_interpolated_properties[i] == "c")
    {
      _interpolate_c = true;
      _c_idx = i;
    }
    if (_interpolated_properties[i] == "cp")
    {
      _interpolate_cp = true;
      _cp_idx = i;
    }
    if (_interpolated_properties[i] == "cv")
    {
      _interpolate_cv = true;
      _cv_idx = i;
    }
    if (_interpolated_properties[i] == "entropy")
    {
      _interpolate_entropy = true;
      _entropy_idx = i;
    }
  }
  constructInterpolation();
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
    mooseError("Molar Mass not specified.");
}

Real
TabulatedFluidProperties::v_from_p_T(Real pressure, Real temperature) const
{
  if (_interpolate_density)
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
  if (_interpolate_density)
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
  if (_interpolate_density)
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
  if (_interpolate_density)
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
  if (_interpolate_density)
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
TabulatedFluidProperties::e_from_p_T(Real pressure, Real temperature) const
{
  if (_interpolate_internal_energy)
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
  if (_interpolate_internal_energy)
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
  Real T = FluidPropertiesUtils::NewtonSolve(
               pressure, rho, _T_initial_guess, _tolerance, lambda, name() + "::T_from_p_rho")
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
TabulatedFluidProperties::h_from_p_T(Real pressure, Real temperature) const
{
  if (_interpolate_enthalpy)
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

void
TabulatedFluidProperties::h_from_p_T(
    Real pressure, Real temperature, Real & h, Real & dh_dp, Real & dh_dT) const
{
  if (_interpolate_enthalpy)
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
  if (_interpolate_viscosity)
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
  if (_interpolate_viscosity)
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
  if (_interpolate_c)
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
  if (_interpolate_c)
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
  if (_interpolate_cp)
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
  if (_interpolate_cp)
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
  if (_interpolate_cv)
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
  if (_interpolate_k)
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
  if (_interpolate_k)
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
  if (_interpolate_entropy)
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
  if (_interpolate_entropy)
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
  if (!_construct_pT_from_vh)
    mooseError("You must construct pT from vh tables when calling e_from_v_h.");
  const Real p = _p_from_v_h_ipol->sample(v, h);
  const Real T = _T_from_v_h_ipol->sample(v, h);
  return e_from_p_T(p, T);
}

void
TabulatedFluidProperties::e_from_v_h(Real v, Real h, Real & e, Real & de_dv, Real & de_dh) const
{
  if (!_construct_pT_from_vh)
    mooseError("You must construct pT from vh tables when calling e_from_v_h.");
  Real p = 0, dp_dv = 0, dp_dh = 0;
  _p_from_v_h_ipol->sampleValueAndDerivatives(v, h, p, dp_dv, dp_dh);
  Real T = 0, dT_dv = 0, dT_dh = 0;
  _T_from_v_h_ipol->sampleValueAndDerivatives(v, h, T, dT_dv, dT_dh);
  Real de_dp, de_dT;
  e_from_p_T(p, T, e, de_dp, de_dT);
  de_dv = de_dp * dp_dv + de_dT * dT_dv;
  de_dh = de_dp * dp_dh + de_dT * dT_dh;
}

std::vector<Real>
TabulatedFluidProperties::henryCoefficients() const
{
  if (_fp)
    return _fp->henryCoefficients();
  else
    mooseError("henryCoefficients not specified.");
}

Real
TabulatedFluidProperties::vaporPressure(Real temperature) const
{
  if (_fp)
    return _fp->vaporPressure(temperature);
  else
    mooseError("vaporPres not specified.");
}

void
TabulatedFluidProperties::vaporPressure(Real temperature, Real & psat, Real & dpsat_dT) const
{
  if (_fp)
    _fp->vaporPressure(temperature, psat, dpsat_dT);
  else
    mooseError("vaporPressure not specified.");
}

Real
TabulatedFluidProperties::p_from_v_e(Real v, Real e) const
{
  if (!_construct_pT_from_ve)
    mooseError("You must construct pT from ve tables when calling p_from_v_e.");
  return _p_from_v_e_ipol->sample(v, e);
}

void
TabulatedFluidProperties::p_from_v_e(Real v, Real e, Real & p, Real & dp_dv, Real & dp_de) const
{
  if (!_construct_pT_from_ve)
    mooseError("You must construct pT from ve tables when calling p_from_v_e.");
  _p_from_v_e_ipol->sampleValueAndDerivatives(v, e, p, dp_dv, dp_de);
}

Real
TabulatedFluidProperties::T_from_v_e(Real v, Real e) const
{
  if (!_construct_pT_from_ve)
    mooseError("You must construct pT from ve tables when calling T_from_v_e.");
  return _T_from_v_e_ipol->sample(v, e);
}

void
TabulatedFluidProperties::T_from_v_e(Real v, Real e, Real & T, Real & dT_dv, Real & dT_de) const
{
  if (!_construct_pT_from_ve)
    mooseError("You must construct pT from ve tables when calling T_from_v_e.");
  _T_from_v_e_ipol->sampleValueAndDerivatives(v, e, T, dT_dv, dT_de);
}

Real
TabulatedFluidProperties::c_from_v_e(Real v, Real e) const
{
  if (!_construct_pT_from_ve)
    mooseError("You must construct pT from ve tables when calling c_from_v_e.");
  Real p = _p_from_v_e_ipol->sample(v, e);
  Real T = _T_from_v_e_ipol->sample(v, e);
  return c_from_p_T(p, T);
}

void
TabulatedFluidProperties::c_from_v_e(Real v, Real e, Real & c, Real & dc_dv, Real & dc_de) const
{
  if (!_construct_pT_from_ve)
    mooseError("You must construct pT from ve tables when calling c_from_v_e.");
  Real p, dp_dv, dp_de;
  _p_from_v_e_ipol->sampleValueAndDerivatives(v, e, p, dp_dv, dp_de);
  Real T, dT_dv, dT_de;
  _T_from_v_e_ipol->sampleValueAndDerivatives(v, e, T, dT_dv, dT_de);
  Real dc_dp, dc_dT;
  c_from_p_T(p, T, c, dc_dp, dc_dT);
  dc_dv = dc_dp * dp_dv + dc_dT * dT_dv;
  dc_de = dc_dp * dp_de + dc_dT * dT_de;
}

Real
TabulatedFluidProperties::cp_from_v_e(Real v, Real e) const
{
  if (!_construct_pT_from_ve)
    mooseError("You must construct pT from ve tables when calling cp_from_v_e.");
  Real p = _p_from_v_e_ipol->sample(v, e);
  Real T = _T_from_v_e_ipol->sample(v, e);
  return cp_from_p_T(p, T);
}

void
TabulatedFluidProperties::cp_from_v_e(Real v, Real e, Real & cp, Real & dcp_dv, Real & dcp_de) const
{
  if (!_construct_pT_from_ve)
    mooseError("You must construct pT from ve tables when calling cp_from_v_e.");
  Real p, dp_dv, dp_de;
  _p_from_v_e_ipol->sampleValueAndDerivatives(v, e, p, dp_dv, dp_de);
  Real T, dT_dv, dT_de;
  _T_from_v_e_ipol->sampleValueAndDerivatives(v, e, T, dT_dv, dT_de);
  Real dcp_dp, dcp_dT;
  cp_from_p_T(p, T, cp, dcp_dp, dcp_dT);
  dcp_dv = dcp_dp * dp_dv + dcp_dT * dT_dv;
  dcp_de = dcp_dp * dp_de + dcp_dT * dT_de;
}

Real
TabulatedFluidProperties::cv_from_v_e(Real v, Real e) const
{
  if (!_construct_pT_from_ve)
    mooseError("You must construct pT from ve tables when calling cv_from_v_e.");
  Real p = _p_from_v_e_ipol->sample(v, e);
  Real T = _T_from_v_e_ipol->sample(v, e);
  return cv_from_p_T(p, T);
}

void
TabulatedFluidProperties::cv_from_v_e(Real v, Real e, Real & cv, Real & dcv_dv, Real & dcv_de) const
{
  if (!_construct_pT_from_ve)
    mooseError("You must construct pT from ve tables when calling cv_from_v_e.");
  Real p, dp_dv, dp_de;
  _p_from_v_e_ipol->sampleValueAndDerivatives(v, e, p, dp_dv, dp_de);
  Real T, dT_dv, dT_de;
  _T_from_v_e_ipol->sampleValueAndDerivatives(v, e, T, dT_dv, dT_de);
  Real dcv_dp, dcv_dT;
  cv_from_p_T(p, T, cv, dcv_dp, dcv_dT);
  dcv_dv = dcv_dp * dp_dv + dcv_dT * dT_dv;
  dcv_de = dcv_dp * dp_de + dcv_dT * dT_de;
}

Real
TabulatedFluidProperties::mu_from_v_e(Real v, Real e) const
{
  if (!_construct_pT_from_ve)
    mooseError("You must construct pT from ve tables when calling mu_from_v_e.");
  Real p = _p_from_v_e_ipol->sample(v, e);
  Real T = _T_from_v_e_ipol->sample(v, e);
  return mu_from_p_T(p, T);
}

void
TabulatedFluidProperties::mu_from_v_e(Real v, Real e, Real & mu, Real & dmu_dv, Real & dmu_de) const
{
  if (!_construct_pT_from_ve)
    mooseError("You must construct pT from ve tables when calling mu_from_v_e.");
  Real p, dp_dv, dp_de;
  _p_from_v_e_ipol->sampleValueAndDerivatives(v, e, p, dp_dv, dp_de);
  Real T, dT_dv, dT_de;
  _T_from_v_e_ipol->sampleValueAndDerivatives(v, e, T, dT_dv, dT_de);
  Real dmu_dp, dmu_dT;
  mu_from_p_T(p, T, mu, dmu_dp, dmu_dT);
  dmu_dv = dmu_dp * dp_dv + dmu_dT * dT_dv;
  dmu_de = dmu_dp * dp_de + dmu_dT * dT_de;
}

Real
TabulatedFluidProperties::k_from_v_e(Real v, Real e) const
{
  if (!_construct_pT_from_ve)
    mooseError("You must construct pT from ve tables when calling k_from_v_e.");
  Real T = _T_from_v_e_ipol->sample(v, e);
  Real p = _p_from_v_e_ipol->sample(v, e);
  return k_from_p_T(p, T);
}

void
TabulatedFluidProperties::k_from_v_e(Real v, Real e, Real & k, Real & dk_dv, Real & dk_de) const
{
  if (!_construct_pT_from_ve)
    mooseError("You must construct pT from ve tables when calling k_from_v_e.");
  Real p, dp_dv, dp_de;
  _p_from_v_e_ipol->sampleValueAndDerivatives(v, e, p, dp_dv, dp_de);
  Real T, dT_dv, dT_de;
  _T_from_v_e_ipol->sampleValueAndDerivatives(v, e, T, dT_dv, dT_de);
  Real dk_dp, dk_dT;
  k_from_p_T(p, T, k, dk_dp, dk_dT);
  dk_dv = dk_dp * dp_dv + dk_dT * dT_dv;
  dk_de = dk_dp * dp_de + dk_dT * dT_de;
}

Real
TabulatedFluidProperties::g_from_v_e(Real v, Real e) const
{
  Real p0 = _p_initial_guess;
  Real T0 = _T_initial_guess;
  Real p, T;
  bool conversion_succeeded;
  p_T_from_v_e(v, e, p0, T0, p, T, conversion_succeeded);
  const Real s = s_from_p_T(p, T);
  const Real h = h_from_p_T(p, T);
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
TabulatedFluidProperties::T_from_h_p(Real h, Real pressure) const
{
  auto lambda = [&](Real pressure, Real current_T, Real & new_h, Real & dh_dp, Real & dh_dT)
  { h_from_p_T(pressure, current_T, new_h, dh_dp, dh_dT); };
  Real T = FluidPropertiesUtils::NewtonSolve(
               pressure, h, _T_initial_guess, _tolerance, lambda, name() + "::T_from_h_p")
               .first;
  // check for nans
  if (std::isnan(T))
    mooseError("Conversion from enthalpy (h = ",
               h,
               ") and pressure (p = ",
               pressure,
               ") to temperature failed to converge.");
  return T;
}

Real
TabulatedFluidProperties::s_from_h_p(Real h, Real pressure) const
{
  Real T = T_from_h_p(h, pressure);
  return s_from_p_T(pressure, T);
}

void
TabulatedFluidProperties::writeTabulatedData(std::string file_name)
{
  if (processor_id() == 0)
  {
    MooseUtils::checkFileWriteable(file_name);

    std::ofstream file_out(file_name.c_str());

    // Write out date and fluid type
    time_t now = time(&now);
    if (_fp)
      file_out << "# " << _fp->fluidName() << " properties created by TabulatedFluidProperties on "
               << ctime(&now) << "\n";
    else
      mooseError("fluidName not specified.");

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
}

void
TabulatedFluidProperties::generateTabulatedData()
{
  _pressure.resize(_num_p);
  _temperature.resize(_num_T);

  // Generate data for all properties entered in input file
  _properties.resize(_interpolated_properties_enum.size());
  _interpolated_properties.resize(_interpolated_properties_enum.size());

  for (std::size_t i = 0; i < _interpolated_properties_enum.size(); ++i)
    _interpolated_properties[i] = _interpolated_properties_enum[i];

  for (std::size_t i = 0; i < _properties.size(); ++i)
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
  for (std::size_t i = 0; i < _properties.size(); ++i)
  {
    if (_interpolated_properties[i] == "density")
      for (unsigned int p = 0; p < _num_p; ++p)
        for (unsigned int t = 0; t < _num_T; ++t)
        {
          if (_fp)
            _properties[i][p * _num_T + t] = _fp->rho_from_p_T(_pressure[p], _temperature[t]);
          else
            paramError("fp", "No fluid properties or csv data provided for density.");
        }

    if (_interpolated_properties[i] == "enthalpy")
      for (unsigned int p = 0; p < _num_p; ++p)
        for (unsigned int t = 0; t < _num_T; ++t)
        {
          if (_fp)
            _properties[i][p * _num_T + t] = _fp->h_from_p_T(_pressure[p], _temperature[t]);
          else
            paramError("fp", "No fluid properties or csv data provided for enthalpy.");
        }

    if (_interpolated_properties[i] == "internal_energy")
      for (unsigned int p = 0; p < _num_p; ++p)
        for (unsigned int t = 0; t < _num_T; ++t)
        {
          if (_fp)
            _properties[i][p * _num_T + t] = _fp->e_from_p_T(_pressure[p], _temperature[t]);
          else
            paramError("fp", "No fluid properties or csv data provided for internal energy.");
        }

    if (_interpolated_properties[i] == "viscosity")
      for (unsigned int p = 0; p < _num_p; ++p)
        for (unsigned int t = 0; t < _num_T; ++t)
        {
          if (_fp)
            _properties[i][p * _num_T + t] = _fp->mu_from_p_T(_pressure[p], _temperature[t]);
          else
            paramError("fp", "No fluid properties or csv data provided for viscosity.");
        }

    if (_interpolated_properties[i] == "k")
      for (unsigned int p = 0; p < _num_p; ++p)
        for (unsigned int t = 0; t < _num_T; ++t)
        {
          if (_fp)
            _properties[i][p * _num_T + t] = _fp->k_from_p_T(_pressure[p], _temperature[t]);
          else
            paramError("interpolated_properties",
                       "No data to interpolate for thermal conductivity.");
        }

    if (_interpolated_properties[i] == "c")
      for (unsigned int p = 0; p < _num_p; ++p)
        for (unsigned int t = 0; t < _num_T; ++t)
        {
          if (_fp)
            _properties[i][p * _num_T + t] = _fp->c_from_p_T(_pressure[p], _temperature[t]);
          else
            paramError("interpolated_properties", "No data to interpolate for speed of sound.");
        }

    if (_interpolated_properties[i] == "cv")
      for (unsigned int p = 0; p < _num_p; ++p)
        for (unsigned int t = 0; t < _num_T; ++t)
        {
          if (_fp)
            _properties[i][p * _num_T + t] = _fp->cv_from_p_T(_pressure[p], _temperature[t]);
          else
            paramError("interpolated_properties",
                       "No data to interpolate for specific heat capacity at constant volume.");
        }

    if (_interpolated_properties[i] == "cp")
      for (unsigned int p = 0; p < _num_p; ++p)
        for (unsigned int t = 0; t < _num_T; ++t)
        {
          if (_fp)
            _properties[i][p * _num_T + t] = _fp->cp_from_p_T(_pressure[p], _temperature[t]);
          else
            paramError("interpolated_properties",
                       "No data to interpolate for specific heat capacity at constant pressure.");
        }

    if (_interpolated_properties[i] == "entropy")
      for (unsigned int p = 0; p < _num_p; ++p)
        for (unsigned int t = 0; t < _num_T; ++t)
        {
          if (_fp)
            _properties[i][p * _num_T + t] = _fp->s_from_p_T(_pressure[p], _temperature[t]);
          else
            paramError("interpolated_properties", "No data to interpolate for entropy.");
        }
  }
}

template <typename T>
void
TabulatedFluidProperties::checkInputVariables(T & pressure, T & temperature) const
{
  if (pressure < _pressure_min || pressure > _pressure_max)
  {
    if (_error_on_out_of_bounds)
      throw MooseException("Pressure " + Moose::stringify(pressure) +
                           " is outside the range of tabulated pressure (" +
                           Moose::stringify(_pressure_min) + ", " +
                           Moose::stringify(_pressure_max) + ").");
    else
      pressure = std::max(_pressure_min, std::min(pressure, _pressure_max));
  }

  if (temperature < _temperature_min || temperature > _temperature_max)
  {
    if (_error_on_out_of_bounds)
      mooseError("Temperature " + Moose::stringify(temperature) +
                 " is outside the range of tabulated temperature (" +
                 Moose::stringify(_temperature_min) + ", " + Moose::stringify(_temperature_max) +
                 ").");
    else
      temperature = std::max(T(_temperature_min), std::min(temperature, T(_temperature_max)));
  }
}

void
TabulatedFluidProperties::checkInitialGuess() const
{
  if (_construct_pT_from_ve || _construct_pT_from_vh)
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

template void TabulatedFluidProperties::checkInputVariables(Real & pressure,
                                                            Real & temperature) const;

template void TabulatedFluidProperties::checkInputVariables(ADReal & pressure,
                                                            ADReal & temperature) const;
