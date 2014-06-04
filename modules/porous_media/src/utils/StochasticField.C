// local includes
#include "StochasticField.h"
#include "MooseError.h" //for mooseError/Assert

#include <fstream>

StochasticField::StochasticField(std::string fname)
{
  std::ifstream file(fname.c_str());
  if (!file)
    mooseError("Error opening file");

  // get the first line of the file (a comment)
  std::string tmp;
  std::getline(file, tmp);

  // get nx, ny, and nz
  std::getline(file, tmp, '=');
  file >> _nx;
  std::getline(file, tmp, '=');
  file >> _ny;
  std::getline(file, tmp, '=');
  file >> _nz;

  // get dx, dy, and dz
  std::getline(file, tmp, '=');
  file >> _dx;
  std::getline(file, tmp, '=');
  file >> _dy;
  std::getline(file, tmp, '=');
  file >> _dz;

  // reset to the next line because there may be units left on this line
  std::getline(file, tmp);

  int num_pts = _nx*_ny*_nz;
  _data.resize(num_pts);

  // We'll index this array like [z][y][x] = [(z*_ny + y)*_nx + x], so
  // read straight from the file and plop the values straight in the array.
  Real val;
  for (int i = 0; i < num_pts; i++)
  {
    file >> val;
    _data[i] = val;
  }
}

Real
StochasticField::value(Point p)
{
  // calculate which bin the real-valued point falls into
  int x = int(p(0)/_dx);
  int y = int(p(1)/_dy);
  int z = int(p(2)/_dz);

  // check for errors in debug builds
  mooseAssert( x >= 0 && _nx - x > 0, "Point out of bounds in StochasticField" );
  mooseAssert( y >= 0 && _ny - y > 0, "Point out of bounds in StochasticField" );
  mooseAssert( z >= 0 && _nz - z > 0, "Point out of bounds in StochasticField" );

  return _data[(z*_ny + y)*_nx + x];
}
