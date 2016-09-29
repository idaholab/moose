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
#ifndef ITERATIONADAPTIVEDT_H
#define ITERATIONADAPTIVEDT_H

#include "TimeStepper.h"
#include "LinearInterpolation.h"
#include "PostprocessorInterface.h"

class Function;
class Piecewise;

/**
 * Adjust the timestep based on the number of iterations.
 * The user can specitfy an optimal_iterations number of non-linear iterations and an iteration_window.
 * The time stepper attempts to increase the time step if the non-linear iteration count is below
 * optimal_iterations - iteration_window and it attempts to reduce the time step if the non-linear
 * iteration count is above optimal_iterations + iteration_window. Similar rules apply to the number
 * of linear iterations with the multiplier linear_iteration_ratio applied to optimal_iterations and
 * iteration_window.
 * This time stepper allows the user to specify a function that limits the maximal time step change.
 * This time stepper allows the user to specify a limiting time step length through a postprocessor.
 */
class IterationAdaptiveDT :
  public TimeStepper,
  public PostprocessorInterface
{
public:
  IterationAdaptiveDT(const InputParameters & parameters);

  virtual void init() override;

  virtual StepperBlock * buildStepper() override;

private:
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

  /// if specified, the postprocessor value is an upper limit for the time step length
  const PostprocessorValue * _pps_value;

  Function * _timestep_limiting_function;
  Piecewise * _piecewise_timestep_limiting_function;
  /// time point defined in the piecewise function
  std::vector<Real> _times;

  Real _max_function_change;
  /// insert sync points at the time nodes of the _piecewise_timestep_limiting_function
  bool _force_step_every_function_point;

  std::set<Real> _tfunc_times;

  /// Piecewise linear definition of time stepping
  LinearInterpolation _time_ipol;

  /// grow the timestep by this factor
  const Real _growth_factor;
  /// cut the timestep by by this factor
  const Real _cutback_factor;

  std::vector<Real> _tfunc_dts;
};

template<>
InputParameters validParams<IterationAdaptiveDT>();

#endif /* ITERATIONADAPTIVEDT_H */
