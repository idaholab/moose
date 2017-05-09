/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "TabulatedFluidProperties.h"
#include "BicubicSplineInterpolation.h"
#include "MooseUtils.h"

// C++ includes
#include <fstream>
#include <ctime>

template <>
InputParameters
validParams<TabulatedFluidProperties>()
{
  InputParameters params = validParams<SinglePhaseFluidPropertiesPT>();
  params.addParam<FileName>("fluid_property_file",
                            "fluid_properties.txt",
                            "Name of the file containing the tabulated fluid property data");
  params.addRangeCheckedParam<Real>("temperature_min",
                                    300.0,
                                    "temperature_min >= 273.15",
                                    "Minimum temperature for tabulated data. Default is 300 K)");
  params.addRangeCheckedParam<Real>("temperature_max",
                                    500.0,
                                    "temperature_max <= 1000.0",
                                    "Maximum temperature for tabulated data. Default is 500 K");
  params.addRangeCheckedParam<Real>("pressure_min",
                                    1.0e5,
                                    "pressure_min >= 1.0e5",
                                    "Minimum pressure for tabulated data. Default is 0.1 MPa)");
  params.addRangeCheckedParam<Real>("pressure_max",
                                    50.0e6,
                                    "pressure_max <= 800.0e6",
                                    "Maximum pressure for tabulated data. Default is 50 MPa");
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
    _fp(getUserObject<SinglePhaseFluidPropertiesPT>("fp"))
{
  // Sanity check on minimum and maximum temperatures and pressures
  if (_temperature_max <= _temperature_min)
    mooseError("temperature_max must be greater than temperature_min in ", name());
  if (_pressure_max <= _pressure_min)
    mooseError("pressure_max must be greater than pressure_min in ", name());
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
    parseTabulatedData(_file_name);

    // If any required tabulated data is missing (eg, the data file did not include
    // one of the required properties), then generate property data for that property
    // so that spline interpolation can be used
    generateMissingTabulatedData();
  }
  else
  {
    _console << "No tabulated properties file named " << _file_name << " exists.\n"
             << "Generating tabulated data and writing output to " << _file_name << "\n";
    generateAllTabulatedData();
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
TabulatedFluidProperties::mu(Real density, Real temperature) const
{
  return _fp.mu(density, temperature);
}

void
TabulatedFluidProperties::mu_drhoT(
    Real density, Real temperature, Real & mu, Real & dmu_drho, Real & dmu_dT) const
{
  _fp.mu_drhoT(density, temperature, mu, dmu_drho, dmu_dT);
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
TabulatedFluidProperties::k(Real density, Real temperature) const
{
  return _fp.k(density, temperature);
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
TabulatedFluidProperties::parseTabulatedData(std::string file_name)
{
  MooseUtils::checkFileReadable(file_name);

  /// List of axes names read from file
  std::vector<std::string> axes_names;
  /// Data for axes
  std::vector<std::vector<Real>> axes;
  /// List of property names read from file
  std::vector<std::string> property_names;
  /// Data for each fluid property
  std::vector<std::vector<Real>> properties;

  std::ifstream file_in(file_name.c_str());
  std::string line;

  while (getline(file_in, line))
  {
    // Skip empty lines
    if (line.empty())
      continue;

    // Skip lines containing whitespace only
    if (std::all_of(line.begin(), line.end(), isspace))
      continue;

    // Skip lines starting with # (treat as comments)
    if (line.find("#") == 0)
      continue;

    // Found an axes keyword
    else if (std::find(_required_axes.begin(), _required_axes.end(), line) != _required_axes.end())
    {
      auto it = std::find(_required_axes.begin(), _required_axes.end(), line);
      auto index = std::distance(_required_axes.begin(), it);
      axes_names.push_back(_required_axes[index]);

      std::vector<Real> data;
      while (getline(file_in, line))
      {
        if (!line.empty())
          parseData(line, data);
        else
          break;
      }
      axes.push_back(data);
    }

    // Found a fluid properties keyword
    else if (std::find(_valid_props.begin(), _valid_props.end(), line) != _valid_props.end())
    {
      auto it = std::find(_valid_props.begin(), _valid_props.end(), line);
      auto index = std::distance(_valid_props.begin(), it);
      property_names.push_back(_valid_props[index]);

      std::vector<Real> data;

      while (getline(file_in, line))
      {
        if (!line.empty())
          parseData(line, data);
        else
          break;
      }
      properties.push_back(data);
    }

    // Found an invalid keyword, so ignore data
    else if (std::find(_valid_props.begin(), _valid_props.end(), line) == _valid_props.end())
    {
      while (getline(file_in, line))
        if (!line.empty())
          continue;
        else
          break;
    }
  }

  // Check that axes data has been provided in ascending order
  for (unsigned int i = 0; i < axes.size(); ++i)
    if (!std::is_sorted(axes[i].begin(), axes[i].end()))
      mooseError(
          "The axes data for ", axes_names[i], " is not monotonically ascending in ", file_name);

  // Check that the correct number of data points have been provided for each property
  unsigned int num_data = 1;
  for (auto ax : axes)
    num_data *= ax.size();

  for (unsigned int i = 0; i < properties.size(); ++i)
    if (properties[i].size() != num_data)
      mooseError(
          "The number of supplied ", property_names[i], " values is not correct in ", file_name);

  for (unsigned int i = 0; i < axes_names.size(); ++i)
  {
    if (axes_names[i] == "pressure")
      _pressure = axes[i];

    else // (axes_names[i] == "temperature")
      _temperature = axes[i];
  }

  // Both pressure and temperature data must be provided
  if (_pressure.empty())
    mooseError("No pressure axes data read in ", file_name);
  if (_temperature.empty())
    mooseError("No temperature axes data read in ", file_name);

  // Number of pressure and temperature data points
  _num_T = _temperature.size();
  _num_p = _pressure.size();

  // Minimum and maximum pressure and temperature. Note that _pressure and
  // _temperature are sorted
  _pressure_min = _pressure.front();
  _pressure_max = _pressure.back();
  _temperature_min = _temperature.front();
  _temperature_max = _temperature.back();

  for (unsigned int i = 0; i < property_names.size(); ++i)
  {
    if (property_names[i] == "density")
      reshapeData2D(_num_p, _num_T, properties[i], _density);

    else if (property_names[i] == "internal_energy")
      reshapeData2D(_num_p, _num_T, properties[i], _internal_energy);

    else // (property_names[i] == "enthalpy")
      reshapeData2D(_num_p, _num_T, properties[i], _enthalpy);
  }
}

void
TabulatedFluidProperties::parseData(std::string line, std::vector<Real> & data)
{
  std::istringstream linestream(line);
  std::string item;

  while (getline(linestream, item, ' '))
  {
    std::istringstream iss(item);
    Real value;
    iss >> value;
    data.push_back(value);
  }
}

void
TabulatedFluidProperties::writeTabulatedData(std::string file_name)
{
  MooseUtils::checkFileWriteable(file_name);

  std::ofstream file_out(file_name.c_str());

  std::vector<std::string> axes_names{"pressure", "temperature"};
  std::vector<std::vector<Real>> axes_data;
  axes_data.push_back(_pressure);
  axes_data.push_back(_temperature);

  std::vector<std::string> property_names{"density", "internal_energy", "enthalpy"};
  std::vector<std::vector<Real>> property_data;
  property_data.push_back(flattenData(_density));
  property_data.push_back(flattenData(_internal_energy));
  property_data.push_back(flattenData(_enthalpy));

  // Write out date and object created
  time_t now = time(&now);
  file_out << "# Created by TabulatedFluidProperties on " << ctime(&now) << "\n";

  // Write out axes
  for (unsigned int i = 0; i < axes_names.size(); ++i)
  {
    file_out << axes_names[i] << "\n";
    for (auto item : axes_data[i])
      file_out << item << "\n";

    file_out << "\n";
  }

  // Write out the fluid property data
  for (unsigned int i = 0; i < property_names.size(); ++i)
  {
    file_out << property_names[i] << "\n";

    unsigned int item_count = 0;
    for (auto item : property_data[i])
    {
      file_out << item << " ";
      ++item_count;
      if (item_count % 10 == 0)
        file_out << "\n";
    }
    file_out << "\n";
  }
}

void
TabulatedFluidProperties::generateAllTabulatedData()
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
TabulatedFluidProperties::generateMissingTabulatedData()
{
  // Generate tabulated data for any property that is missing
  if (_density.empty())
  {
    _density.resize(_num_p);
    for (unsigned int i = 0; i < _num_p; ++i)
      for (unsigned int j = 0; j < _num_T; ++j)
        _density[i].push_back(_fp.rho(_pressure[i], _temperature[j]));
  }

  if (_internal_energy.empty())
  {
    _internal_energy.resize(_num_p);
    for (unsigned int i = 0; i < _num_p; ++i)
      for (unsigned int j = 0; j < _num_T; ++j)
        _internal_energy[i].push_back(_fp.e(_pressure[i], _temperature[j]));
  }

  if (_enthalpy.empty())
  {
    _enthalpy.resize(_num_p);
    for (unsigned int i = 0; i < _num_p; ++i)
      for (unsigned int j = 0; j < _num_T; ++j)
        _enthalpy[i].push_back(_fp.h(_pressure[i], _temperature[j]));
  }
}

void
TabulatedFluidProperties::reshapeData2D(unsigned int nrow,
                                        unsigned int ncol,
                                        std::vector<Real> & vec,
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

std::vector<Real>
TabulatedFluidProperties::flattenData(std::vector<std::vector<Real>> & mat)
{
  std::vector<Real> vec;

  if (!mat.empty())
  {
    const unsigned int n = mat.size();
    const unsigned int m = mat[0].size();

    for (unsigned int i = 0; i < n; ++i)
      for (unsigned int j = 0; j < m; ++j)
        vec.push_back(mat[i][j]);
  }
  return vec;
}

void
TabulatedFluidProperties::checkInputVariables(Real pressure, Real temperature) const
{
  if (pressure < _pressure_min || pressure > _pressure_max)
    mooseError("Pressure ",
               pressure,
               " is outside the range of tabulated pressure (",
               _pressure_min,
               ", ",
               _pressure_max,
               ".");

  if (temperature < _temperature_min || temperature > _temperature_max)
    mooseError("Temperature ",
               temperature,
               " is outside the range of tabulated temperature (",
               _temperature_min,
               ", ",
               _temperature_max,
               ".");
}
