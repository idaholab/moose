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
#include "MooseObject.h"

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
   * @param allow_unsorted If false, an error is thrown if the inputted list of times is unsorted
   * @return The Times object with name associated with the parameter 'param'
   */
  const Times & getTimes(const std::string & param, bool allow_unsorted = false) const;

  /// Same as getTimes, except returning a pointer which is null if parameter is not valid
  const Times * getOptionalTimes(const std::string & params, bool allow_unsorted = false) const;

  /**
   * Get a function with a given name
   * @param name The name of the function to retrieve
   * @param allow_unsorted If false, an error is thrown if the inputted list of times is unsorted
   * @return The function with name 'name'
   */
  const Times & getTimesByName(const TimesName & name, bool allow_unsorted = false) const;

private:
  template <typename... Args>
  [[noreturn]] void timesError(Args &&... args) const
  {
    if (!this->_curr_param.empty())
      _tmi_moose_object.paramError(_curr_param, std::forward<Args>(args)...);
    else
      _tmi_moose_object.mooseError(std::forward<Args>(args)...);
  }

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
   * @param allow_unsorted If false, an error is thrown if the inputted list of times is unsorted
   * @return Whether or not the conversion was successful
   */
  bool getDefaultTimesObject(const std::string & input, bool allow_unsorted) const;

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

  /// Passed in parameter name to provide useful errors
  mutable std::string _curr_param = "";
};
