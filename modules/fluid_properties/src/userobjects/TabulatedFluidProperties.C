//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TabulatedFluidProperties.h"
#include "BicubicInterpolation.h"
#include "MooseUtils.h"
#include "Conversion.h"

// C++ includes
#include <fstream>
#include <ctime>

registerMooseObject("FluidPropertiesApp", TabulatedFluidProperties);

template <>
InputParameters
validParams<TabulatedFluidProperties>()
{
  InputParameters params = validParams<SinglePhaseFluidPropertiesPT>();
  params.addParam<FileName>(
      "fluid_property_file",
      "fluid_properties.csv",
      "Name of the csv file containing the tabulated fluid property data. If no file exists, then "
      "one will be written using the temperature and pressure range specified.");
  params.addRangeCheckedParam<Real>("temperature_min",
                                    300.0,
                                    "temperature_min > 0",
                                    "Minimum temperature for tabulated data. Default is 300 K)");
  params.addParam<Real>(
      "temperature_max", 500.0, "Maximum temperature for tabulated data. Default is 500 K");
  params.addRangeCheckedParam<Real>("pressure_min",
                                    1.0e5,
                                    "pressure_min > 0",
                                    "Minimum pressure for tabulated data. Default is 0.1 MPa)");
  params.addParam<Real>(
      "pressure_max", 50.0e6, "Maximum pressure for tabulated data. Default is 50 MPa");
  params.addRangeCheckedParam<unsigned int>(
      "num_T", 100, "num_T > 0", "Number of points to divide temperature range. Default is 100");
  params.addRangeCheckedParam<unsigned int>(
      "num_p", 100, "num_p > 0", "Number of points to divide pressure range. Default is 100");
  params.addRequiredParam<UserObjectName>("fp", "The name of the FluidProperties UserObject");
  MultiMooseEnum properties("density enthalpy internal_energy viscosity k cv cp entropy",
                            "density enthalpy internal_energy");
  params.addParam<MultiMooseEnum>("interpolated_properties",
                                  properties,
                                  "Properties to interpolate if no data file is provided");
  params.addClassDescription(
      "Fluid properties using bicubic interpolation on tabulated values provided");
  return params;
}

TabulatedFluidProperties::TabulatedFluidProperties(const InputParameters & parameters)
  : SinglePhaseFluidPropertiesPT(parameters),
    _file_name(getParam<FileName>("fluid_property_file")),
    _temperature_min(getParam<Real>("temperature_min")),
    _temperature_max(getParam<Real>("temperature_max")),
    _pressure_min(getParam<Real>("pressure_min")),
    _pressure_max(getParam<Real>("pressure_max")),
    _num_T(getParam<unsigned int>("num_T")),
    _num_p(getParam<unsigned int>("num_p")),
    _fp(getUserObject<SinglePhaseFluidPropertiesPT>("fp")),
    _interpolated_properties_enum(getParam<MultiMooseEnum>("interpolated_properties")),
    _interpolated_properties(),
    _interpolate_density(false),
    _interpolate_enthalpy(false),
    _interpolate_internal_energy(false),
    _interpolate_viscosity(false),
    _interpolate_k(false),
    _interpolate_cp(false),
    _interpolate_cv(false),
    _interpolate_entropy(false),
    _density_idx(0),
    _enthalpy_idx(0),
    _internal_energy_idx(0),
    _viscosity_idx(0),
    _k_idx(0),
    _cp_idx(0),
    _cv_idx(0),
    _entropy_idx(0),
    _csv_reader(_file_name, &_communicator)
{
  // Sanity check on minimum and maximum temperatures and pressures
  if (_temperature_max <= _temperature_min)
    mooseError(name(), ": temperature_max must be greater than temperature_min");
  if (_pressure_max <= _pressure_min)
    mooseError(name(), ": pressure_max must be greater than pressure_min");

  // Lines starting with # in the data file are treated as comments
  _csv_reader.setComment("#");
}

TabulatedFluidProperties::~TabulatedFluidProperties() {}

void
TabulatedFluidProperties::initialSetup()
{
  // Check to see if _file_name supplied exists. If it does, that data
  // will be used. If it does not exist, data will be generated and then
  // written to _file_name.
  std::ifstream file(_file_name.c_str());
  if (file.good())
  {
    _console << "Reading tabulated properties from " << _file_name << "\n";
    _csv_reader.read();

    const std::vector<std::string> & column_names = _csv_reader.getNames();

    // Check that all required columns are present
    for (std::size_t i = 0; i < _required_columns.size(); ++i)
    {
      if (std::find(column_names.begin(), column_names.end(), _required_columns[i]) ==
          column_names.end())
        mooseError(name(),
                   ": no ",
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
          mooseError(name(),
                     ": ",
                     column_names[i],
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
      mooseError(
          name(), ": the column data for pressure is not monotonically increasing in ", _file_name);

    // The first pressure value is repeated for each temperature value. Counting the
    // number of repeats provides the number of temperature values
    auto num_T = std::count(_pressure.begin(), _pressure.end(), _pressure.front());

    // Now remove the duplicates in the pressure vector
    auto last_unique = std::unique(_pressure.begin(), _pressure.end());
    _pressure.erase(last_unique, _pressure.end());
    _num_p = _pressure.size();

    // Check that the number of rows in the csv file is equal to _num_p * _num_T
    if (column_data[0].size() != _num_p * static_cast<unsigned int>(num_T))
      mooseError(name(),
                 ": the number of rows in ",
                 _file_name,
                 " is not equal to the number of unique pressure values ",
                 _num_p,
                 " multiplied by the number of unique temperature values ",
                 num_T);

    // Need to make sure that the temperature values are provided in ascending order
    // as well as duplicated for each pressure value
    std::vector<Real> temp0(_temperature.begin(), _temperature.begin() + num_T);
    if (!std::is_sorted(temp0.begin(), temp0.end()))
      mooseError(name(),
                 ": the column data for temperature is not monotonically increasing in ",
                 _file_name);

    auto it_temp = _temperature.begin() + num_T;
    for (std::size_t i = 1; i < _pressure.size(); ++i)
    {
      std::vector<Real> temp(it_temp, it_temp + num_T);
      if (temp != temp0)
        mooseError(name(),
                   ": temperature values for pressure ",
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
    _console << "No tabulated properties file named " << _file_name << " exists.\n"
             << "Generating tabulated data and writing output to " << _file_name << "\n";

    generateTabulatedData();

    // Write tabulated data to file
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

  // Construct bicubic interpolants from tabulated data
  std::vector<std::vector<Real>> data_matrix;
  _property_ipol.resize(_properties.size());

  for (std::size_t i = 0; i < _property_ipol.size(); ++i)
  {
    reshapeData2D(_num_p, _num_T, _properties[i], data_matrix);
    _property_ipol[i] =
        libmesh_make_unique<BicubicInterpolation>(_pressure, _temperature, data_matrix);
  }
}

std::string
TabulatedFluidProperties::fluidName() const
{
  return _fp.fluidName();
}

Real
TabulatedFluidProperties::molarMass() const
{
  return _fp.molarMass();
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
    return _fp.rho_from_p_T(pressure, temperature);
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
    _fp.rho_from_p_T(pressure, temperature, rho, drho_dp, drho_dT);
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
    return _fp.e_from_p_T(pressure, temperature);
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
    _fp.e_from_p_T(pressure, temperature, e, de_dp, de_dT);
}

void
TabulatedFluidProperties::rho_e_dpT(Real pressure,
                                    Real temperature,
                                    Real & rho,
                                    Real & drho_dp,
                                    Real & drho_dT,
                                    Real & e,
                                    Real & de_dp,
                                    Real & de_dT) const
{
  rho_from_p_T(pressure, temperature, rho, drho_dp, drho_dT);
  e_from_p_T(pressure, temperature, e, de_dp, de_dT);
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
    return _fp.h_from_p_T(pressure, temperature);
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
    _fp.h_from_p_T(pressure, temperature, h, dh_dp, dh_dT);
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
    return _fp.mu_from_p_T(pressure, temperature);
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
    return _fp.mu_from_p_T(pressure, temperature, mu, dmu_dp, dmu_dT);
}

void
TabulatedFluidProperties::rho_mu(Real pressure, Real temperature, Real & rho, Real & mu) const
{
  rho = this->rho_from_p_T(pressure, temperature);
  mu = this->mu_from_p_T(pressure, temperature);
}

void
TabulatedFluidProperties::rho_mu_dpT(Real pressure,
                                     Real temperature,
                                     Real & rho,
                                     Real & drho_dp,
                                     Real & drho_dT,
                                     Real & mu,
                                     Real & dmu_dp,
                                     Real & dmu_dT) const
{
  rho_from_p_T(pressure, temperature, rho, drho_dp, drho_dT);
  mu_from_p_T(pressure, temperature, mu, dmu_dp, dmu_dT);
}

Real
TabulatedFluidProperties::c_from_p_T(Real pressure, Real temperature) const
{
  return _fp.c_from_p_T(pressure, temperature);
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
    return _fp.cp_from_p_T(pressure, temperature);
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
    return _fp.cv_from_p_T(pressure, temperature);
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
    return _fp.k_from_p_T(pressure, temperature);
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
    return _fp.k_from_p_T(pressure, temperature, k, dk_dp, dk_dT);
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
    return _fp.s_from_p_T(pressure, temperature);
}

void
TabulatedFluidProperties::s_from_p_T(Real p, Real T, Real & s, Real & ds_dp, Real & ds_dT) const
{
  SinglePhaseFluidProperties::s_from_p_T(p, T, s, ds_dp, ds_dT);
}

Real
TabulatedFluidProperties::henryConstant(Real temperature) const
{
  return _fp.henryConstant(temperature);
}

void
TabulatedFluidProperties::henryConstant_dT(Real temperature, Real & Kh, Real & dKh_dT) const
{
  _fp.henryConstant_dT(temperature, Kh, dKh_dT);
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
    file_out << "# " << _fp.fluidName() << " properties created by TabulatedFluidProperties on "
             << ctime(&now) << "\n";

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
          _properties[i][p * _num_T + t] = _fp.rho_from_p_T(_pressure[p], _temperature[t]);

    if (_interpolated_properties[i] == "enthalpy")
      for (unsigned int p = 0; p < _num_p; ++p)
        for (unsigned int t = 0; t < _num_T; ++t)
          _properties[i][p * _num_T + t] = _fp.h_from_p_T(_pressure[p], _temperature[t]);

    if (_interpolated_properties[i] == "internal_energy")
      for (unsigned int p = 0; p < _num_p; ++p)
        for (unsigned int t = 0; t < _num_T; ++t)
          _properties[i][p * _num_T + t] = _fp.e_from_p_T(_pressure[p], _temperature[t]);

    if (_interpolated_properties[i] == "viscosity")
      for (unsigned int p = 0; p < _num_p; ++p)
        for (unsigned int t = 0; t < _num_T; ++t)
          _properties[i][p * _num_T + t] = _fp.mu_from_p_T(_pressure[p], _temperature[t]);

    if (_interpolated_properties[i] == "k")
      for (unsigned int p = 0; p < _num_p; ++p)
        for (unsigned int t = 0; t < _num_T; ++t)
          _properties[i][p * _num_T + t] = _fp.k_from_p_T(_pressure[p], _temperature[t]);

    if (_interpolated_properties[i] == "cv")
      for (unsigned int p = 0; p < _num_p; ++p)
        for (unsigned int t = 0; t < _num_T; ++t)
          _properties[i][p * _num_T + t] = _fp.cv_from_p_T(_pressure[p], _temperature[t]);

    if (_interpolated_properties[i] == "cp")
      for (unsigned int p = 0; p < _num_p; ++p)
        for (unsigned int t = 0; t < _num_T; ++t)
          _properties[i][p * _num_T + t] = _fp.cp_from_p_T(_pressure[p], _temperature[t]);

    if (_interpolated_properties[i] == "entropy")
      for (unsigned int p = 0; p < _num_p; ++p)
        for (unsigned int t = 0; t < _num_T; ++t)
          _properties[i][p * _num_T + t] = _fp.s_from_p_T(_pressure[p], _temperature[t]);
  }
}

void
TabulatedFluidProperties::reshapeData2D(unsigned int nrow,
                                        unsigned int ncol,
                                        const std::vector<Real> & vec,
                                        std::vector<std::vector<Real>> & mat)
{
  if (!vec.empty())
  {
    mat.resize(nrow);
    for (unsigned int i = 0; i < nrow; ++i)
      mat[i].resize(ncol);

    for (unsigned int i = 0; i < nrow; ++i)
      for (unsigned int j = 0; j < ncol; ++j)
        mat[i][j] = vec[i * ncol + j];
  }
}

void
TabulatedFluidProperties::checkInputVariables(Real & pressure, Real & temperature) const
{
  if (pressure < _pressure_min || pressure > _pressure_max)
    throw MooseException(
        "Pressure " + Moose::stringify(pressure) + " is outside the range of tabulated pressure (" +
        Moose::stringify(_pressure_min) + ", " + Moose::stringify(_pressure_max) + ").");

  if (temperature < _temperature_min || temperature > _temperature_max)
    throw MooseException("Temperature " + Moose::stringify(temperature) +
                         " is outside the range of tabulated temperature (" +
                         Moose::stringify(_temperature_min) + ", " +
                         Moose::stringify(_temperature_max) + ").");
}
