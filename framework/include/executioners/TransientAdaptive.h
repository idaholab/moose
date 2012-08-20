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

#ifndef TRANSIENTADAPTIVE_H
#define TRANSIENTADAPTIVE_H

#include "Executioner.h"
#include "LinearInterpolation.h"
#include "FEProblem.h"

#include "mesh_function.h"

// LibMesh includes
#include <parameters.h>
#include <point.h>
#include <vector_value.h>

// System includes
#include <string>

// Forward Declarations
class TransientAdaptive;
class TimeStepper;

enum TimeStepperStatus {
  STATUS_ITERATING                = 0,
  STATUS_CONVERGED_TIME           = 1,
  STATUS_CONVERGED_ITS            = 2,
  STATUS_DIVERGED_NONLINEAR_SOLVE = -1,
  STATUS_DIVERGED_STEP_REJECTED   = -2
};

template<>
InputParameters validParams<TransientAdaptive>();

/**
 * Transient executioners solve time steps sequentially.
 */
class TransientAdaptive: public Executioner
{
public:
  /**
   * Constructor
   *
   * @param name The name given to the Executioner in the input file.
   * @param parameters The parameters object holding data for the class to use.
   * @return Whether or not the solve was successful.
   */
  TransientAdaptive(const std::string & name, InputParameters parameters);

  virtual ~TransientAdaptive();

  virtual void execute();

  virtual Problem &problem() { return _fe_problem; }

protected:
  FEProblem & _fe_problem;
  TimeStepper *_time_stepper;
  bool keepGoing(TimeStepperStatus status, Real time) const;
};

#endif // TRANSIENTADAPTIVE_H
