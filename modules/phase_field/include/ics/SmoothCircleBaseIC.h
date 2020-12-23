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
 * SmoothcircleBaseIC is the base class for all initial conditions that create circles. The circles
 * can have sharp interfaces or a finite interface width. Note that all children must resize _radii
 * and _centers.
 */
class SmoothCircleBaseIC : public InitialCondition
{
public:
  static InputParameters validParams();

  SmoothCircleBaseIC(const InputParameters & parameters);

  virtual Real value(const Point & p);
  virtual RealGradient gradient(const Point & p);

  virtual void initialSetup();

protected:
  virtual Real computeCircleValue(const Point & p, const Point & center, const Real & radius);
  virtual RealGradient
  computeCircleGradient(const Point & p, const Point & center, const Real & radius);

  virtual void computeCircleRadii() = 0;
  virtual void computeCircleCenters() = 0;

  MooseMesh & _mesh;

  Real _invalue;
  Real _outvalue;
  Real _int_width;
  bool _3D_spheres;
  bool _zero_gradient;

  unsigned int _num_dim;

  std::vector<Point> _centers;
  std::vector<Real> _radii;

  enum class ProfileType
  {
    COS,
    TANH
  } _profile;

  MooseRandom _random;
};
