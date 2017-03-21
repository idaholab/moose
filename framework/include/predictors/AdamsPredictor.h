/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef ADAMSPREDICTOR_H
#define ADAMSPREDICTOR_H

// MOOSE includes
#include "Predictor.h"

// Forward declarations
class AdamsPredictor;

namespace libMesh
{
template <typename T>
class NumericVector;
}

template <>
InputParameters validParams<AdamsPredictor>();

/**
 * Implements an explicit Adams predictor based on two old solution
 * vectors.
 */
class AdamsPredictor : public Predictor
{
public:
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
  int & _t_step_old;
  Real & _dt_older;
  Real & _dtstorage;
};

#endif /* ADAMSPREDICTOR_H */
