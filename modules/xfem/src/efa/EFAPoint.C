//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "EFAPoint.h"
#include <cmath>
#include "EFAError.h"

EFAPoint::EFAPoint(const double x, const double y, const double z) : _x(x), _y(y), _z(z) {}

double
EFAPoint::operator()(const unsigned int i) const
{
  switch (i)
  {
    case 0:
      return _x;
    case 1:
      return _y;
    case 2:
      return _z;
    default:
      EFAError("EFAPoint: Out of index range.");
  }
}

EFAPoint &
EFAPoint::operator/=(const double a)
{
  _x /= a;
  _y /= a;
  _z /= a;
  return *this;
}

EFAPoint &
EFAPoint::operator*=(const double a)
{
  _x *= a;
  _y *= a;
  _z *= a;
  return *this;
}

EFAPoint &
EFAPoint::operator+=(const EFAPoint & point)
{
  _x += point._x;
  _y += point._y;
  _z += point._z;
  return *this;
}

EFAPoint
EFAPoint::operator*(const double a)
{
  return EFAPoint(this->_x * a, this->_y * a, this->_z * a);
}

double
EFAPoint::operator*(const EFAPoint & point)
{
  return this->_x * point._x + this->_y * point._y + this->_z * point._z;
}

EFAPoint
EFAPoint::operator+(const EFAPoint & point)
{
  return EFAPoint(this->_x + point._x, this->_y + point._y, this->_z + point._z);
}

EFAPoint
EFAPoint::operator-(const EFAPoint & point)
{
  return EFAPoint(this->_x - point._x, this->_y - point._y, this->_z - point._z);
}

double
EFAPoint::norm()
{
  return std::sqrt(_x * _x + _y * _y + _z * _z);
}

void
EFAPoint::zero()
{
  _x = 0.0;
  _y = 0.0;
  _z = 0.0;
}

EFAPoint
EFAPoint::cross(const EFAPoint & point)
{
  double x = this->_y * point._z - this->_z * point._y;
  double y = this->_z * point._x - this->_x * point._z;
  double z = this->_x * point._y - this->_y * point._x;
  return EFAPoint(x, y, z);
}
