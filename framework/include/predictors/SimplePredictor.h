//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Predictor.h"

/**
 * A SimplePredictor uses an algorithm that will predict the next solution based on
 * previous solutions.  Basically, it works like:
 *
 *             sol - prev_sol
 * sol = sol + -------------- * dt * scale_factor
 *                 dt_old
 *
 * The scale factor can be set to 1 for times when the solution is expected
 * to change linearly or smoothly.  If the solution is less continuous over
 * time, it may be better to set to to 0.
 *   In the ideal case of a linear model with linearly changing bcs, the Predictor
 * can determine the solution before the solver is invoked (a solution is computed
 * in zero solver iterations).  Even outside the ideal case, a good Predictor
 * significantly reduces the number of solver iterations required.
 *  It is important to compute the initial residual to be used as a relative
 * convergence criterion before applying the predictor.  If this is not done,
 * the residual is likely to be much lower after applying the predictor, which would
 * result in a much more stringent criterion for convergence than would have been
 * used if the predictor were not enabled.
 *
 */
class SimplePredictor : public Predictor
{
public:
  static InputParameters validParams();

  SimplePredictor(const InputParameters & parameters);

  virtual bool shouldApply() override;
  virtual void apply(NumericVector<Number> & sln) override;
};
