#include "QuadPowerIC.h"
#include "Function.h"
#include "QuadSubChannelMesh.h"

using namespace std;
using namespace Eigen;

registerMooseObject("SubChannelApp", QuadPowerIC);

InputParameters
QuadPowerIC::validParams()
{
  InputParameters params = QuadSubChannelBaseIC::validParams();
  params.addRequiredParam<Real>("power", "[W]");
  params.addRequiredParam<std::string>(
      "filename",
      "name of power profile .txt file (should be a single column). It's "
      "a Radial Power Profile. [UnitLess]");
  params.addParam<FunctionName>(
      "axial_heat_rate",
      "1.0",
      "user provided normalized function of axial heat rate [Unitless]. "
      "The integral over pin heated length should equal the heated length");
  return params;
}

QuadPowerIC::QuadPowerIC(const InputParameters & params)
  : QuadSubChannelBaseIC(params),
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
  _pin_power_correction.resize(ny - 1, nx - 1);
  _pin_power_correction.setOnes();
  double vin;
  ifstream inFile;

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

  inFile.open(_filename);
  int i(0);
  while (inFile >> vin)
  {
    _power_dis(i, 0) = vin;
    i++;
  }
  inFile.close();

  _power_dis.resize(ny - 1, nx - 1);
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
QuadPowerIC::initialSetup()
{
  auto nx = _mesh.getNx();
  auto ny = _mesh.getNy();
  auto nz = _mesh.getNumOfAxialCells();
  auto z_grid = _mesh.getZGrid();
  auto heated_length = _mesh.getHeatedLength();
  auto unheated_length_entry = _mesh.getHeatedLengthEntry();

  _estimate_power.resize(ny - 1, nx - 1);
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
        // row
        unsigned int j = (i_pin / (nx - 1));
        // column
        unsigned int i = i_pin - j * (nx - 1);
        // use of trapezoidal rule to add to total power of pin
        _estimate_power(j, i) += _ref_qprime(j, i) *
                                 (_axial_heat_rate.value(_t, p1) + _axial_heat_rate.value(_t, p2)) *
                                 dz / 2.0;
      }
    }
  }

  // if a rod has zero power (_ref_qprime(j, i) = 0) then I need to avoid dividing by zero. I
  // divide by a wrong non-zero number which is not correct but this error doesn't mess things cause
  // _ref_qprime(j, i) = 0.0
  for (unsigned int i_pin = 0; i_pin < (ny - 1) * (nx - 1); i_pin++)
  {
    // row
    unsigned int j = (i_pin / (nx - 1));
    // column
    unsigned int i = i_pin - j * (nx - 1);
    if (_estimate_power(j, i) == 0.0)
      _estimate_power(j, i) = 1.0;
  }
  _pin_power_correction = _ref_power.cwiseQuotient(_estimate_power);
}

Real
QuadPowerIC::value(const Point & p)
{
  auto nx = _mesh.getNx();
  auto ny = _mesh.getNy();
  auto idx = _mesh.getSubchannelIndexFromPoint(p);
  unsigned int i = idx % nx; // column
  unsigned int j = idx / nx; // row
  auto heated_length = _mesh.getHeatedLength();
  auto unheated_length_entry = _mesh.getHeatedLengthEntry();
  Point p1(0, 0, unheated_length_entry);
  Point P = p - p1;
  // if we are adjacent to the heated part of the fuel rod
  if (p(2) >= unheated_length_entry && p(2) <= unheated_length_entry + heated_length)
  {
    // Compute and return the corrected axial heat rate deposited in channel at point p
    // Corners contact 1/4 of one pin
    if (i == 0 && j == 0)
      return 0.25 * _ref_qprime(j, i) * _pin_power_correction(j, i) * _axial_heat_rate.value(_t, P);
    else if (i == 0 && j == ny - 1)
      return 0.25 * _ref_qprime(j - 1, i) * _pin_power_correction(j - 1, i) *
             _axial_heat_rate.value(_t, P);
    else if (i == nx - 1 && j == 0)
      return 0.25 * _ref_qprime(j, i - 1) * _pin_power_correction(j, i - 1) *
             _axial_heat_rate.value(_t, P);
    else if (i == nx - 1 && j == ny - 1)
      return 0.25 * _ref_qprime(j - 1, i - 1) * _pin_power_correction(j - 1, i - 1) *
             _axial_heat_rate.value(_t, P);
    // Sides contact 1/4 of two pins
    else if (i == 0)
      return 0.25 *
             (_ref_qprime(j - 1, i) * _pin_power_correction(j - 1, i) +
              _ref_qprime(j, i) * _pin_power_correction(j, i)) *
             _axial_heat_rate.value(_t, P);
    else if (i == nx - 1)
      return 0.25 *
             (_ref_qprime(j - 1, i - 1) * _pin_power_correction(j - 1, i - 1) +
              _ref_qprime(j, i - 1) * _pin_power_correction(j, i - 1)) *
             _axial_heat_rate.value(_t, P);
    else if (j == 0)
      return 0.25 *
             (_ref_qprime(j, i - 1) * _pin_power_correction(j, i - 1) +
              _ref_qprime(j, i) * _pin_power_correction(j, i)) *
             _axial_heat_rate.value(_t, P);
    else if (j == ny - 1)
      return 0.25 *
             (_ref_qprime(j - 1, i - 1) * _pin_power_correction(j - 1, i - 1) +
              _ref_qprime(j - 1, i) * _pin_power_correction(j - 1, i)) *
             _axial_heat_rate.value(_t, P);
    // interior contacts 1/4 of 4 pins
    else
      return 0.25 *
             (_ref_qprime(j - 1, i - 1) * _pin_power_correction(j - 1, i - 1) +
              _ref_qprime(j, i - 1) * _pin_power_correction(j, i - 1) +
              _ref_qprime(j - 1, i) * _pin_power_correction(j - 1, i) +
              _ref_qprime(j, i) * _pin_power_correction(j, i)) *
             _axial_heat_rate.value(_t, P);
  }
  else
    return 0.0;
}
