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

#ifndef DT2TRANSIENT_H
#define DT2TRANSIENT_H

#include "Moose.h"
#include "Transient.h"

// LibMesh includes
#include <parameters.h>
#include <point.h>
#include <vector_value.h>

// System includes
#include <string>

// Forward Declarations
class DT2Transient;

template<>
InputParameters validParams<DT2Transient>();

/**
 * TransientExecutioner executioners usually loop through a number of timesteps... calling solve()
 * for each timestep.
 */
class DT2Transient : public Transient
{
public:

  /**
   * Constructor
   *
   * @param name The name given to the Executioner in the input file.
   * @param parameters The parameters object holding data for the class to use.
   * @return Whether or not the solve was successful.
   */
  DT2Transient(const std::string & name, InputParameters parameters);

  virtual ~DT2Transient();

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

};

#endif //DT2TRANSIENT_H
