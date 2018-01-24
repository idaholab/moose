//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TabulatedFluidProperties.h"
#include "BicubicSplineInterpolation.h"
#include "MooseUtils.h"
#include "Conversion.h"

// C++ includes
#include <fstream>
#include <ctime>

template <>
InputParameters
validParams<TabulatedFluidProperties>()
{
  InputParameters params = validParams<SinglePhaseFluidPropertiesPT>();
  params.addParam<FileName>("fluid_property_file",
                            "fluid_properties.csv",
                            "Name of the csv file containing the tabulated fluid property data");
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
  params.addClassDescription(
      "Fluid properties using bicubic spline interpolation on tabulated values provided");
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
    _csv_reader(_file_name, &_communicator)
{
  // Sanity check on minimum and maximum temperatures and pressures
  if (_temperature_max <= _temperature_min)
    mooseError("temperature_max must be greater than temperature_min in ", name());
  if (_pressure_max <= _pressure_min)
    mooseError("pressure_max must be greater than pressure_min in ", name());

  // Lines starting with # are treated as comments
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
        mooseError("No ",
                   _required_columns[i],
                   " data read in ",
                   _file_name,
                   ". A column named ",
                   _required_columns[i],
                   " must be present");
    }

    std::map<std::string, unsigned int> data_index;
    for (std::size_t i = 0; i < _required_columns.size(); ++i)
    {
      auto it = std::find(column_names.begin(), column_names.end(), _required_columns[i]);
      data_index[_required_columns[i]] = std::distance(column_names.begin(), it);
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

    // Extract the fluid property data and reshape into 2D arrays for interpolation
    reshapeData2D(_num_p, _num_T, column_data[data_index.find("density")->second], _density);
    reshapeData2D(_num_p, _num_T, column_data[data_index.find("enthalpy")->second], _enthalpy);
    reshapeData2D(
        _num_p, _num_T, column_data[data_index.find("internal_energy")->second], _internal_energy);
  }
  else
  {
    _console << "No tabulated properties file named " << _file_name << " exists.\n"
             << "Generating tabulated data and writing output to " << _file_name << "\n";

    generateTabulatedData();

    // Write tabulated data to file
    writeTabulatedData(_file_name);
  }

  // Construct bicubic splines from tabulated data
  _density_ipol = libmesh_make_unique<BicubicSplineInterpolation>();
  _internal_energy_ipol = libmesh_make_unique<BicubicSplineInterpolation>();
  _enthalpy_ipol = libmesh_make_unique<BicubicSplineInterpolation>();

  _density_ipol->setData(
      _pressure, _temperature, _density, _drho_dp_0, _drho_dp_n, _drho_dT_0, _drho_dT_n);

  _internal_energy_ipol->setData(
      _pressure, _temperature, _internal_energy, _de_dp_0, _de_dp_n, _de_dT_0, _de_dT_n);

  _enthalpy_ipol->setData(
      _pressure, _temperature, _enthalpy, _dh_dp_0, _dh_dp_n, _dh_dT_0, _dh_dT_n);
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
TabulatedFluidProperties::rho(Real pressure, Real temperature) const
{
  checkInputVariables(pressure, temperature);
  return _density_ipol->sample(pressure, temperature);
}

void
TabulatedFluidProperties::rho_dpT(
    Real pressure, Real temperature, Real & rho, Real & drho_dp, Real & drho_dT) const
{
  checkInputVariables(pressure, temperature);
  rho = _density_ipol->sample(pressure, temperature);
  drho_dp = _density_ipol->sampleDerivative(pressure, temperature, _wrt_p);
  drho_dT = _density_ipol->sampleDerivative(pressure, temperature, _wrt_T);
}

Real
TabulatedFluidProperties::e(Real pressure, Real temperature) const
{
  checkInputVariables(pressure, temperature);
  return _internal_energy_ipol->sample(pressure, temperature);
}

void
TabulatedFluidProperties::e_dpT(
    Real pressure, Real temperature, Real & e, Real & de_dp, Real & de_dT) const
{
  checkInputVariables(pressure, temperature);
  e = _internal_energy_ipol->sample(pressure, temperature);
  de_dp = _internal_energy_ipol->sampleDerivative(pressure, temperature, _wrt_p);
  de_dT = _internal_energy_ipol->sampleDerivative(pressure, temperature, _wrt_T);
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
  checkInputVariables(pressure, temperature);
  rho_dpT(pressure, temperature, rho, drho_dp, drho_dT);
  e_dpT(pressure, temperature, e, de_dp, de_dT);
}

Real
TabulatedFluidProperties::h(Real pressure, Real temperature) const
{
  checkInputVariables(pressure, temperature);
  return _enthalpy_ipol->sample(pressure, temperature);
}

void
TabulatedFluidProperties::h_dpT(
    Real pressure, Real temperature, Real & h, Real & dh_dp, Real & dh_dT) const
{
  checkInputVariables(pressure, temperature);
  h = _enthalpy_ipol->sample(pressure, temperature);
  dh_dp = _enthalpy_ipol->sampleDerivative(pressure, temperature, _wrt_p);
  dh_dT = _enthalpy_ipol->sampleDerivative(pressure, temperature, _wrt_T);
}

Real
TabulatedFluidProperties::mu(Real pressure, Real temperature) const
{
  Real rho = this->rho(pressure, temperature);
  return this->mu_from_rho_T(rho, temperature);
}

void
TabulatedFluidProperties::mu_dpT(
    Real pressure, Real temperature, Real & mu, Real & dmu_dp, Real & dmu_dT) const
{
  Real rho, drho_dp, drho_dT;
  this->rho_dpT(pressure, temperature, rho, drho_dp, drho_dT);
  Real dmu_drho;
  this->mu_drhoT_from_rho_T(rho, temperature, drho_dT, mu, dmu_drho, dmu_dT);
  dmu_dp = dmu_drho * drho_dp;
}

Real
TabulatedFluidProperties::mu_from_rho_T(Real density, Real temperature) const
{
  return _fp.mu_from_rho_T(density, temperature);
}

void
TabulatedFluidProperties::mu_drhoT_from_rho_T(Real density,
                                              Real temperature,
                                              Real ddensity_dT,
                                              Real & mu,
                                              Real & dmu_drho,
                                              Real & dmu_dT) const
{
  _fp.mu_drhoT_from_rho_T(density, temperature, ddensity_dT, mu, dmu_drho, dmu_dT);
}

Real
TabulatedFluidProperties::c(Real pressure, Real temperature) const
{
  return _fp.c(pressure, temperature);
}

Real
TabulatedFluidProperties::cp(Real pressure, Real temperature) const
{
  return _fp.cp(pressure, temperature);
}

Real
TabulatedFluidProperties::cv(Real pressure, Real temperature) const
{
  return _fp.cv(pressure, temperature);
}

Real
TabulatedFluidProperties::k(Real pressure, Real temperature) const
{
  Real rho = this->rho(pressure, temperature);
  return this->k_from_rho_T(rho, temperature);
}

void
TabulatedFluidProperties::k_dpT(
    Real /*pressure*/, Real /*temperature*/, Real & /*k*/, Real & /*dk_dp*/, Real & /*dk_dT*/) const
{
  mooseError(name(), "k_dpT() is not implemented");
}

Real
TabulatedFluidProperties::k_from_rho_T(Real density, Real temperature) const
{
  return _fp.k_from_rho_T(density, temperature);
}

Real
TabulatedFluidProperties::s(Real pressure, Real temperature) const
{
  return _fp.s(pressure, temperature);
}

Real
TabulatedFluidProperties::beta(Real pressure, Real temperature) const
{
  return _fp.beta(pressure, temperature);
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

    std::string column_names{"pressure, temperature, density, enthalpy, internal_energy"};

    // Write out column names
    file_out << column_names << "\n";

    // Write out the fluid property data
    for (unsigned int i = 0; i < _num_p; ++i)
      for (unsigned int j = 0; j < _num_T; ++j)
        file_out << _pressure[i] << ", " << _temperature[j] << ", " << _density[i][j] << ", "
                 << _enthalpy[i][j] << ", " << _internal_energy[i][j] << "\n";
  }
}

void
TabulatedFluidProperties::generateTabulatedData()
{
  _pressure.resize(_num_p);
  _temperature.resize(_num_T);

  _density.resize(_num_p);
  _internal_energy.resize(_num_p);
  _enthalpy.resize(_num_p);

  for (unsigned int i = 0; i < _num_p; ++i)
  {
    _density[i].resize(_num_T);
    _internal_energy[i].resize(_num_T);
    _enthalpy[i].resize(_num_T);
  }

  // Temperature is divided equally into _num_T segments
  Real delta_T = (_temperature_max - _temperature_min) / static_cast<Real>(_num_T - 1);

  for (unsigned int j = 0; j < _num_T; ++j)
    _temperature[j] = _temperature_min + j * delta_T;

  // Divide the pressure into _num_p equal segments
  Real delta_p = (_pressure_max - _pressure_min) / static_cast<Real>(_num_p - 1);

  for (unsigned int i = 0; i < _num_p; ++i)
    _pressure[i] = _pressure_min + i * delta_p;

  // Generate the tabulated data at the pressure and temperature points
  for (unsigned int i = 0; i < _num_p; ++i)
    for (unsigned int j = 0; j < _num_T; ++j)
    {
      _density[i][j] = _fp.rho(_pressure[i], _temperature[j]);
      _internal_energy[i][j] = _fp.e(_pressure[i], _temperature[j]);
      _enthalpy[i][j] = _fp.h(_pressure[i], _temperature[j]);
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
      for (unsigned int j = 0; j < ncol; ++j)
        mat[i].push_back(vec[i * ncol + j]);
  }
}

void
TabulatedFluidProperties::checkInputVariables(Real & pressure, Real & temperature) const
{
  if (pressure < _pressure_min || pressure > _pressure_max)
    throw MooseException(
        "Pressure " + Moose::stringify(pressure) + " is outside the range of tabulated pressure (" +
        Moose::stringify(_pressure_min) + ", " + Moose::stringify(_pressure_max) + ".");

  if (temperature < _temperature_min || temperature > _temperature_max)
    throw MooseException("Temperature " + Moose::stringify(temperature) +
                         " is outside the range of tabulated temperature (" +
                         Moose::stringify(_temperature_min) + ", " +
                         Moose::stringify(_temperature_max) + ".");
}
