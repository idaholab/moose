//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

/*
Define Function for Spatial Distribution of Static Friction Coefficient Mu_s
Problem-Specific: TPV205-2D
*/

#include "StaticFricCoeffMus.h"

registerMooseObject("TensorMechanicsApp", StaticFricCoeffMus);

InputParameters
StaticFricCoeffMus::validParams()
{
  InputParameters params = Function::validParams();
  params.addClassDescription("Static Friction Spatial Distribution.");
  params.addParam<Real>("xcoord_left", -15.0e3, "x coordinate (left end of patch)");
  params.addParam<Real>("xcoord_right", 15.0e3, "x coordinate (right end of patch)");
  params.addParam<Real>("mu_s_weakening_patch", 0.677, "mu_s weakening patch");
  params.addParam<Real>("mu_s_strengthing_patch", 10000.0, "mu_s strengthing patch");
  return params;
}

StaticFricCoeffMus::StaticFricCoeffMus(const InputParameters & parameters)
  : Function(parameters),
    _xcoord_left(getParam<Real>("xcoord_left")),
    _xcoord_right(getParam<Real>("xcoord_right")),
    _mu_s_weakening_patch(getParam<Real>("mu_s_weakening_patch")),
    _mu_s_strengthing_patch(getParam<Real>("mu_s_strengthing_patch"))
{
}

Real
StaticFricCoeffMus::value(Real /*t*/, const Point & p) const
{

  Real x_coord = p(0);

  double mu_s = 0.0;
  if (x_coord >= _xcoord_left && x_coord <= _xcoord_right)
  {
    mu_s = _mu_s_weakening_patch;
  }
  else
  {
    mu_s = _mu_s_strengthing_patch;
  }
  return mu_s;
}
