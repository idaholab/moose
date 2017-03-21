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

template <>
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
  /// True if the CLI option is found
  bool _cli_option_found;

  /// True if we're going to attempt to write color output
  bool _color_output;

  /// True if termination of the solve has been requested
  bool _termination_requested;
};

#endif /* PROBLEM_H */
