#include "PsbtPowerIC.h"

using namespace std;
using namespace Eigen;

registerMooseObject("SubChannelApp", PsbtPowerIC);

InputParameters
PsbtPowerIC::validParams()
{
  InputParameters params = PsbtIC::validParams();
  params.addParam<Real>("power", 413.0, " [MW]");
  params.addParam<std::string>("filename", 413.0, "Location of power profile txt file");
  return params;
}

PsbtPowerIC::PsbtPowerIC(const InputParameters & params) : PsbtIC(params)
{
  _mesh = dynamic_cast<SubChannelMesh *>(&_fe_problem.mesh());

  power = getParam<Real>("power");
  filename = getParam<std::string>("filename");

  power_dis.resize((_mesh->ny_ - 1) * (_mesh->nx_ - 1), 1);
  power_dis.setZero();

  numberoflines = 0;
  double vin;
  ifstream inFile;
  inFile.open(filename);
  if (!inFile)
  {
    cerr << "Unable to open file : " << filename << endl;
    exit(1); // call system to stop
  }
  while (inFile >> vin)
  {
    numberoflines = numberoflines + 1;
  }
  if (inFile.fail() && !inFile.eof())
  {
    cerr << "ecountered non numerical input in input file : "
         << " at line " << numberoflines << endl;
    exit(1);
  }
  inFile.close();

  cout << "Number of Lines in power profile input file is: " << numberoflines << endl;

  inFile.open(filename);
  int i(0);
  while (inFile >> vin)
  {
    power_dis(i) = vin;
    i++;
  }
  inFile.close();

  power_dis.resize(_mesh->ny_ - 1, _mesh->nx_ - 1);
  auto sum = power_dis.sum();

  std::cout << "filename:" << filename << std::endl;
  std::cout << "power_dis: "
            << "\n"
            << power_dis << std::endl;

  auto fpin_power = power / sum;           // MW
  auto ref_power = power_dis * fpin_power; // MW

  // Convert the reference power to a linear heat rate.
  Real heated_length = _mesh->heated_length_;     // in m
  _ref_qprime = 1e+3 * ref_power / heated_length; // in KW/m

  std::cout << "_ref_qprime "
            << "\n"
            << _ref_qprime << std::endl;
}

Real
PsbtPowerIC::value(const Point & p)
{
  _mesh = dynamic_cast<SubChannelMesh *>(&_fe_problem.mesh());
  // Determine which channel this point is in.
  auto inds = index_point(p);
  auto i = inds.first;  // x index
  auto j = inds.second; // y index

  // Compute and return the channel axial heat rate per channel.
  // Corners contact 1/4 of a  one pin
  if (i == 0 && j == 0)
  {
    return 0.25 * _ref_qprime(j, i);
  }
  else if (i == 0 && j == _mesh->ny_ - 1)
  {
    return 0.25 * _ref_qprime(j - 1, i);
  }
  else if (i == _mesh->nx_ - 1 && j == 0)
  {
    return 0.25 * _ref_qprime(j, i - 1);
  }
  else if (i == _mesh->nx_ - 1 && j == _mesh->ny_ - 1)
  {
    return 0.25 * _ref_qprime(j - 1, i - 1);
  }
  // Sides contact 1/4 of  two pins
  else if (i == 0)
  {
    return 0.25 * (_ref_qprime(j - 1, i) + _ref_qprime(j, i));
  }
  else if (i == _mesh->nx_ - 1)
  {
    return 0.25 * (_ref_qprime(j - 1, i - 1) + _ref_qprime(j, i - 1));
  }
  else if (j == 0)
  {
    return 0.25 * (_ref_qprime(j, i - 1) + _ref_qprime(j, i));
  }
  else if (j == _mesh->ny_ - 1)
  {
    return 0.25 * (_ref_qprime(j - 1, i - 1) + _ref_qprime(j - 1, i));
  }
  // interior contacts 1/4 of 4 pins
  else
  {
    return 0.25 * (_ref_qprime(j - 1, i - 1) + _ref_qprime(j, i - 1) + _ref_qprime(j - 1, i) +
                   _ref_qprime(j, i));
  }
}
