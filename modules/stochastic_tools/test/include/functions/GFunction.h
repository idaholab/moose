//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#pragma once

#include "Function.h"

/* SOBOL test function, see Slaughter, Eq. 5.51 */
class GFunction : public Function
{
public:
  static InputParameters validParams();
  GFunction(const InputParameters & parameters);
  virtual Real value(Real t, const Point & p) const override;

protected:
  const std::vector<Real> & _q_vector;
  const std::vector<Real> & _x_vector;
};
