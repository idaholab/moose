//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "TimeStepper.h"

// C++ includes
#include <fstream>

namespace libMesh
{
template <typename T>
class NumericVector;
}

/**
 * A TimeStepper based on the AB2 method.  Increases the timestep if
 * the difference between the actual and AB2-predicted solutions is
 * small enough.
 */
class AB2PredictorCorrector : public TimeStepper
{
public:
  static InputParameters validParams();

  AB2PredictorCorrector(const InputParameters & parameters);

  virtual void step() override;
  virtual void preExecute() override;
  virtual void preSolve() override;
  virtual void postSolve() override;
  virtual bool converged() const override;

protected:
  virtual Real computeDT() override;
  virtual Real computeInitialDT() override;

  virtual Real estimateTimeError(NumericVector<Number> & sol);

  NumericVector<Number> & _u1;
  NumericVector<Number> & _aux1;
  NumericVector<Number> & _pred1;

  /// dt of the big step
  Real & _dt_full;

  /// global relative time discretization error estimate
  Real & _error;
  /// error tolerance
  Real _e_tol;
  /// maximal error
  Real _e_max;
  /// maximum increase ratio
  Real _max_increase;
  /// steps to take before increasing dt
  int _steps_between_increase;
  /// steps taken at current dt
  int & _dt_steps_taken;
  int _start_adapting;
  Real & _my_dt_old;
  ///infinity norm of the solution vector
  Real & _infnorm;
  /// scaling_parameter for time step selection, default is 0.8
  Real _scaling_parameter;
  std::ofstream myfile;
};
