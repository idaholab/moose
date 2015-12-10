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

#ifndef PROBLEM_H
#define PROBLEM_H

#include "MooseObject.h"

class TimePeriodOld;
class Problem;

template<>
InputParameters validParams<Problem>();

/**
 * Class that hold the whole problem being solved.
 */
class Problem : public MooseObject
{
public:
  Problem(const InputParameters & parameters);
  virtual ~Problem();

  virtual void init() = 0;

  /**
   * For Internal Use
   */
  void _setCLIOption() { _cli_option_found = true; }

  // Time periods //////

  /**
   * Add a time period
   * @param name Name of the time period
   * @param start_time Start time of the time period
   */
  TimePeriodOld & addTimePeriod(const std::string & name, Real start_time);

  /**
   * Get time period by name
   * @param name Name of the time period to get
   * @return Pointer to the time period struct if found, otherwise NULL
   */
  virtual TimePeriodOld * getTimePeriodByName(const std::string & name);

  const std::vector<TimePeriodOld *> & getTimePeriods() const;

  /**
   * Allow objects to request clean termination of the solve
   */
  virtual void terminateSolve() { _termination_requested = true; };

  /**
   * Check of termination has been requested. This should be called by
   * transient Executioners in the keepGoing() member.
   */
  virtual bool isSolveTerminationRequested() { return _termination_requested; };

protected:
  /// Time periods
  std::vector<TimePeriodOld *> _time_periods;

  /// True if the CLI option is found
  bool _cli_option_found;

  /// True if we're going to attempt to write color output
  bool _color_output;

  /// True if termination of the solve has been requested
  bool _termination_requested;
};

#endif /* PROBLEM_H */
