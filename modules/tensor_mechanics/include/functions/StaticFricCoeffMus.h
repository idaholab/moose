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
*/

#pragma once

#include "Function.h"

class StaticFricCoeffMus : public Function
{
public:
  StaticFricCoeffMus(const InputParameters & parameters);

  static InputParameters validParams();

  using Function::value;
  virtual Real value(Real t, const Point & p) const override;

  // x coordinate (left end of patch)
  Real _xcoord_left;

  // x coordinate (right end of patch)
  Real _xcoord_right;

  // mu_s weakening patch
  Real _mu_s_weakening_patch;

  // mu_s strengthing patch
  Real _mu_s_strengthing_patch;
};
