//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SmoothSuperellipsoidBaseIC.h"

/**
 * SmoothSuperellipsoidIC creates a Superellipsoid of given semiaxes a,b,c and exponent n
 * centered at a given point in the domain.
 * If int_width > zero, the border of the Superellipsoid with smoothly transition from
 * the invalue to the outvalue.
 */
class SmoothSuperellipsoidIC : public SmoothSuperellipsoidBaseIC
{
public:
  static InputParameters validParams();

  SmoothSuperellipsoidIC(const InputParameters & parameters);

protected:
  virtual void computeSuperellipsoidCenters();
  virtual void computeSuperellipsoidSemiaxes();
  virtual void computeSuperellipsoidExponents();

  const Real _x1;
  const Real _y1;
  const Real _z1;
  const Real _a;
  const Real _b;
  const Real _c;
  const Real _n;
  const Point _center;
};
