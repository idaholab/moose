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

#ifndef ITERATIONADAPTIVESTEPPER_H
#define ITERATIONADAPTIVESTEPPER_H

#include "Stepper.h"

class IterationAdaptiveStepper;

template<>
InputParameters validParams<IterationAdaptiveStepper>();

/**
 * Adjust the timestep based on the number of iterations.
 * The user can specitfy an optimal_iterations number of non-linear iterations and an iteration_window.
 * The time stepper attempts to increase the time step if the non-linear iteration count is below
 * optimal_iterations - iteration_window and it attempts to reduce the time step if the non-linear
 * iteration count is above optimal_iterations + iteration_window. Similar rules apply to the number
 * of linear iterations with the multiplier linear_iteration_ratio applied to optimal_iterations and
 * iteration_window.
 */
class IterationAdaptiveStepper : public Stepper
{
public:
  IterationAdaptiveStepper(const InputParameters & parameters);

  virtual Real computeInitialDT() override;

  virtual Real computeDT() override;

  virtual Real computeFailedDT() override;

protected:
  /// The dt from the input file.
  const Real _input_dt;

  /// Adapt the timestep to maintain this non-linear iteration count...
  int _optimal_iterations;
  /// ...plus/minus this value.
  int _iteration_window;
  /// use _optimal_iterations and _iteration_window multiplied with this factor for linear iterations
  const int _linear_iteration_ratio;
  /// adaptive timestepping is active if the optimal_iterations input parameter is specified
  bool _adaptive_timestepping;

  /// grow the timestep by this factor
  const Real _growth_factor;
  /// cut the timestep by by this factor
  const Real _cutback_factor;
};

#endif /* ITERATIONADAPTIVESTEPPER_H */
