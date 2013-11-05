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

#include "MooseObject.h"
#include "Restartable.h"
#include "libmesh/numeric_vector.h"

class Predictor;
class FEProblem;
class NonlinearSystem;

template<>
InputParameters validParams<Predictor>();

/**
 * Base class for predictors
 *
 */
class Predictor :
  public MooseObject,
  public Restartable
{
public:
  Predictor(const std::string & name, InputParameters parameters);
  virtual ~Predictor();

  virtual int order() { return 0; }
  virtual void apply(NumericVector<Number> & sln) = 0;

  virtual NumericVector<Number> & solutionPredictor() { return _solution_predictor; }

protected:
  FEProblem & _fe_problem;
  NonlinearSystem & _nl;

  int & _t_step;
  Real & _dt;
  Real & _dt_old;
  const NumericVector<Number> & _solution;
  NumericVector<Number> & _solution_old;
  NumericVector<Number> & _solution_older;
  NumericVector<Number> & _solution_predictor;

  ///
  Real _scale;
};

#endif /* PREDICTOR_H */
