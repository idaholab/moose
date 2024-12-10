//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TriInterWrapperPowerIC.h"
#include "Function.h"
#include "TriInterWrapperMesh.h"

registerMooseObject("SubChannelApp", TriInterWrapperPowerIC);

InputParameters
TriInterWrapperPowerIC::validParams()
{
  InputParameters params = TriInterWrapperBaseIC::validParams();
  params.addClassDescription("Computes linear power rate (W/m) that goes into interwrapper cells "
                             "in a triangular subchannel lattice");
  params.addParam<Real>("power", 0.0, "The total heating power [W]");
  params.addParam<std::string>("filename",
                               "file_was_not_found",
                               "name of power profile .txt file (should be a single column). It's "
                               "a Radial Power Profile. [UnitLess]");
  params.addParam<FunctionName>("axial_heat_rate",
                                "1.0",
                                "user provided normalized function of axial heat rate [Unitless]. "
                                "The integral over pin length should equal the heated length");
  return params;
}

TriInterWrapperPowerIC::TriInterWrapperPowerIC(const InputParameters & params)
  : TriInterWrapperBaseIC(params),
    _power(getParam<Real>("power")),
    _numberoflines(0),
    _filename(getParam<std::string>("filename")),
    _axial_heat_rate(getFunction("axial_heat_rate"))
{
  auto n_rods = _mesh.getNumOfRods();

  _power_dis.resize(n_rods);
  _pin_power_correction.resize(n_rods);
  _ref_power.resize(n_rods);
  _ref_qprime.resize(n_rods);
  _estimate_power.resize(n_rods);
  for (unsigned int i = 0; i < n_rods; i++)
  {
    _power_dis[i] = 0.0;
    _pin_power_correction[i] = 1.0;
  }

  if (_filename.compare("file_was_not_found"))
  {

    Real vin;
    std::ifstream inFile;

    while (inFile >> vin)
      _numberoflines += 1;

    inFile.open(_filename);
    if (!inFile)
      mooseError(name(), ": Unable to open file: ", _filename);

    if (inFile.fail() && !inFile.eof())
      mooseError(name(), ": Non numerical input at line: ", _numberoflines);

    if (_numberoflines != n_rods)
      mooseError(name(), ": Radial profile file doesn't have correct size: ", n_rods);
    inFile.close();

    inFile.open(_filename);
    int i = 0;
    while (inFile >> vin)
    {
      _power_dis[i] = vin;
      i++;
    }
    inFile.close();
  }
  Real sum = 0.0;

  for (unsigned int i = 0; i < n_rods; i++)
  {
    sum = sum + _power_dis[i];
  }
  // full pin (100%) power of one pin [W]
  auto fpin_power = _power / sum;
  // actual pin power [W]
  for (unsigned int i = 0; i < n_rods; i++)
  {
    _ref_power[i] = _power_dis[i] * fpin_power;
  }

  // Convert the actual pin power to a linear heat rate [W/m]
  auto heated_length = _mesh.getHeatedLength();

  for (unsigned int i = 0; i < n_rods; i++)
  {
    _ref_qprime[i] = _ref_power[i] / heated_length;
  }
}

void
TriInterWrapperPowerIC::initialSetup()
{
  auto n_rods = _mesh.getNumOfRods();
  auto nz = _mesh.getNumOfAxialCells();
  auto z_grid = _mesh.getZGrid();
  auto heated_length = _mesh.getHeatedLength();
  auto unheated_length_entry = _mesh.getHeatedLengthEntry();

  _estimate_power.resize(n_rods);

  for (unsigned int iz = 1; iz < nz + 1; iz++)
  {
    // Compute the height of this element.
    auto dz = z_grid[iz] - z_grid[iz - 1];
    // Compute axial location of nodes.
    auto z2 = z_grid[iz];
    auto z1 = z_grid[iz - 1];
    Point p1(0, 0, z1 - unheated_length_entry);
    Point p2(0, 0, z2 - unheated_length_entry);
    // cycle through pins

    if (z2 > unheated_length_entry && z2 <= unheated_length_entry + heated_length)
    {
      for (unsigned int i_pin = 0; i_pin < n_rods; i_pin++)
      {
        // use of trapezoidal rule  to calculate local power
        _estimate_power[i_pin] +=
            _ref_qprime[i_pin] * (_axial_heat_rate.value(_t, p1) + _axial_heat_rate.value(_t, p2)) *
            dz / 2.0;
      }
    }
  }
  for (unsigned int i_pin = 0; i_pin < n_rods; i_pin++)
  {
    _pin_power_correction[i_pin] = _ref_power[i_pin] / _estimate_power[i_pin];
  }
}

Real
TriInterWrapperPowerIC::value(const Point & p)
{
  // Determine which subchannel this point is in.
  auto i = _mesh.getSubchannelIndexFromPoint(p);
  auto subch_type = _mesh.getSubchannelType(i);
  Real sum = 0.0;
  auto heated_length = _mesh.getHeatedLength();
  auto unheated_length_entry = _mesh.getHeatedLengthEntry();
  Point p1(0, 0, unheated_length_entry);
  // Point P = p - p1;

  // if we are adjacent to the heated part of the fuel rod
  if (p(2) >= unheated_length_entry && p(2) <= unheated_length_entry + heated_length)
  {
    if (subch_type == EChannelType::CENTER)
    {
      for (unsigned int j = 0; j < 3; j++)
      {
        auto rod_idx = _mesh.getRodIndex(i, j);
        sum = sum + 1.0 / 6.0 * _ref_qprime[rod_idx] * _pin_power_correction[rod_idx] *
                        _axial_heat_rate.value(_t, p);
      }
      return sum;
    }
    else if (subch_type == EChannelType::EDGE)
    {
      for (unsigned int j = 0; j < 2; j++)
      {
        auto rod_idx = _mesh.getRodIndex(i, j);
        sum = sum + 1.0 / 4.0 * _ref_qprime[rod_idx] * _pin_power_correction[rod_idx] *
                        _axial_heat_rate.value(_t, p);
      }
      return sum;
    }
    else if (subch_type == EChannelType::CORNER)
    {
      auto rod_idx = _mesh.getRodIndex(i, 0);
      sum = 1.0 / 6.0 * _ref_qprime[rod_idx] * _pin_power_correction[rod_idx] *
            _axial_heat_rate.value(_t, p);
      return sum;
    }
  }
  return 0.;
}
