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

#include "IterationAdaptiveStepper.h"
#include "FEProblem.h"
#include "Transient.h"
#include "MooseApp.h"

template<>
InputParameters validParams<IterationAdaptiveStepper>()
{
  InputParameters params = validParams<Stepper>();

  params.addClassDescription("Adjust the timestep based on the number of iterations");
  params.addRequiredParam<Real>("dt", "The default timestep size between solves");
  params.addRequiredParam<int>("optimal_iterations", "The target number of nonlinear iterations for adaptive timestepping");
  params.addParam<int>("iteration_window", "Attempt to grow/shrink timestep if the iteration count is below/above 'optimal_iterations plus/minus iteration_window' (default = optimal_iterations/5).");
  params.addParam<unsigned>("linear_iteration_ratio", "The ratio of linear to nonlinear iterations to determine target linear iterations and window for adaptive timestepping (default = 25)");
  params.addParam<Real>("growth_factor", 2.0, "Factor to apply to timestep if easy convergence (if 'optimal_iterations' is specified) or if recovering from failed solve");
  params.addParam<Real>("cutback_factor", 0.5, "Factor to apply to timestep if difficult convergence (if 'optimal_iterations' is specified) or if solution failed");

  return params;
}

IterationAdaptiveStepper::IterationAdaptiveStepper(const InputParameters & parameters) :
    Stepper(parameters),
    _input_dt(getParam<Real>("dt")),
    _optimal_iterations(getParam<int>("optimal_iterations")),
    _linear_iteration_ratio(isParamValid("linear_iteration_ratio") ? getParam<unsigned>("linear_iteration_ratio") : 25),  // Default to 25
    _adaptive_timestepping(false),
    _growth_factor(getParam<Real>("growth_factor")),
    _cutback_factor(getParam<Real>("cutback_factor"))
{
  if (isParamValid("iteration_window"))
    _iteration_window = getParam<int>("iteration_window");
  else
    _iteration_window = std::ceil(_optimal_iterations / 5.0);
}

Real
IterationAdaptiveStepper::computeInitialDT()
{
  return _input_dt;
}

Real
IterationAdaptiveStepper::computeDT()
{
  bool can_shrink = true;
  bool can_grow = _converged[0] && _converged[1];

  unsigned int growth_nl_its = 0;
  unsigned int growth_l_its = 0;
  unsigned int shrink_nl_its = _optimal_iterations + _iteration_window;
  unsigned int shrink_l_its = _linear_iteration_ratio * (_optimal_iterations + _iteration_window);

  if (_optimal_iterations > _iteration_window)
  {
    growth_nl_its = _optimal_iterations - _iteration_window;
    growth_l_its = _linear_iteration_ratio * (_optimal_iterations - _iteration_window);
  }

  if (can_grow && (_nonlin_iters < growth_nl_its && _lin_iters < growth_l_its))
    return _dt[0] * _growth_factor;
  else if (can_shrink && (_nonlin_iters > shrink_nl_its || _lin_iters > shrink_l_its))
    return _dt[0] * _cutback_factor;
  else
    return _dt[0];
}

Real
IterationAdaptiveStepper::computeFailedDT()
{
  return _cutback_factor * _dt[0];
}
