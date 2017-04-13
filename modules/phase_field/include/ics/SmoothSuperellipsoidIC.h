/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef SMOOTHSUPERELLIPSOIDIC_H
#define SMOOTHSUPERELLIPSOIDIC_H

#include "SmoothSuperellipsoidBaseIC.h"

// Forward Declarations
class SmoothSuperellipsoidIC;

template <>
InputParameters validParams<SmoothSuperellipsoidIC>();

/**
 * SmoothSuperellipsoidIC creates a Superellipsoid of given semiaxes a,b,c and exponent n
 * centered at a given point in the domain.
 * If int_width > zero, the border of the Superellipsoid with smoothly transition from
 * the invalue to the outvalue.
 */
class SmoothSuperellipsoidIC : public SmoothSuperellipsoidBaseIC
{
public:
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

#endif // SMOOTHSUPERELLIPSOIDIC_H
