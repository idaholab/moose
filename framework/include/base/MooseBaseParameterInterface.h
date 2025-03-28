//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseBaseErrorInterface.h"

#include "MooseBase.h"
#include "InputParameters.h"
#include "MooseUtils.h"
#include "MooseObjectParameterName.h"

#include <string>

#define usingMooseBaseParameterInterfaceMembers                                                    \
  using MooseBaseParameterInterface::parameters;                                                   \
  using MooseBaseParameterInterface::uniqueName;                                                   \
  using MooseBaseParameterInterface::isParamValid;                                                 \
  using MooseBaseParameterInterface::isParamSetByUser;                                             \
  using MooseBaseParameterInterface::paramError;                                                   \
  using MooseBaseParameterInterface::_pars;                                                        \
  using MooseBaseParameterInterface::_factory;                                                     \
  using MooseBaseParameterInterface::_action_factory

/**
 * Every object that can be built by the factory should be derived from this class.
 */
class MooseBaseParameterInterface
{
public:
  MooseBaseParameterInterface(const MooseBase & base, const InputParameters & parameters);

  virtual ~MooseBaseParameterInterface() = default;

  /**
   * The unique parameter name of a valid parameter of this object for accessing parameter controls
   */
  MooseObjectParameterName uniqueParameterName(const std::string & parameter_name) const
  {
    return MooseObjectParameterName(
        _pars.get<std::string>("_moose_base"), _moose_base.name(), parameter_name);
  }

  /**
   * Get the parameters of the object
   * @return The parameters of the object
   */
  const InputParameters & parameters() const { return _pars; }

  /**
   * The unique name for accessing input parameters of this object in the InputParameterWarehouse
   */
  MooseObjectName uniqueName() const
  {
    return MooseObjectName(_pars.get<std::string>("_unique_name"));
  }

  /**
   * Retrieve a parameter for the object
   * @param name The name of the parameter
   * @return The value of the parameter
   */
  template <typename T>
  const T & getParam(const std::string & name) const;

  /**
   * Query a parameter for the object
   *
   * If the parameter is not valid, nullptr will be returned
   *
   * @param name The name of the parameter
   * @return A pointer to the parameter value, if it exists
   */
  template <typename T>
  const T * queryParam(const std::string & name) const;

  /**
   * Retrieve a renamed parameter for the object. This helper makes sure we
   * check both names before erroring, and that only one parameter is passed to avoid
   * silent errors
   * @param old_name the old name for the parameter
   * @param new_name the new name for the parameter
   */
  template <typename T>
  const T & getRenamedParam(const std::string & old_name, const std::string & new_name) const;

  /**
   * Retrieve two parameters and provide pair of parameters for the object
   * @param param1 The name of first parameter
   * @param param2 The name of second parameter
   * @return Vector of pairs of first and second parameters
   */
  template <typename T1, typename T2>
  std::vector<std::pair<T1, T2>> getParam(const std::string & param1,
                                          const std::string & param2) const;

  /**
   * Verifies that the requested parameter exists and is not NULL and returns it to the caller.
   * The template parameter must be a pointer or an error will be thrown.
   */
  template <typename T>
  T getCheckedPointerParam(const std::string & name, const std::string & error_string = "") const;

  /**
   * Test if the supplied parameter is valid
   * @param name The name of the parameter to test
   */
  inline bool isParamValid(const std::string & name) const { return _pars.isParamValid(name); }

  /**
   * Test if the supplied parameter is set by a user, as opposed to not set or set to default
   * @param nm The name of the parameter to test
   */
  inline bool isParamSetByUser(const std::string & nm) const { return _pars.isParamSetByUser(nm); }

  /**
   * Emits an error prefixed with the file and line number of the given param (from the input
   * file) along with the full parameter path+name followed by the given args as the message.
   * If this object's parameters were not created directly by the Parser, then this function falls
   * back to the normal behavior of mooseError - only printing a message using the given args.
   */
  template <typename... Args>
  [[noreturn]] void paramError(const std::string & param, Args... args) const;

  /**
   * Emits a warning prefixed with the file and line number of the given param (from the input
   * file) along with the full parameter path+name followed by the given args as the message.
   * If this object's parameters were not created directly by the Parser, then this function falls
   * back to the normal behavior of mooseWarning - only printing a message using the given args.
   */
  template <typename... Args>
  void paramWarning(const std::string & param, Args... args) const;

  /**
   * Emits an informational message prefixed with the file and line number of the given param
   * (from the input file) along with the full parameter path+name followed by the given args as
   * the message.  If this object's parameters were not created directly by the Parser, then this
   * function falls back to the normal behavior of mooseInfo - only printing a message using
   * the given args.
   */
  template <typename... Args>
  void paramInfo(const std::string & param, Args... args) const;

  /**
   * Connect controllable parameter of this action with the controllable parameters of the
   * objects added by this action.
   * @param parameter Name of the controllable parameter of this action
   * @param object_type Type of the object added by this action.
   * @param object_name Name of the object added by this action.
   * @param object_parameter Name of the parameter of the object.
   */
  void connectControllableParams(const std::string & parameter,
                                 const std::string & object_type,
                                 const std::string & object_name,
                                 const std::string & object_parameter) const;

protected:
  /// Parameters of this object, references the InputParameters stored in the InputParametersWarehouse
  const InputParameters & _pars;

  /// The Factory associated with the MooseApp
  Factory & _factory;

  /// Builds Actions
  ActionFactory & _action_factory;

private:
  /// The MooseBase object that inherits this class
  const MooseBase & _moose_base;
};

template <typename T>
const T &
MooseBaseParameterInterface::getParam(const std::string & name) const
{
  return InputParameters::getParamHelper(name, _pars, static_cast<T *>(0));
}

template <typename T>
const T *
MooseBaseParameterInterface::queryParam(const std::string & name) const
{
  return isParamValid(name) ? &getParam<T>(name) : nullptr;
}

template <typename T>
const T &
MooseBaseParameterInterface::getRenamedParam(const std::string & old_name,
                                             const std::string & new_name) const
{
  // this enables having a default on the new parameter but bypassing it with the old one
  // Most important: accept new parameter
  if (isParamSetByUser(new_name) && !isParamValid(old_name))
    return InputParameters::getParamHelper(new_name, _pars, static_cast<T *>(0));
  // Second most: accept old parameter
  else if (isParamValid(old_name) && !isParamSetByUser(new_name))
    return InputParameters::getParamHelper(old_name, _pars, static_cast<T *>(0));
  // Third most: accept default for new parameter
  else if (isParamValid(new_name) && !isParamValid(old_name))
    return InputParameters::getParamHelper(new_name, _pars, static_cast<T *>(0));
  // Refuse: no default, no value passed
  else if (!isParamValid(old_name) && !isParamValid(new_name))
    mooseError(_pars.blockFullpath() + ": parameter '" + new_name +
               "' is being retrieved without being set.\n"
               "Did you misspell it?");
  // Refuse: both old and new parameters set by user
  else
    mooseError(_pars.blockFullpath() + ": parameter '" + new_name +
               "' may not be provided alongside former parameter '" + old_name + "'");
}

template <typename... Args>
[[noreturn]] void
MooseBaseParameterInterface::paramError(const std::string & param, Args... args) const
{
  _pars.paramError(param, std::forward<Args>(args)...);
}

template <typename... Args>
void
MooseBaseParameterInterface::paramWarning(const std::string & param, Args... args) const
{
  mooseWarning(_pars.paramMessage(param, std::forward<Args>(args)...));
}

template <typename... Args>
void
MooseBaseParameterInterface::paramInfo(const std::string & param, Args... args) const
{
  mooseInfo(_pars.paramMessage(param, std::forward<Args>(args)...));
}

template <typename T1, typename T2>
std::vector<std::pair<T1, T2>>
MooseBaseParameterInterface::getParam(const std::string & param1, const std::string & param2) const
{
  return _pars.get<T1, T2>(param1, param2);
}

template <typename T>
T
MooseBaseParameterInterface::getCheckedPointerParam(const std::string & name,
                                                    const std::string & error_string) const
{
  return parameters().getCheckedPointerParam<T>(name, error_string);
}
