//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseObject.h"
#include "PerfGraphInterface.h"

class TimePeriodOld;
/**
 * Class that hold the whole problem being solved.
 */
class Problem : public MooseObject, public PerfGraphInterface
{
public:
  static InputParameters validParams();

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
  virtual bool isSolveTerminationRequested() const { return _termination_requested; };

  /**
   * Return console handle
   */
  const ConsoleStream & console() const { return _console; }

protected:
  /// True if the CLI option is found
  bool _cli_option_found;

  /// True if we're going to attempt to write color output
  bool _color_output;

  /// True if termination of the solve has been requested
  bool _termination_requested;
};
