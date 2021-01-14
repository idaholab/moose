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

// Forward declarations
class DT2;

namespace libMesh
{
template <typename T>
class NumericVector;
}

template <>
InputParameters validParams<DT2>();

/**
 * An adaptive timestepper that compares the solution obtained from a
 * single step of size dt with two steps of size dt/2 and adjusts the
 * next timestep accordingly.
 */
class DT2 : public TimeStepper
{
public:
  static InputParameters validParams();

  DT2(const InputParameters & parameters);

  virtual void preExecute() override;
  virtual void preSolve() override;
  virtual void step() override;

  virtual void rejectStep() override;
  virtual bool converged() const override;

protected:
  virtual Real computeInitialDT() override;
  virtual Real computeDT() override;

  ///
  NumericVector<Number> *_u_diff, *_u1, *_u2;
  NumericVector<Number> *_u_saved, *_u_older_saved;
  NumericVector<Number> *_aux1, *_aux_saved, *_aux_older_saved;

  /// global relative time discretization error estimate
  Real _error;
  /// error tolerance
  Real _e_tol;
  /// maximal error
  Real _e_max;
  /// maximum increase ratio
  Real _max_increase;
};
