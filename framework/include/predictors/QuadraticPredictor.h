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
  /// Older accepted solution used in the quadratic history.
  NumericVector<Number> & _older_solution;

  /// Oldest accepted solution used in the quadratic history.
  NumericVector<Number> & _oldest_solution;

  /// Older time-step size used in the quadratic history.
  Real & _dt_older;

  /// Previous accepted time-step size saved during history rotation.
  Real & _dt_storage;

  /// Time for _older_solution.
  Real & _older_solution_time;

  /// Time for _oldest_solution.
  Real & _oldest_solution_time;

  /// Time storage matching the nonlinear system's older solution vector.
  Real & _solution_older_time_storage;

  /// Number of accepted solution states seen by the predictor history.
  unsigned int & _history_size;
};
