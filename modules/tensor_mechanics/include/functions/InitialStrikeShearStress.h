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

#pragma once

#include "Function.h"

class InitialStrikeShearStress : public Function
{
public:
  InitialStrikeShearStress(const InputParameters & parameters);

  static InputParameters validParams();

  using Function::value;
  virtual Real value(Real t, const Point & p) const override;

  //half length of initialization patch
  Real _len;

  //x coordinate (center of left patch)
  Real _xcoord_leftpatchcenter;

  //x coordinate (center of middle patch)
  Real _xcoord_middlepatchcenter;

  //x coordinate (center of right patch)
  Real _xcoord_rightpathcenter;

  //initial shear traction (left patch)
  Real _Tso_leftpatch;

  //initial shear traction (center patch)
  Real _Tso_centerpatch;

  //initial shear traction (right)
  Real _Tso_rightpatch;

  //initial shear traction (elsewhere)
  Real _Tso_else;
};
