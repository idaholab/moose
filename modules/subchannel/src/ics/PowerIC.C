#include "PowerIC.h"
#include "Function.h"

using namespace std;
using namespace Eigen;

registerMooseObject("SubChannelApp", PowerIC);

InputParameters
PowerIC::validParams()
{
  InputParameters params = SubChannelBaseIC::validParams();
  params.addRequiredParam<Real>("power", "[W]");
  params.addParam<std::string>("filename",
                               413.0,
                               "name of power profile .txt file (should be a single column). It's "
                               "a Radial Power Profile. [UnitLess]");
  params.addParam<FunctionName>("axial_heat_rate",
                                1.0,
                                "user provided normalized function of axial heat rate [Unitless]. "
                                "The integral over pin length should equal the heated length");
  return params;
}

PowerIC::PowerIC(const InputParameters & params)
  : SubChannelBaseIC(params),
    _mesh(dynamic_cast<SubChannelMesh &>(_fe_problem.mesh())),
    _axial_heat_rate(getFunction("axial_heat_rate"))
{
  _power = getParam<Real>("power");
  _filename = getParam<std::string>("filename");
  _power_dis.resize((_mesh._ny - 1) * (_mesh._nx - 1), 1);
  _power_dis.setZero();
  _pin_power_correction.resize(_mesh._ny - 1, _mesh._nx - 1);
  _pin_power_correction.setOnes();
  _numberoflines = 0;
  double vin;
  ifstream inFile;

  inFile.open(_filename);
  if (!inFile)
    mooseError("unable to open file : ", _filename);

  while (inFile >> vin)
    _numberoflines += 1;

  if (inFile.fail() && !inFile.eof())
    mooseError("non numerical input at line : ", _numberoflines);

  inFile.close();

  inFile.open(_filename);
  int i(0);
  while (inFile >> vin)
  {
    _power_dis(i) = vin;
    i++;
  }
  inFile.close();

  _power_dis.resize(_mesh._ny - 1, _mesh._nx - 1);
  auto sum = _power_dis.sum();
  // full pin (100%) power of one pin [W]
  auto fpin_power = _power / sum;
  // actual pin power [W]
  _ref_power = _power_dis * fpin_power;
  // Convert the actual pin power to a linear heat rate [W/m]
  auto heated_length = _mesh._heated_length;
  _ref_qprime = _ref_power / heated_length;
}

void
PowerIC::initialSetup()
{
  _estimate_power.resize(_mesh._ny - 1, _mesh._nx - 1);
  _estimate_power.setZero();
  for (unsigned int iz = 1; iz < _mesh._nz + 1; iz++)
  {
    // Compute the height of this element.
    auto dz = _mesh._z_grid[iz] - _mesh._z_grid[iz - 1];
    // Compute axial location of nodes.
    auto z2 = _mesh._z_grid[iz];
    auto z1 = _mesh._z_grid[iz - 1];
    Point p1(0, 0, z1);
    Point p2(0, 0, z2);
    // cycle through pins
    for (unsigned int i_pin = 0; i_pin < (_mesh._ny - 1) * (_mesh._nx - 1); i_pin++)
    {
      // row
      unsigned int j = (i_pin / (_mesh._nx - 1));
      // column
      unsigned int i = i_pin - j * (_mesh._nx - 1);
      // use of trapezoidal rule  to calculate local power
      _estimate_power(j, i) += _ref_qprime(j, i) *
                               (_axial_heat_rate.value(_t, p1) + _axial_heat_rate.value(_t, p2)) *
                               dz / 2.0;
    }
  }
  _pin_power_correction = _ref_power.cwiseQuotient(_estimate_power);
};

Real
PowerIC::value(const Point & p)
{
  // Determine which subchannel this point is in.
  auto inds = index_point(p);
  // x index
  auto i = inds.first;
  // y index
  auto j = inds.second;
  // Compute and return the estimated channel axial heat rate per channel
  // Corners contact 1/4 of a  one pin
  if (i == 0 && j == 0)
    return 0.25 * _ref_qprime(j, i) * _pin_power_correction(j, i) * _axial_heat_rate.value(_t, p);
  else if (i == 0 && j == _mesh._ny - 1)
    return 0.25 * _ref_qprime(j - 1, i) * _pin_power_correction(j - 1, i) *
           _axial_heat_rate.value(_t, p);
  else if (i == _mesh._nx - 1 && j == 0)
    return 0.25 * _ref_qprime(j, i - 1) * _pin_power_correction(j, i - 1) *
           _axial_heat_rate.value(_t, p);
  else if (i == _mesh._nx - 1 && j == _mesh._ny - 1)
    return 0.25 * _ref_qprime(j - 1, i - 1) * _pin_power_correction(j - 1, i - 1) *
           _axial_heat_rate.value(_t, p);
  // Sides contact 1/4 of  two pins
  else if (i == 0)
    return 0.25 *
           (_ref_qprime(j - 1, i) * _pin_power_correction(j - 1, i) +
            _ref_qprime(j, i) * _pin_power_correction(j, i)) *
           _axial_heat_rate.value(_t, p);
  else if (i == _mesh._nx - 1)
    return 0.25 *
           (_ref_qprime(j - 1, i - 1) * _pin_power_correction(j - 1, i - 1) +
            _ref_qprime(j, i - 1) * _pin_power_correction(j, i - 1)) *
           _axial_heat_rate.value(_t, p);
  else if (j == 0)
    return 0.25 *
           (_ref_qprime(j, i - 1) * _pin_power_correction(j, i - 1) +
            _ref_qprime(j, i) * _pin_power_correction(j, i)) *
           _axial_heat_rate.value(_t, p);
  else if (j == _mesh._ny - 1)
    return 0.25 *
           (_ref_qprime(j - 1, i - 1) * _pin_power_correction(j - 1, i - 1) +
            _ref_qprime(j - 1, i) * _pin_power_correction(j - 1, i)) *
           _axial_heat_rate.value(_t, p);
  // interior contacts 1/4 of 4 pins
  else
    return 0.25 *
           (_ref_qprime(j - 1, i - 1) * _pin_power_correction(j - 1, i - 1) +
            _ref_qprime(j, i - 1) * _pin_power_correction(j, i - 1) +
            _ref_qprime(j - 1, i) * _pin_power_correction(j - 1, i) +
            _ref_qprime(j, i) * _pin_power_correction(j, i)) *
           _axial_heat_rate.value(_t, p);
}
