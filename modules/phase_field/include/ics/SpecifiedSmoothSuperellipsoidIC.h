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
 * SpecifiedSmoothSuperellipsoidIC creates multiple SmoothSuperellipsoids (number = size of
 * x_positions) that are positioned in the
 * set locations with the set semiaxes a, b, c and exponents n
 */
class SpecifiedSmoothSuperellipsoidIC : public SmoothSuperellipsoidBaseIC
{
public:
  static InputParameters validParams();

  SpecifiedSmoothSuperellipsoidIC(const InputParameters & parameters);

protected:
  virtual void computeSuperellipsoidCenters();
  virtual void computeSuperellipsoidSemiaxes();
  virtual void computeSuperellipsoidExponents();

  std::vector<Real> _x_positions;
  std::vector<Real> _y_positions;
  std::vector<Real> _z_positions;
  std::vector<Real> _input_as;
  std::vector<Real> _input_bs;
  std::vector<Real> _input_cs;
  std::vector<Real> _input_ns;
};
