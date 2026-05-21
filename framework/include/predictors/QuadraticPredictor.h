//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "Predictor.h"

/**
 * Quadratic Lagrange extrapolation predictor based on three accepted solutions.
 */
class QuadraticPredictor : public Predictor
{
public:
  static InputParameters validParams();

  QuadraticPredictor(const InputParameters & parameters);

  virtual int order() override { return 2; }
  virtual void timestepSetup() override;
  virtual bool shouldApply() override;
  virtual void apply(NumericVector<Number> & sln) override;

protected:
  /// Accepted solution at the previous solution time, u^{n-1}.
  NumericVector<Number> & _older_solution;

  /// Accepted solution at the solution time before that, u^{n-2}.
  NumericVector<Number> & _oldest_solution;

  /// Time step from u^{n-2} to u^{n-1}; used as h2 in the Lagrange formula.
  Real & _dt_older;

  /// Previous accepted time-step size saved during history rotation.
  Real & _dt_storage;

  /// Number of accepted solution states seen by the predictor history.
  unsigned int & _history_size;
};
