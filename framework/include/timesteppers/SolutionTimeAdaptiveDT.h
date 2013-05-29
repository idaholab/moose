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

class SolutionTimeAdaptiveDT;

template<>
InputParameters validParams<SolutionTimeAdaptiveDT>();

/**
 *
 */
class SolutionTimeAdaptiveDT : public TimeStepper
{
public:
  SolutionTimeAdaptiveDT(const std::string & name, InputParameters parameters);
  virtual ~SolutionTimeAdaptiveDT();

  virtual void preSolve();
  virtual void postSolve();

  virtual void computeInitialDT();
  virtual void computeDT();

  virtual void rejectStep();

protected:
  /**
   * Multiplier specifying the direction the timestep is currently going.
   * Positive for up.  Negative for down.
   */
  int _direction;

  /// Percentage to change the timestep by either way.
  Real _percent_change;

  timeval _solve_start, _solve_end;

  Real _older_sol_time_vs_dt, _old_sol_time_vs_dt, _sol_time_vs_dt;

  bool _adapt_log;

  std::ofstream _adaptive_log;

};

#endif /* SOLUTIONTIMEADAPTIVEDT_H */
