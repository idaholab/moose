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

#ifndef SOLUTIONTIMEADAPTIVEDT_H
#define SOLUTIONTIMEADAPTIVEDT_H

#include "TimeStepper.h"

#include <fstream>

class SolutionTimeAdaptiveDT;

template<>
InputParameters validParams<SolutionTimeAdaptiveDT>();

/**
 *
 */
class SolutionTimeAdaptiveDT : public TimeStepper
{
public:
  SolutionTimeAdaptiveDT(const InputParameters & parameters);

  virtual Stepper* buildStepper() override;

private:

  /**
   * Multiplier specifying the direction the timestep is currently going.
   * Positive for up.  Negative for down.
   */
  int _direction;

  /// Percentage to change the timestep by either way.
  Real _percent_change;
};

#endif /* SOLUTIONTIMEADAPTIVEDT_H */
