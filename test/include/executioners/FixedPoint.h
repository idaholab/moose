//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SolveObject.h"

#include "FixedPointProblem.h"

class FixedPoint : public SolveObject
{
public:
  static InputParameters validParams();

  FixedPoint(Executioner & ex);

  virtual bool solve() override;

protected:
  FixedPointProblem & _fp_problem;
  const unsigned int _fp_max_its;
  const Real _fp_abs_tol;
  const Real _fp_rel_tol;
  const Real _fp_abs_step_tol;
  const Real _fp_rel_step_tol;
};
