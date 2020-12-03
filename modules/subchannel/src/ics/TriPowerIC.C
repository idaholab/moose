#include "TriPowerIC.h"
#include "Function.h"
#include "TriSubChannelMesh.h"

registerMooseObject("SubChannelApp", TriPowerIC);

InputParameters
TriPowerIC::validParams()
{
  InputParameters params = TriSubChannelBaseIC::validParams();
  params.addRequiredParam<Real>("power", "[W]");
  params.addRequiredParam<std::string>(
      "filename",
      "Name of file with power profile (values are stored in a single "
      "column). It's a radial power profile. [Unitless]");
  params.addParam<FunctionName>("axial_heat_rate",
                                "1.0",
                                "user provided normalized function of axial heat rate [Unitless]. "
                                "The integral over pin length should equal the heated length");
  return params;
}

TriPowerIC::TriPowerIC(const InputParameters & params)
  : TriSubChannelBaseIC(params),
    _mesh(dynamic_cast<TriSubChannelMesh &>(_fe_problem.mesh())),
    _power(getParam<Real>("power")),
    _numberoflines(0),
    _filename(getParam<std::string>("filename")),
    _axial_heat_rate(getFunction("axial_heat_rate"))
{
  _power_dis.resize(_mesh._nrods);
  _pin_power_correction.resize(_mesh._nrods);
  _ref_power.resize(_mesh._nrods);
  _ref_qprime.resize(_mesh._nrods);
  _estimate_power.resize(_mesh._nrods);
  for (unsigned int i = 0; i < _mesh._nrods; i++)
  {
    _power_dis[i] = 0.0;
    _pin_power_correction[i] = 1.0;
  }

  Real vin;
  std::ifstream inFile;

  inFile.open(_filename);
  if (!inFile)
    mooseError(name(), ": Unable to open file: ", _filename);

  while (inFile >> vin)
    _numberoflines += 1;

  if (inFile.fail() && !inFile.eof())
    mooseError(name(), ": Non numerical input at line: ", _numberoflines);

  if (_numberoflines != _mesh._nrods)
    mooseError(name(), ": Radial profile file doesn't have correct size: ", _mesh._nrods);
  inFile.close();

  inFile.open(_filename);
  int i = 0;
  while (inFile >> vin)
  {
    _power_dis[i] = vin;
    i++;
  }
  inFile.close();
  Real sum = 0.0;

  for (unsigned int i = 0; i < _mesh._nrods; i++)
  {
    sum = sum + _power_dis[i];
  }
  // full pin (100%) power of one pin [W]
  auto fpin_power = _power / sum;
  // actual pin power [W]
  for (unsigned int i = 0; i < _mesh._nrods; i++)
  {
    _ref_power[i] = _power_dis[i] * fpin_power;
  }

  // Convert the actual pin power to a linear heat rate [W/m]
  auto heated_length = _mesh._heated_length;

  for (unsigned int i = 0; i < _mesh._nrods; i++)
  {
    _ref_qprime[i] = _ref_power[i] / heated_length;
  }
}

void
TriPowerIC::initialSetup()
{
  _estimate_power.resize(_mesh._nrods);

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
    for (unsigned int i_pin = 0; i_pin < _mesh._nrods; i_pin++)
    {
      // use of trapezoidal rule  to calculate local power
      _estimate_power[i_pin] += _ref_qprime[i_pin] *
                                (_axial_heat_rate.value(_t, p1) + _axial_heat_rate.value(_t, p2)) *
                                dz / 2.0;
    }
  }
  for (unsigned int i_pin = 0; i_pin < _mesh._nrods; i_pin++)
  {
    _pin_power_correction[i_pin] = _ref_power[i_pin] / _estimate_power[i_pin];
  }
};

Real
TriPowerIC::value(const Point & p)
{
  // Determine which subchannel this point is in.
  auto i = index_point(p);
  Real sum = 0.0;
  if (_mesh._subch_type[i] == TriSubChannelMesh::CENTER)
  {
    for (unsigned int j = 0; j < 3; j++)
    {
      sum = sum + 1.0 / 6.0 * _ref_qprime[_mesh._subchannel_to_rod_map[i][j]] *
                      _pin_power_correction[_mesh._subchannel_to_rod_map[i][j]] *
                      _axial_heat_rate.value(_t, p);
    }
    return sum;
  }
  else if (_mesh._subch_type[i] == TriSubChannelMesh::EDGE)
  {
    for (unsigned int j = 0; j < 2; j++)
    {
      sum = sum + 1.0 / 4.0 * _ref_qprime[_mesh._subchannel_to_rod_map[i][j]] *
                      _pin_power_correction[_mesh._subchannel_to_rod_map[i][j]] *
                      _axial_heat_rate.value(_t, p);
    }
    return sum;
  }
  else if (_mesh._subch_type[i] == TriSubChannelMesh::CORNER)
  {
    sum = 1.0 / 6.0 * _ref_qprime[_mesh._subchannel_to_rod_map[i][0]] *
          _pin_power_correction[_mesh._subchannel_to_rod_map[i][0]] * _axial_heat_rate.value(_t, p);
    return sum;
  }

  return 0.;
}
