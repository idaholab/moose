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
#include "MooseMesh.h"
#include "MooseArray.h"
#include "XTermConstants.h"

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
class Output;
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
   * Get the name of this problem
   * @return The name of this problem
   */
  virtual const std::string & name();

  /**
   * Get reference to all-purpose parameters
   */
  InputParameters & parameters() { return _pars; }

  virtual void init() = 0;

  // Output system /////
  virtual void output(bool force = false) = 0;

  /**
   * Whether or not to color output to the terminal.
   * @param state Pass true for color, false to not color.
   */
  void setColorOutput(bool state) { _color_output = state; }

  /**
   * Return the current status of whether or not to color terminal output.
   */
  bool shouldColorOutput() const { return _color_output; }

  /**
   * Returns a character string to produce a specific color in terminals supporting
   * color. The list of color constants is available in XTermConstants.h
   * @param color (from XTermConstants.h)
   */
  template <typename T>
  std::string colorText(const std::string color, T text) const;

  /**
   * For Internal Use
   */
  void _setCLIOption() { _cli_option_found = true; }

  // Time periods //////

  /**
   * Add a time period
   * @param name Name of the time period
   * @param start_time Start time of the time period
   * @param end_time End time of the time period
   */
  TimePeriod & addTimePeriod(const std::string & name, Real start_time);

  /**
   * Get time period by name
   * @param name Name of the time period to get
   * @return Pointer to the time period struct if found, otherwise NULL
   */
  virtual TimePeriod * getTimePeriodByName(const std::string & name);

  const std::vector<TimePeriod *> & getTimePeriods() const;

protected:
  /// Time periods
  std::vector<TimePeriod *> _time_periods;

  /// True if the CLI option is found
  bool _cli_option_found;

  /// True if we're going to attempt to write color output
  bool _color_output;
};

template <typename T>
std::string
Problem::colorText(std::string color, T text) const
{
  std::ostringstream oss;
  oss << std::scientific;

  if (_color_output)
  {
    if (_cli_option_found && color.length() == 5 && color[3] == '2')
      color.replace(3, 1, "5", 1);

    oss << color << text << DEFAULT;
  }
  else
    oss << text;

  return oss.str();
}


#endif /* PROBLEM_H */
