/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef SMOOTHSUPERELLIPSOIDIC_H
#define SMOOTHSUPERELLIPSOIDIC_H

#include "Kernel.h"
#include "SmoothSuperellipsoidBaseIC.h"

// System includes
#include <string>

// Forward Declarations
class SmoothSuperellipsoidIC;

template<>
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

  virtual void computeSuperellipsoidCenters();
  virtual void computeSuperellipsoidSemiaxes();
  virtual void computeSuperellipsoidExponents();

protected:
  Real _x1;
  Real _y1;
  Real _z1;
  Real _a;
  Real _b;
  Real _c;
  Real _n;
  Point _center;
};

#endif //SMOOTHSUPERELLIPSOIDIC_H
