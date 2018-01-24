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

#ifndef DT2_H
#define DT2_H

// MOOSE includes
#include "TimeStepper.h"

// Forward declarations
class DT2;

namespace libMesh
{
template <typename T>
class NumericVector;
}

template <>
InputParameters validParams<DT2>();

/**
 * An adaptive timestepper that compares the solution obtained from a
 * single step of size dt with two steps of size dt/2 and adjusts the
 * next timestep accordingly.
 */
class DT2 : public TimeStepper
{
public:
  DT2(const InputParameters & parameters);

  virtual void preExecute() override;
  virtual void preSolve() override;
  virtual void step() override;

  virtual void rejectStep() override;
  virtual bool converged() override;

protected:
  virtual Real computeInitialDT() override;
  virtual Real computeDT() override;

  ///
  NumericVector<Number> *_u_diff, *_u1, *_u2;
  NumericVector<Number> *_u_saved, *_u_older_saved;
  NumericVector<Number> *_aux1, *_aux_saved, *_aux_older_saved;

  /// global relative time discretization error estimate
  Real _error;
  /// error tolerance
  Real _e_tol;
  /// maximal error
  Real _e_max;
  /// maximum increase ratio
  Real _max_increase;
};

#endif /* DT2_H */
