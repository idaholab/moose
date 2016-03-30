/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef SMOOTHSUPERELLIPSOIDBASEIC_H
#define SMOOTHSUPERELLIPSOIDBASEIC_H

#include "Kernel.h"
#include "InitialCondition.h"
#include "MooseRandom.h"

// System includes
#include <string>

// Forward Declarations
class SmoothSuperellipsoidBaseIC;

template<>
InputParameters validParams<SmoothSuperellipsoidBaseIC>();

/**
 * SmoothSuperellipsoidBaseIC is the base class for all initial conditions that create superellipsoids.
 * A superellipsoid is described by \f$ \left|\frac{x}{a}\right|^n + \left|\frac{y}{b}\right|^n + \left|\frac{z}{c}\right|^n = 1\f$.
 * Note that all children must resize _a, _b, _c, _n, and _centers.
 */
class SmoothSuperellipsoidBaseIC : public InitialCondition
{
public:
  SmoothSuperellipsoidBaseIC(const InputParameters & parameters);

  virtual Real value(const Point & p);
  virtual RealGradient gradient(const Point & p);

  virtual void initialSetup();

protected:
  virtual Real computeSuperellipsoidValue(const Point & p, const Point & center, const Real & a, const Real & b, const Real & c, const Real & n);
  virtual RealGradient computeSuperellipsoidGradient(const Point & p, const Point & center, const Real & a, const Real & b, const Real & c, const Real & n);

  virtual void computeSuperellipsoidRadii() = 0;
  virtual void computeSuperellipsoidCenters() = 0;

  MooseMesh & _mesh;

  Real _invalue;
  Real _outvalue;
  Real _int_width;
  bool _zero_gradient;

  unsigned int _num_dim;

  std::vector<Point> _centers;
  std::vector<Real> _a;
  std::vector<Real> _b;
  std::vector<Real> _c;
  std::vector<Real> _n;

  MooseRandom _random;
};

#endif //SMOOTHSUPERELLIPSOIDBASEIC_H
