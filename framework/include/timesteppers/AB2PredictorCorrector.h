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

#ifndef AB2PREDICTORCORRECTOR_H
#define AB2PREDICTORCORRECTOR_H

#include "TimeStepper.h"
#include <iostream>
#include <fstream>
#include "libmesh/numeric_vector.h"

// System includes
#include <string>

// Forward Declarations
class AB2PredictorCorrector;

template<>
InputParameters validParams<AB2PredictorCorrector>();

/**
 *
 */
class AB2PredictorCorrector : public TimeStepper
{
public:
  AB2PredictorCorrector(const std::string & name, InputParameters parameters);
  virtual ~AB2PredictorCorrector();

  virtual Real computeDT();
  virtual Real computeInitialDT();
  virtual void step();
  virtual void preExecute();
  virtual void preSolve();
  virtual bool converged();

protected:
  virtual Real estimateTimeError(NumericVector<Number> & sol);

  virtual int stringtoint(std::string string);
  ///
  NumericVector<Number> & _u1;
  NumericVector<Number> & _aux1;

  /// dt of the big step
  Real _dt_full;

  /// global relative time discretization error estimate
  Real _error;
  /// error tolerance
  Real _e_tol;
  /// maximal error
  Real _e_max;
  /// maximum increase ratio
  Real _max_increase;
  /// steps to take before increasing dt
  int _steps_between_increase;
  /// steps taken at current dt
  int _dt_steps_taken;
  int _start_adapting;
  Real _my_dt_old;
  ///infinity norm of the solution vector
  Real _infnorm;
  /// scaling_parameter for time step selection, default is 0.8
  Real _scaling_parameter;
  std::ofstream myfile;
};

#endif // AB2PREDICTORCORRECTOR_H
