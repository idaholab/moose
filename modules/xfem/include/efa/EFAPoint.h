/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef EFAPOINT_H
#define EFAPOINT_H

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

#endif
