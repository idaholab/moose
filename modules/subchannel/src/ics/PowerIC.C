#include "PowerIC.h"

using namespace std;
using namespace Eigen;

registerMooseObject("SubChannelApp", PowerIC);

InputParameters
PowerIC::validParams()
{
  InputParameters params = IC::validParams();
  params.addRequiredParam<Real>("power", "[W]");
  params.addParam<std::string>("filename", 413.0, "name of power profile .txt file (should be a single column)");
  params.addParam<FunctionName>("axial_heat_rate", 1.0, "user provided normalized function of axial heat rate [unitless]. The integral over pin length should equal the heated length" );
  return params;
}

PowerIC::PowerIC(const InputParameters & params) : IC(params), _axial_heat_rate(getFunction("axial_heat_rate"))
{
  _mesh = dynamic_cast<SubChannelMesh *> (& _fe_problem.mesh());
  _power = getParam<Real>("power");
  _filename = getParam<std::string>("filename");
  _power_dis.resize((_mesh->_ny - 1) * (_mesh->_nx - 1), 1);
  _power_dis.setZero();
  _numberoflines = 0;
  double vin;
  ifstream inFile;

  inFile.open(_filename);
  if (!inFile)
  {
    cerr << "Unable to open file : " << _filename << endl;
    exit(1); // call system to stop
  }

  while (inFile >> vin)
    _numberoflines += 1;

  if (inFile.fail() && !inFile.eof())
  {
    cerr << "ecountered non numerical input in input file : "
         << " at line " << _numberoflines << endl;
    exit(1);
  }
  inFile.close();

  inFile.open(_filename);
  int i(0);
  while (inFile >> vin)
  {
    _power_dis(i) = vin;
    i++;
  }
  inFile.close();

  _power_dis.resize(_mesh->_ny - 1, _mesh->_nx - 1);
  auto sum = _power_dis.sum();
  auto fpin_power = _power / sum;           // full pin power W
  auto ref_power = _power_dis * fpin_power; // W
  // Convert the reference power to a linear heat rate.
  auto heated_length = _mesh->_heated_length; // in m
  _ref_qprime = ref_power / heated_length; // in W/m
}

Real PowerIC::value(const Point & p)
{
  _mesh = dynamic_cast<SubChannelMesh *>(&_fe_problem.mesh());
  auto inds = index_point(p); // Determine which channel this point is in.
  auto i = inds.first;  // x index
  auto j = inds.second; // y index

  // Compute and return the channel axial heat rate per channel.
  // Corners contact 1/4 of a  one pin
  if (i == 0 && j == 0)
    return 0.25 * _ref_qprime(j, i);
  else if (i == 0 && j == _mesh->_ny - 1)
    return 0.25 * _ref_qprime(j - 1, i);
  else if (i == _mesh->_nx - 1 && j == 0)
    return 0.25 * _ref_qprime(j, i - 1);
  else if (i == _mesh->_nx - 1 && j == _mesh->_ny - 1)
    return 0.25 * _ref_qprime(j - 1, i - 1);
  // Sides contact 1/4 of  two pins
  else if (i == 0)
    return 0.25 * (_ref_qprime(j - 1, i) + _ref_qprime(j, i));
  else if (i == _mesh->_nx - 1)
    return 0.25 * (_ref_qprime(j - 1, i - 1) + _ref_qprime(j, i - 1));
  else if (j == 0)
    return 0.25 * (_ref_qprime(j, i - 1) + _ref_qprime(j, i));
  else if (j == _mesh->_ny - 1)
    return 0.25 * (_ref_qprime(j - 1, i - 1) + _ref_qprime(j - 1, i));
  // interior contacts 1/4 of 4 pins
  else
    return 0.25 * (_ref_qprime(j - 1, i - 1) + _ref_qprime(j, i - 1) + _ref_qprime(j - 1, i) + _ref_qprime(j, i));
}
