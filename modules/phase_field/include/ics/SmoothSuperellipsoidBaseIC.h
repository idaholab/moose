//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InitialCondition.h"
#include "MooseRandom.h"

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
  static InputParameters validParams();

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
