//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InputParameters.h"
#include "Times.h"

// Forward declarations
class Times;
class FEProblemBase;
class MooseObject;

/**
 * Interface for objects that need to use Times
 */
class TimesInterface
{
public:
  static InputParameters validParams();
  TimesInterface(const MooseObject * moose_object);

  /**
   * Get a Times object with a given name
   * @param name The name of the parameter key of the Times to retrieve
   * @return The Times object with name associated with the parameter 'param'
   */
  const Times & getTimes(const std::string & param) const;

  /// Same as getTimes, except returning a pointer which is null if parameter is not valid
  const Times * getOptionalTimes(const std::string & params) const;

  /**
   * Get a function with a given name
   * @param name The name of the function to retrieve
   * @return The function with name 'name'
   */
  const Times & getTimesByName(const TimesName & name) const;

private:
  class DefaultTimes : public Times
  {
  public:
    DefaultTimes(const std::vector<Real> & default_times, const FEProblemBase & fe_problem);

  private:
    std::vector<Real> _default_times;
  };

  /**
   * Tries to convert the input string into a vector of reals and fills in _default_times_objs
   *
   * @param input String to convert to a vector of reals
   * @return Whether or not the conversion was successful
   */
  bool getDefaultTimesObject(const std::string & input) const;

  /// MooseObject this interface is interacting with
  const MooseObject & _tmi_moose_object;

  /// Parameters of the object with this interface
  const InputParameters & _tmi_params;

  /// Reference to FEProblemBase instance
  FEProblemBase & _tmi_feproblem;

  /// Thread ID
  const THREAD_ID _tmi_tid;

  /// Default times objects constructed by this object
  mutable std::deque<DefaultTimes> _default_times_objs;
};
