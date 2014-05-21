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

  //std::cerr << "(x,y,z) = (" << x << "," << y << "," << z << ")\n";

  // check for errors in debug builds
  mooseAssert( x >= 0 && _nx - x > 0, "Point out of bounds in StochasticField" );
  mooseAssert( y >= 0 && _ny - y > 0, "Point out of bounds in StochasticField" );
  mooseAssert( z >= 0 && _nz - z > 0, "Point out of bounds in StochasticField" );

  return _data[(z*_ny + y)*_nx + x];
}

/*void
test_sf(std::string fname)
{
  StochasticField sf(fname);
  Real epsilon = 0.00000001;

  std::cout << "The first measured value:\n";

  Point p(0,0,0);
  Point p1(0.23E-03, 0.23E-03, 0.5);
  Point p2(0.4687E-03 - epsilon, 0.4687E-03 - epsilon, 0.1000E+01 - epsilon);
  std::cout << sf.value(p) << ", " << sf.value(p1) << ", " << sf.value(p2) << "\n\n";

  std::cout << "The second measured value:\n";
  Point p3(0.4687E-03, 0, 0);
  Point p4(0.4787E-03, 0, 0);
  Point p5(0.6687E-03, 0.23E-03, 0.5);
  Point p6(2.0*0.4687E-03 - epsilon, 0.4687E-03 - epsilon, 1 - epsilon);
  std::cout << sf.value(p3) << ", " << sf.value(p4) << ", " << sf.value(p5) << ", " << sf.value(p6) << "\n";

  std::cout << "The last measured value:\n";
  Point p7(0.4687E-03*639, 0.4687E-03*319, 0);
  Point p8(0.4687E-03*640 - epsilon, 0.4687E-03*320 - epsilon, 1 - epsilon);
  std::cout << sf.value(p7) << ", " << sf.value(p8) << "\n\n";
}
*/
