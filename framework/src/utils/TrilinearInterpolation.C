//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TrilinearInterpolation.h"
#include "MooseError.h"

TrilinearInterpolation::TrilinearInterpolation(const std::vector<Real> & x,
                                               const std::vector<Real> & y,
                                               const std::vector<Real> & z,
                                               const std::vector<Real> & data)
  : _x_axis(x), _y_axis(y), _z_axis(z), _fxyz(data)
{
  if (_x_axis.size() < 1)
    mooseError("x vector has zero elements. At least one element is required.");
  if (_y_axis.size() < 1)
    mooseError("y vector has zero elements. At least one element is required.");
  if (_z_axis.size() < 1)
    mooseError("z vector has zero elements. At least one element is required.");
  if (_x_axis.size() * _y_axis.size() * _z_axis.size() != data.size())
    mooseError("The size of data (",
               data.size(),
               ") does not match the supplied dimensions (",
               _x_axis.size(),
               ", ",
               _y_axis.size(),
               ", ",
               _z_axis.size(),
               ")");
}

void
TrilinearInterpolation::getCornerIndices(
    const std::vector<Real> & v, Real x, int & lower, int & upper, Real & d) const
{
  unsigned int N = v.size();
  if (x < v[0])
  {
    lower = 0;
    upper = 0;
  }
  else if (x >= v[N - 1])
  {
    lower = N - 1;
    upper = N - 1;
  }
  else
  {
    for (unsigned int i = 0; i < N - 1; i++)
    {
      if (x > v[i] && x < v[i + 1])
      {
        lower = i;
        upper = i + 1;
        d = (x - v[lower]) / (v[upper] - v[lower]);
        break;
      }
      else if (x == v[i])
      {
        lower = i;
        upper = i;
        break;
      }
    }
  }
}

Real
TrilinearInterpolation::getCornerValues(int x, int y, int z) const
{
  int nY = _y_axis.size();
  int nZ = _z_axis.size();

  return _fxyz[x * nY * nZ + y * nZ + z];
}

Real
TrilinearInterpolation::sample(Real x, Real y, Real z) const
{
  int x0 = 0;
  int y0 = 0;
  int z0 = 0;
  int x1 = 0;
  int y1 = 0;
  int z1 = 0;
  Real Dx = 0;
  Real Dy = 0;
  Real Dz = 0;

  // find the the indices of the cube, which contains the point
  getCornerIndices(_x_axis, x, x0, x1, Dx);
  getCornerIndices(_y_axis, y, y0, y1, Dy);
  getCornerIndices(_z_axis, z, z0, z1, Dz);

  // find the corresponding function values for the corner indices
  Real f000 = getCornerValues(x0, y0, z0);
  Real f001 = getCornerValues(x0, y0, z1);
  Real f010 = getCornerValues(x0, y1, z0);
  Real f011 = getCornerValues(x0, y1, z1);
  Real f100 = getCornerValues(x1, y0, z0);
  Real f101 = getCornerValues(x1, y0, z1);
  Real f110 = getCornerValues(x1, y1, z0);
  Real f111 = getCornerValues(x1, y1, z1);

  // interpolation
  Real f00 = (f100 - f000) * Dx + f000;
  Real f10 = (f110 - f010) * Dx + f010;
  Real f01 = (f101 - f001) * Dx + f001;
  Real f11 = (f111 - f011) * Dx + f011;
  Real f0 = (f10 - f00) * Dy + f00;
  Real f1 = (f11 - f01) * Dy + f01;

  return (f1 - f0) * Dz + f0;
}
