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

#include "ParallelUniqueId.h"
#include "InputParameters.h"
#include "ExecStore.h"
#include "MooseArray.h"
#include "MooseObject.h"
#include "MooseUtils.h"
#include "Output.h"

// libMesh
#include "libmesh/libmesh_common.h"
#include "libmesh/equation_systems.h"
#include "libmesh/quadrature.h"
#include "libmesh/elem.h"
#include "libmesh/node.h"
#include "libmesh/nonlinear_implicit_system.h"

#include <string>

class MooseVariable;
class MooseVariableScalar;
class Material;
class Function;
class TimePeriod;
class Problem;

template<>
InputParameters validParams<Problem>();

/**
 * Class that hold the whole problem being solved.
 */
class Problem : public MooseObject
{
public:
  Problem(const std::string & name, InputParameters parameters);
  virtual ~Problem();

  /**
   * Get reference to all-purpose parameters
   */
  InputParameters & parameters() { return _pars; }

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
  TimePeriod & addTimePeriod(const std::string & name, Real start_time);

  /**
   * Get time period by name
   * @param name Name of the time period to get
   * @return Pointer to the time period struct if found, otherwise NULL
   */
  virtual TimePeriod * getTimePeriodByName(const std::string & name);

  const std::vector<TimePeriod *> & getTimePeriods() const;

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
  std::vector<TimePeriod *> _time_periods;

  /// True if the CLI option is found
  bool _cli_option_found;

  /// True if we're going to attempt to write color output
  bool _color_output;

  /// True if termination of the solve has been requested
  bool _termination_requested;
};

#endif /* PROBLEM_H */
