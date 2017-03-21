/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "MTPiecewiseConst2D.h"

template <>
InputParameters
validParams<MTPiecewiseConst2D>()
{
  InputParameters params = validParams<Function>();
  return params;
}

MTPiecewiseConst2D::MTPiecewiseConst2D(const InputParameters & parameters) : Function(parameters) {}

Real
MTPiecewiseConst2D::value(Real /*t*/, const Point & p)
{
  Real val = 0;
  Real x = p(0);
  Real y = p(1);

  if ((x >= -0.6 && x < 0.2 && y >= 0.65 && y < 0.75) ||
      (x >= -0.6 && x < 0.2 && y >= 0.0 && y < 0.1) ||
      (x >= -0.6 && x < -.5 && y >= 0.0 && y < 0.75) ||
      (x >= 0.1 && x < 0.2 && y >= 0.0 && y < 0.75))
    val += 0.3;

  if ((x >= -0.1 && x < 0.65 && y >= 0.25 && y < 0.35) ||
      (x >= -0.1 && x < 0.65 && y >= -.15 && y < -.05) ||
      (x >= -0.1 && x < 0.65 && y >= -.55 && y < -.45) ||
      (x >= -0.1 && x < 0.0 && y >= -.15 && y < 0.35) ||
      (x >= 0.55 && x < 0.65 && y >= -.55 && y < -.05))
    val += 0.5;

  return val;
}
