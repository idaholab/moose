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

#ifndef SOLUTIONTIMEADAPTIVESTEPPER_H
#define SOLUTIONTIMEADAPTIVESTEPPER_H

#include "Stepper.h"

class SolutionTimeAdaptiveStepper;

template<>
InputParameters validParams<SolutionTimeAdaptiveStepper>();

/**
 * Essentially takes the place of what used to be ConstantDT
 * The name was so terribly bad for that old one that we gotta get rid of it.
 * We might cross-register this object with ConstantDT once it's deployed though
 * and register it as a deprecated name.
 *
 * This one cuts the timestep by half when a solve fails and regrows by
 * a growth_factor when there is convergence.
 */
class SolutionTimeAdaptiveStepper : public Stepper
{
public:
  SolutionTimeAdaptiveStepper(const InputParameters & parameters);
  virtual ~SolutionTimeAdaptiveStepper();

  virtual Real computeInitialDT() override;

  virtual Real computeDT() override;

  virtual Real computeFailedDT() override;

protected:
  /// Multiplier specifying the direction the timestep is currently going.
  int _direction;

  /// Percentage to change the timestep by either way.
  const Real & _percent_change;

  /// Initial dt
  const Real & _input_dt;

  /// Number of steps in the same direction
  int _n_steps;
};

#endif /* SOLUTIONTIMEADAPTIVESTEPPER_H */
