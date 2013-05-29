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

#ifndef DT2_H_
#define DT2_H_

#include "TimeStepper.h"
#include "libmesh/numeric_vector.h"

class DT2;

template<>
InputParameters validParams<DT2>();

/**
 *
 */
class DT2 : public TimeStepper
{
public:
  DT2(const std::string & name, InputParameters parameters);

  virtual void preExecute();
  virtual void preSolve();
  virtual void step();

  virtual void computeInitialDT();
  virtual void computeDT();

  virtual void rejectStep();
  virtual bool converged();

protected:
  ///
  NumericVector<Number> * _u_diff, * _u1, * _u2;
  NumericVector<Number> * _u_saved, * _u_older_saved;
  NumericVector<Number> * _aux1, * _aux_saved, * _aux_older_saved;

  /// global relative time discretization error estimate
  Real _error;
  /// error tolerance
  Real _e_tol;
  /// maximal error
  Real _e_max;
  /// maximum increase ratio
  Real _max_increase;
};


#endif /* DT2_H_ */
