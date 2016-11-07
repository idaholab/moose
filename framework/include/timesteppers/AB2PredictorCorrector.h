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

// MOOSE includes
#include "TimeStepper.h"

// C++ includes
#include <fstream>

// Forward Declarations
class AB2PredictorCorrector;

namespace libMesh
{
template <typename T>
class NumericVector;
}

template <>
InputParameters validParams<AB2PredictorCorrector>();

/**
 * A TimeStepper based on the AB2 method.  Increases the timestep if
 * the difference between the actual and AB2-predicted solutions is
 * small enough.
 */
class AB2PredictorCorrector : public TimeStepper
{
public:
  AB2PredictorCorrector(const InputParameters & parameters);

  virtual void step() override;
  virtual void preExecute() override;
  virtual void preSolve() override;
  virtual bool converged() override;

protected:
  virtual Real computeDT() override;
  virtual Real computeInitialDT() override;

  virtual Real estimateTimeError(NumericVector<Number> & sol);

  int stringtoint(std::string string);
  ///
  NumericVector<Number> & _u1;
  NumericVector<Number> & _aux1;
  NumericVector<Number> & _pred1;

  /// dt of the big step
  Real & _dt_full;

  /// global relative time discretization error estimate
  Real & _error;
  /// error tolerance
  Real _e_tol;
  /// maximal error
  Real _e_max;
  /// maximum increase ratio
  Real _max_increase;
  /// steps to take before increasing dt
  int _steps_between_increase;
  /// steps taken at current dt
  int & _dt_steps_taken;
  int _start_adapting;
  Real & _my_dt_old;
  ///infinity norm of the solution vector
  Real & _infnorm;
  /// scaling_parameter for time step selection, default is 0.8
  Real _scaling_parameter;
  std::ofstream myfile;
};

#endif // AB2PREDICTORCORRECTOR_H
