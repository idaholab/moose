/********************************************************************/
/*                   DO NOT MODIFY THIS HEADER                      */
/*          Subchannel: Thermal Hydraulics Reactor Analysis         */
/*                                                                  */
/*              (c) 2022 Battelle Energy Alliance, LLC              */
/*                      ALL RIGHTS RESERVED                         */
/*                                                                  */
/*             Prepared by Battelle Energy Alliance, LLC            */
/*               Under Contract No. DE-AC07-05ID14517               */
/*               With the U. S. Department of Energy                */
/*                                                                  */
/*               See COPYRIGHT for full restrictions                */
/********************************************************************/

#include "QuadInterWrapperPowerIC.h"
#include "Function.h"
#include "QuadInterWrapperMesh.h"

using namespace std;
using namespace Eigen;

registerMooseObject("SubChannelApp", QuadInterWrapperPowerIC);

InputParameters
QuadInterWrapperPowerIC::validParams()
{
  InputParameters params = QuadInterWrapperBaseIC::validParams();
  params.addParam<Real>("power", 0.0, "[W]");
  params.addParam<std::string>("filename",
                               "file_was_not_found",
                               "name of power profile .txt file (should be a single column). It's "
                               "a Radial Power Profile. [UnitLess]");
  params.addParam<FunctionName>(
      "axial_heat_rate",
      "1.0",
      "user provided normalized function of axial heat rate [Unitless]. "
      "The integral over pin heated length should equal the heated length");
  return params;
}

QuadInterWrapperPowerIC::QuadInterWrapperPowerIC(const InputParameters & params)
  : QuadInterWrapperBaseIC(params),
    _power(getParam<Real>("power")),
    _numberoflines(0),
    _filename(getParam<std::string>("filename")),
    _axial_heat_rate(getFunction("axial_heat_rate"))
{
  auto nx = _mesh.getNx();
  auto ny = _mesh.getNy();
  auto heated_length = _mesh.getHeatedLength();

  _power_dis.resize((ny - 1) * (nx - 1), 1);
  _power_dis.setZero();
  _assembly_power_correction.resize((ny - 1) * (nx - 1), 1);
  _assembly_power_correction.setOnes();
  double vin;
  ifstream inFile;

  _console << "Power file: " << _filename << std::endl;

  if (_filename.compare("file_was_not_found"))
  {
    inFile.open(_filename);
    if (!inFile)
      mooseError(name(), "unable to open file : ", _filename);

    while (inFile >> vin)
      _numberoflines += 1;

    if (inFile.fail() && !inFile.eof())
      mooseError(name(), " non numerical input at line : ", _numberoflines);

    if (_numberoflines != (ny - 1) * (nx - 1))
      mooseError(name(), " Radial profile file doesn't have correct size : ", (ny - 1) * (nx - 1));
    inFile.close();
  }
  else
  {
    _numberoflines = (ny - 1) * (nx - 1);
  }

  if (_filename.compare("file_was_not_found"))
  {
    inFile.open(_filename);
    int i(0);
    while (inFile >> vin)
    {
      _power_dis(i, 0) = vin;
      i++;
    }
    inFile.close();
  }

  _console << " Power distribution matrix :\n" << _power_dis << " \n";
  auto sum = _power_dis.sum();
  // full (100%) power of one pin [W]
  auto fpin_power = _power / sum;
  // actual pin power [W]
  _ref_power = _power_dis * fpin_power;
  // Convert the actual pin power to a linear heat rate [W/m]
  _ref_qprime = _ref_power / heated_length;
}

void
QuadInterWrapperPowerIC::initialSetup()
{

  auto nx = _mesh.getNx();
  auto ny = _mesh.getNy();
  auto nz = _mesh.getNumOfAxialCells();
  auto z_grid = _mesh.getZGrid();
  auto heated_length = _mesh.getHeatedLength();
  auto unheated_length_entry = _mesh.getHeatedLengthEntry();

  _estimate_power.resize((ny - 1) * (nx - 1), 1);
  _estimate_power.setZero();
  for (unsigned int iz = 1; iz < nz + 1; iz++)
  {
    // Compute the height of this element.
    auto dz = z_grid[iz] - z_grid[iz - 1];
    // Compute axial location of nodes.
    auto z2 = z_grid[iz];
    auto z1 = z_grid[iz - 1];
    Point p1(0, 0, z1 - unheated_length_entry);
    Point p2(0, 0, z2 - unheated_length_entry);
    // cycle through pins to estimate the total power of each pin
    if (z2 > unheated_length_entry && z2 <= unheated_length_entry + heated_length)
    {
      for (unsigned int i_pin = 0; i_pin < (ny - 1) * (nx - 1); i_pin++)
      {
        // use of trapezoidal rule to add to total power of pin
        _estimate_power(i_pin) +=
            _ref_qprime(i_pin) * (_axial_heat_rate.value(_t, p1) + _axial_heat_rate.value(_t, p2)) *
            dz / 2.0;
      }
    }
  }

  // if a rod has zero power (_ref_qprime(j, i) = 0) then I need to avoid dividing by zero. I
  // divide by a wrong non-zero number which is not correct but this error doesn't mess things cause
  // _ref_qprime(j, i) = 0.0
  for (unsigned int i_pin = 0; i_pin < (ny - 1) * (nx - 1); i_pin++)
  {
    if (_estimate_power(i_pin) == 0.0)
      _estimate_power(i_pin) = 1.0;
  }
  _assembly_power_correction = _ref_power.cwiseQuotient(_estimate_power);
}

Real
QuadInterWrapperPowerIC::value(const Point & p)
{
  auto heated_length = _mesh.getHeatedLength();
  auto unheated_length_entry = _mesh.getHeatedLengthEntry();
  Point p1(0, 0, unheated_length_entry);
  Point P = p - p1;
  auto pin_mesh_exist = _mesh.pinMeshExist();

  if (pin_mesh_exist)
  {
    // project axial heat rate on pins
    auto i_pin = _mesh.getPinIndexFromPoint(p);
    {
      if (p(2) >= unheated_length_entry && p(2) <= unheated_length_entry + heated_length)
        return _ref_qprime(i_pin) * _assembly_power_correction(i_pin) *
               _axial_heat_rate.value(_t, P);
      else
        return 0.0;
    }
  }
  else
  {
    // project axial heat rate on subchannels
    auto i_ch = _mesh.getSubchannelIndexFromPoint(p);
    // if we are adjacent to the heated part of the fuel rod
    if (p(2) >= unheated_length_entry && p(2) <= unheated_length_entry + heated_length)
    {
      auto heat_rate = 0.0;
      for (auto i_pin : _mesh.getChannelPins(i_ch))
      {
        heat_rate += 0.25 * _ref_qprime(i_pin) * _assembly_power_correction(i_pin) *
                     _axial_heat_rate.value(_t, P);
      }
      return heat_rate;
    }
    else
      return 0.0;
  }
}
