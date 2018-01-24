/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef SMOOTHSUPERELLIPSOIDBASEIC_H
#define SMOOTHSUPERELLIPSOIDBASEIC_H

#include "InitialCondition.h"
#include "MooseRandom.h"

// Forward Declarations
class SmoothSuperellipsoidBaseIC;

template <>
InputParameters validParams<SmoothSuperellipsoidBaseIC>();

/**
 * SmoothSuperellipsoidBaseIC is the base class for all initial conditions that create
 * superellipsoids.
 * A superellipsoid is described by \f$ \left|\frac{x}{a}\right|^n + \left|\frac{y}{b}\right|^n +
 * \left|\frac{z}{c}\right|^n = 1\f$.
 * Note that all children must resize _as, _bs, _cs, _ns, and _centers.
 */
class SmoothSuperellipsoidBaseIC : public InitialCondition
{
public:
  SmoothSuperellipsoidBaseIC(const InputParameters & parameters);

  virtual Real value(const Point & p);
  virtual RealGradient gradient(const Point & p);

  virtual void initialSetup();

protected:
  virtual Real
  computeSuperellipsoidValue(const Point & p, const Point & center, Real a, Real b, Real c, Real n);
  virtual Real computeSuperellipsoidInverseValue(
      const Point & p, const Point & center, Real a, Real b, Real c, Real n);
  RealGradient computeSuperellipsoidGradient(
      const Point & p, const Point & center, Real a, Real b, Real c, Real n);

  virtual void computeSuperellipsoidSemiaxes() = 0;
  virtual void computeSuperellipsoidExponents() = 0;
  virtual void computeSuperellipsoidCenters() = 0;

  MooseMesh & _mesh;

  Real _invalue;
  Real _outvalue;
  Real _nestedvalue;
  Real _int_width;
  bool _zero_gradient;

  unsigned int _num_dim;

  std::vector<Point> _centers;
  std::vector<Real> _as;
  std::vector<Real> _bs;
  std::vector<Real> _cs;
  std::vector<Real> _ns;

  MooseRandom _random;
};

#endif // SMOOTHSUPERELLIPSOIDBASEIC_H
