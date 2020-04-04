//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

class EFAPoint
{
public:
  EFAPoint(const double x = 0., const double y = 0., const double z = 0.);

private:
  double _x;
  double _y;
  double _z;

public:
  double operator()(const unsigned int i) const;
  EFAPoint & operator/=(const double a);
  EFAPoint & operator*=(const double a);
  EFAPoint & operator+=(const EFAPoint & point);
  EFAPoint operator*(const double a);
  double operator*(const EFAPoint & point);
  EFAPoint operator+(const EFAPoint & point);
  EFAPoint operator-(const EFAPoint & point);
  double norm();
  void zero();
  EFAPoint cross(const EFAPoint & point);
};
