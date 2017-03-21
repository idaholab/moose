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

template <>
InputParameters validParams<SolutionTimeAdaptiveDT>();

/**
 *
 */
class SolutionTimeAdaptiveDT : public TimeStepper
{
public:
  SolutionTimeAdaptiveDT(const InputParameters & parameters);
  virtual ~SolutionTimeAdaptiveDT();

  virtual void step() override;

  virtual void rejectStep() override;

protected:
  virtual Real computeInitialDT() override;
  virtual Real computeDT() override;

  /**
   * Multiplier specifying the direction the timestep is currently going.
   * Positive for up.  Negative for down.
   */
  short _direction;

  /// Percentage to change the timestep by either way.
  Real _percent_change;

  /// Ratios to control whether to increase or decrease the current timestep
  Real _older_sol_time_vs_dt, _old_sol_time_vs_dt, _sol_time_vs_dt;

  /// Boolean to control whether a separate adapt log is written to a file
  bool _adapt_log;

  /// The filehandle to hold the log
  std::ofstream _adaptive_log;
};

#endif /* SOLUTIONTIMEADAPTIVEDT_H */
