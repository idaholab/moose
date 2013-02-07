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

#ifndef AdaptiveErrorEstimateTRANSIENT_H
#define AdaptiveErrorEstimateTRANSIENT_H

#include "Transient.h"
#include "InputParameters.h"

// LibMesh includes
#include "numeric_vector.h"

// System includes
#include <string>
#include <fstream>

// Forward Declarations
class AdaptiveErrorEstimateTransient;

template<>
InputParameters validParams<AdaptiveErrorEstimateTransient>();

/**
 * TransientExecutioner executioners usually loop through a number of timesteps... calling solve()
 * for each timestep.
 */
class AdaptiveErrorEstimateTransient : public Transient
{
public:

  /**
   * Constructor
   *
   * @param name The name given to the Executioner in the input file.
   * @param parameters The parameters object holding data for the class to use.
   * @return Whether or not the solve was successful.
   */
  AdaptiveErrorEstimateTransient(const std::string & name, InputParameters parameters);

  virtual ~AdaptiveErrorEstimateTransient();

  /**
   * Optional override.
   *
   * @return The dt to use for this timestep.
   */
  virtual Real computeDT();

  /**
   * Whether or not the last solve converged.
   */
  virtual bool lastSolveConverged();

  virtual void preExecute();

protected:

  virtual void preSolve();

  virtual void postSolve();

  ///
  NumericVector<Number> * _u_diff, * _u1, * _u2;

  NumericVector<Number> * _u_saved, * _u_older_saved;
  NumericVector<Number> * _aux1, * _aux_saved, * _aux_older_saved;

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

#endif //DT2TRANSIENT_H
