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

#ifndef PREDICTOR_H
#define PREDICTOR_H

// MOOSE includes
#include "MooseObject.h"
#include "Restartable.h"

// Forward declarations
class Predictor;
class FEProblemBase;
class NonlinearSystemBase;

namespace libMesh
{
template <typename T>
class NumericVector;
}

template <>
InputParameters validParams<Predictor>();

/**
 * Base class for predictors.
 */
class Predictor : public MooseObject, public Restartable
{
public:
  Predictor(const InputParameters & parameters);
  virtual ~Predictor();

  virtual int order() { return 0; }
  virtual void timestepSetup();
  virtual bool shouldApply();
  virtual void apply(NumericVector<Number> & sln) = 0;

  virtual NumericVector<Number> & solutionPredictor() { return _solution_predictor; }

protected:
  FEProblemBase & _fe_problem;
  NonlinearSystemBase & _nl;

  int & _t_step;
  Real & _dt;
  Real & _dt_old;
  const NumericVector<Number> & _solution;
  NumericVector<Number> & _solution_old;
  NumericVector<Number> & _solution_older;
  NumericVector<Number> & _solution_predictor;

  /// Amount by which to scale the predicted value.  Must be in [0,1].
  Real _scale;

  /// Times for which the predictor should not be applied
  std::vector<Real> _skip_times;

  /// Old times for which the predictor should not be applied
  std::vector<Real> _skip_times_old;
};

#endif /* PREDICTOR_H */
