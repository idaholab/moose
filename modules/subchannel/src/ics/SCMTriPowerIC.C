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

#include "SCMTriPowerIC.h"
#include "Function.h"
#include "TriSubChannelMesh.h"

registerMooseObject("SubChannelApp", SCMTriPowerIC);

InputParameters
SCMTriPowerIC::validParams()
{
  InputParameters params = TriSubChannelBaseIC::validParams();
  params.addClassDescription(
      "Computes axial power rate (W/m) that goes into the subchannel cells "
      "or is assigned to the fuel pins, in a triangular lattice arrangement");
  params.addRequiredParam<Real>("power", "The total power of the subassembly [W]");
  params.addRequiredParam<std::string>(
      "filename", "name of radial power profile .txt file (should be a single column) [UnitLess].");
  params.addParam<FunctionName>("axial_heat_rate",
                                "1.0",
                                "user provided normalized function of axial heat rate [Unitless]. "
                                "The integral over pin length should equal the heated length");
  return params;
}

SCMTriPowerIC::SCMTriPowerIC(const InputParameters & params)
  : TriSubChannelBaseIC(params),
    _power(getParam<Real>("power")),
    _numberoflines(0),
    _filename(getParam<std::string>("filename")),
    _axial_heat_rate(getFunction("axial_heat_rate"))
{
  auto n_rods = _mesh.getNumOfRods();
  auto heated_length = _mesh.getHeatedLength();

  _power_dis.resize(n_rods, 1);
  _power_dis.setZero();
  _pin_power_correction.resize(n_rods, 1);
  _pin_power_correction.setOnes();

  Real vin;
  std::ifstream inFile;

  inFile.open(_filename);
  if (!inFile)
    mooseError(name(), ": Unable to open file: ", _filename);

  while (inFile >> vin)
    _numberoflines += 1;

  if (inFile.fail() && !inFile.eof())
    mooseError(name(), ": Non numerical input at line: ", _numberoflines);

  if (_numberoflines != n_rods)
    mooseError(name(), ": Radial profile file doesn't have correct size: ", n_rods);
  inFile.close();

  inFile.open(_filename);
  int i = 0;
  while (inFile >> vin)
  {
    _power_dis(i, 0) = vin;
    i++;
  }
  inFile.close();
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
SCMTriPowerIC::initialSetup()
{
  auto n_rods = _mesh.getNumOfRods();
  auto nz = _mesh.getNumOfAxialCells();
  auto z_grid = _mesh.getZGrid();
  auto heated_length = _mesh.getHeatedLength();
  auto unheated_length_entry = _mesh.getHeatedLengthEntry();

  _estimate_power.resize(n_rods, 1);
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
    // cycle through pins

    if (z2 > unheated_length_entry && z2 <= unheated_length_entry + heated_length)
    {
      for (unsigned int i_pin = 0; i_pin < n_rods; i_pin++)
      {
        // use of trapezoidal rule  to calculate local power
        _estimate_power(i_pin) +=
            _ref_qprime(i_pin) * (_axial_heat_rate.value(_t, p1) + _axial_heat_rate.value(_t, p2)) *
            dz / 2.0;
      }
    }
  }
  // if a rod has zero power (_ref_qprime(i_pin) = 0) then I need to avoid dividing by zero. I
  // divide by a wrong non-zero number which is not correct but this error doesn't mess things cause
  // _ref_qprime(j, i) = 0.0
  for (unsigned int i_pin = 0; i_pin < n_rods; i_pin++)
  {
    if (_estimate_power(i_pin) == 0.0)
      _estimate_power(i_pin) = 1.0;
  }
  _pin_power_correction = _ref_power.cwiseQuotient(_estimate_power);
}

Real
SCMTriPowerIC::value(const Point & p)
{
  auto heat_rate = 0.0;
  auto heated_length = _mesh.getHeatedLength();
  auto unheated_length_entry = _mesh.getHeatedLengthEntry();
  Point p1(0, 0, unheated_length_entry);
  Point P = p - p1;
  auto pin_mesh_exist = _mesh.pinMeshExist();

  if (pin_mesh_exist)
  {
    // project axial heat rate on pins
    auto i_pin = _mesh.getPinIndexFromPoint(p);
    if (p(2) >= unheated_length_entry && p(2) <= unheated_length_entry + heated_length)
      return _ref_qprime(i_pin) * _pin_power_correction(i_pin) * _axial_heat_rate.value(_t, P);
    else
      return 0.0;
  }
  else
  {
    // Determine which subchannel this point is in.
    auto i_ch = _mesh.getSubchannelIndexFromPoint(p);
    auto subch_type = _mesh.getSubchannelType(i_ch);
    // project axial heat rate on subchannels
    if (p(2) >= unheated_length_entry && p(2) <= unheated_length_entry + heated_length)
    {
      double factor;
      switch (subch_type)
      {
        case EChannelType::CENTER:
          factor = 1.0 / 6.0;
          break;
        case EChannelType::EDGE:
          factor = 1.0 / 4.0;
          break;
        case EChannelType::CORNER:
          factor = 1.0 / 6.0;
          break;
        default:
          return 0.0; // handle invalid subch_type if needed
      }
      for (auto i_pin : _mesh.getChannelPins(i_ch))
      {
        heat_rate += factor * _ref_qprime(i_pin) * _pin_power_correction(i_pin) *
                     _axial_heat_rate.value(_t, P);
      }
      return heat_rate;
    }
  }
  return 0.0;
}
