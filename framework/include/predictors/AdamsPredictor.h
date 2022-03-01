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
#include "Predictor.h"

namespace libMesh
{
template <typename T>
class NumericVector;
}

/**
 * Implements an explicit Adams predictor based on two old solution
 * vectors.
 */
class AdamsPredictor : public Predictor
{
public:
  static InputParameters validParams();

  AdamsPredictor(const InputParameters & parameters);

  virtual int order() override { return _order; }
  virtual void timestepSetup() override;
  virtual bool shouldApply() override;
  virtual void apply(NumericVector<Number> & sln) override;
  virtual NumericVector<Number> & solutionPredictor() override { return _solution_predictor; }

protected:
  int _order;
  NumericVector<Number> & _current_old_solution;
  NumericVector<Number> & _older_solution;
  NumericVector<Number> & _oldest_solution;
  NumericVector<Number> & _tmp_previous_solution;
  NumericVector<Number> & _tmp_residual_old;
  NumericVector<Number> & _tmp_third_vector;
  Real & _dt_older;
  Real & _dtstorage;
};
