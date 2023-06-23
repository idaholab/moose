//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

/*
Define Function for Initial Shear Stress along Strike Direction
Problem-Specific: TPV205-2D
*/

#include "InitialStrikeShearStress.h"

registerMooseObject("TensorMechanicsTestApp", InitialStrikeShearStress);

InputParameters
InitialStrikeShearStress::validParams()
{
  InputParameters params = Function::validParams();
  params.addClassDescription("Initial Shear Stress Spatial Distribution.");
  params.addParam<Real>("len", 1.5e3, "half length of initialization patch");
  params.addParam<Real>("xcoord_leftpatchcenter", -7.5e3, "x coordinate (center of left patch)");
  params.addParam<Real>("xcoord_middlepatchcenter", 0.0, "x coordinate (center of middle patch)");
  params.addParam<Real>("xcoord_rightpathcenter", 7.5e3, "x coordinate (center of right patch)");
  params.addParam<Real>("Tso_leftpatch", 78.0e6, "initial shear traction (left patch)");
  params.addParam<Real>("Tso_centerpatch", 81.6e6, "initial shear traction (center patch)");
  params.addParam<Real>("Tso_rightpatch", 62.0e6, "initial shear traction (right)");
  params.addParam<Real>("Tso_else", 62.0e6, "initial shear traction (elsewhere)");
  return params;
}

InitialStrikeShearStress::InitialStrikeShearStress(const InputParameters & parameters)
  : Function(parameters),
    _len(getParam<Real>("len")),
    _xcoord_leftpatchcenter(getParam<Real>("xcoord_leftpatchcenter")),
    _xcoord_middlepatchcenter(getParam<Real>("xcoord_middlepatchcenter")),
    _xcoord_rightpathcenter(getParam<Real>("xcoord_rightpathcenter")),
    _Tso_leftpatch(getParam<Real>("Tso_leftpatch")),
    _Tso_centerpatch(getParam<Real>("Tso_centerpatch")),
    _Tso_rightpatch(getParam<Real>("Tso_rightpatch")),
    _Tso_else(getParam<Real>("Tso_else"))
{
}

Real
InitialStrikeShearStress::value(Real /*t*/, const Point & p) const
{

  Real x_coord = p(0);

  double T1_o = 0.0;
  if ((x_coord <= (_xcoord_middlepatchcenter + _len)) &&
      (x_coord >= (_xcoord_middlepatchcenter - _len)))
  {
    T1_o = _Tso_centerpatch;
  }
  else if ((x_coord <= (_xcoord_leftpatchcenter + _len)) &&
           (x_coord >= (_xcoord_leftpatchcenter - _len)))
  {
    T1_o = _Tso_leftpatch;
  }
  else if ((x_coord <= (_xcoord_rightpathcenter + _len)) &&
           (x_coord >= (_xcoord_rightpathcenter - _len)))
  {
    T1_o = _Tso_rightpatch;
  }
  else
  {
    T1_o = _Tso_else;
  }
  return T1_o;
}
