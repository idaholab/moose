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

#ifndef NUMNONLINEARITERATIONS_H
#define NUMNONLINEARITERATIONS_H

#include "GeneralPostprocessor.h"

// Forward Declarations
class NumNonlinearIterations;

template <>
InputParameters validParams<NumNonlinearIterations>();

/**
 * NumNonlinearIterations is a postprocessor that reports the number of nonlinear iterations
 */

class NumNonlinearIterations : public GeneralPostprocessor
{
public:
  NumNonlinearIterations(const InputParameters & parameters);

  /**
   * Initialization to be done at each timestep
   */
  virtual void timestepSetup() override;

  virtual void initialize() override {}
  virtual void execute() override {}

  /**
   * Get the numer of nonlinear iterations
   */
  virtual Real getValue() override;

protected:
  /// Pointer to the FEProblemBase
  FEProblemBase * _fe_problem;

  /// True if we should accumulate over all nonlinear solves done as part of Picard iterations in a step.
  bool _accumulate_over_step;

  /// Stores the nonlinear iteration count
  unsigned int _num_iters;

  /// Stores the last time this was executed
  Real _time;
};

#endif // NUMNONLINEARITERATIONS_H
