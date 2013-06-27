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

#include "TimeIntegrator.h"
#include "libmesh/numeric_vector.h"

class Predictor;

template<>
InputParameters validParams<Predictor>();

/**
 * Base class for predictors
 *
 * Currently it implements only a simple predictor
 *
 * A Predictor is an algorithm that will predict the next solution based on
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
class Predictor : public TimeIntegrator
{
public:
  Predictor(const std::string & name, InputParameters parameters);
  virtual ~Predictor();

  virtual int order() { return 0; }
  virtual void computeTimeDerivatives() { }
  virtual void apply(NumericVector<Number> & sln);

protected:
  const NumericVector<Number> & _solution;
  NumericVector<Number> & _solution_old;
  NumericVector<Number> & _solution_older;

  ///
  Real _scale;
};

#endif /* PREDICTOR_H */
