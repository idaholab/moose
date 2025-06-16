//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ConsoleStreamInterface.h"
#include "StreamArguments.h"
#include "InputParameters.h"
#include "MooseError.h"
#include "MooseObjectName.h"
#include "MooseObjectParameterName.h"

class MooseApp;

namespace hit
{
class Node;
}

#define usingMooseBaseMembers                                                                      \
  using MooseBase::getMooseApp;                                                                    \
  using MooseBase::type;                                                                           \
  using MooseBase::name;                                                                           \
  using MooseBase::typeAndName;                                                                    \
  using MooseBase::uniqueName;                                                                     \
  using MooseBase::parameters;                                                                     \
  using MooseBase::isParamValid;                                                                   \
  using MooseBase::isParamSetByUser;                                                               \
  using MooseBase::paramError;                                                                     \
  using MooseBase::paramWarning;                                                                   \
  using MooseBase::paramInfo;                                                                      \
  using MooseBase::_app;                                                                           \
  using MooseBase::_type;                                                                          \
  using MooseBase::_name;                                                                          \
  using MooseBase::_pars

/**
 * Base class for everything in MOOSE with a name and a type.
 * You will most likely want to inherit instead
 * - MooseObject for an object created within a system
 * - Action for a class performing a setup task, like creating objects
 */
class MooseBase : public ConsoleStreamInterface
{
public:
  /// The name of the parameter that contains the object type
  static const std::string type_param;
  /// The name of the parameter that contains the object name
  static const std::string name_param;
  /// The name of the parameter that contains the unique object name
  static const std::string unique_name_param;
  /// The name of the parameter that contains the MooseApp
  static const std::string app_param;
  /// The name of the parameter that contains the moose system base
  static const std::string moose_base_param;

  static InputParameters validParams();

  /**
   * Primary constructor for general objects
   * @param params The parameters
   */
  MooseBase(const InputParameters & params);

  /**
   * Constructor to only be used by MooseApp.
   * @param app The app
   * @param params The app params
   */
  MooseBase(MooseApp & app, const InputParameters & params);

  virtual ~MooseBase() = default;

  /**
   * Get the MooseApp this class is associated with.
   */
  MooseApp & getMooseApp() const { return _app; }

  /**
   * Get the type of this class.
   * @return the name of the type of this class
   */
  const std::string & type() const { return _type; }

  /**
   * Get the name of the class
   * @return The name of the class
   */
  const std::string & name() const { return _name; }

  /**
   * Get the class's combined type and name; useful in error handling.
   * @return The type and name of this class in the form '<type()> "<name()>"'.
   */
  std::string typeAndName() const;

  /**
   * @returns The unique parameter name of a valid parameter of this
   * object for accessing parameter controls
   */
  MooseObjectParameterName uniqueParameterName(const std::string & parameter_name) const;

  /**
   * @returns The unique name for accessing input parameters of this object in
   * the InputParameterWarehouse
   */
  MooseObjectName uniqueName() const;

  /**
   * Get the parameters of the object
   * @return The parameters of the object
   */
  const InputParameters & parameters() const { return _pars; }

  /**
   * @returns The block-level hit node for this object, if any
   */
  const hit::Node * getHitNode() const { return getHitNode(_pars); }

  /**
   * @returns Whether or not this object has a registered base (set via
   * InputParameters::registerBase())
   */
  bool hasBase() const { return _pars.hasBase(); }

  /**
   * @returns The registered base for this object (set via InputParameters::registerBase())
   */
  const std::string & getBase() const { return _pars.getBase(); }

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
   * @param name The name of the parameter to test
   */
  inline bool isParamSetByUser(const std::string & name) const
  {
    return _pars.isParamSetByUser(name);
  }

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
   * @returns A prefix to be used in messages that contain the input
   * file location associated with this object (if any) and the
   * name and type of the object.
   */
  std::string messagePrefix(const bool hit_prefix = true) const
  {
    return messagePrefix(_pars, hit_prefix);
  }

  /**
   * Deprecated message prefix; the error type is no longer used
   */
  std::string errorPrefix(const std::string &) const { return messagePrefix(); }

  /**
   * Emits an error prefixed with object name and type and optionally a file path
   * to the top-level block parameter if available.
   */
  template <typename... Args>
  [[noreturn]] void mooseError(Args &&... args) const
  {
    callMooseError(argumentsToString(std::forward<Args>(args)...), /* with_prefix = */ true);
  }

  template <typename... Args>
  [[noreturn]] void mooseDocumentedError(const std::string & repo_name,
                                         const unsigned int issue_num,
                                         Args &&... args) const
  {
    callMooseError(moose::internal::formatMooseDocumentedError(
                       repo_name, issue_num, argumentsToString(std::forward<Args>(args)...)),
                   /* with_prefix = */ true);
  }

  /**
   * Emits an error without the prefixing included in mooseError().
   */
  template <typename... Args>
  [[noreturn]] void mooseErrorNonPrefixed(Args &&... args) const
  {
    callMooseError(argumentsToString(std::forward<Args>(args)...), /* with_prefix = */ false);
  }

  /**
   * Emits a warning prefixed with object name and type.
   */
  template <typename... Args>
  void mooseWarning(Args &&... args) const
  {
    moose::internal::mooseWarningStream(_console, messagePrefix(true), std::forward<Args>(args)...);
  }

  /**
   * Emits a warning without the prefixing included in mooseWarning().
   */
  template <typename... Args>
  void mooseWarningNonPrefixed(Args &&... args) const
  {
    moose::internal::mooseWarningStream(_console, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void mooseDeprecated(Args &&... args) const
  {
    moose::internal::mooseDeprecatedStream(
        _console, false, true, messagePrefix(true), std::forward<Args>(args)...);
  }

  template <typename... Args>
  void mooseInfo(Args &&... args) const
  {
    moose::internal::mooseInfoStream(_console, messagePrefix(true), std::forward<Args>(args)...);
  }

  /**
   * External method for calling moose error with added object context.
   * @param msg The message
   * @param with_prefix If true, add the prefix from messagePrefix(), which is the object
   * information (type, name, etc)
   * @param node Optional hit node to add file path context as a prefix
   */
  [[noreturn]] void
  callMooseError(std::string msg, const bool with_prefix, const hit::Node * node = nullptr) const;

  /**
   * External method for calling moose error with added object context.
   *
   * Needed so that objects without the MooseBase context (InputParameters)
   * can call errors with context
   *
   * @param app The app pointer (if available); adds multiapp context and clears the console
   * @param params The parameters, needed to obtain object information
   * @param msg The message
   * @param with_prefix If true, add the prefix from messagePrefix(), which is the object
   * information (type, name, etc)
   * @param node Optional hit node to add file path context as a prefix
   */
  [[noreturn]] static void callMooseError(MooseApp * const app,
                                          const InputParameters & params,
                                          std::string msg,
                                          const bool with_prefix,
                                          const hit::Node * node);

protected:
  /// The MOOSE application this is associated with
  MooseApp & _app;

  /// The type of this class
  const std::string & _type;

  /// The name of this class
  const std::string & _name;

  /// The object's parameters
  const InputParameters & _pars;

private:
  /**
   * Internal method for getting the message prefix for an object (object type, name, etc).
   *
   * Needs to be static so that we can call it externally from InputParameters for
   * errors that do not have context of the MooseBase
   */
  static std::string messagePrefix(const InputParameters & params, const bool hit_prefix);

  /**
   * Internal method for getting a hit node (if available) given a set of parameters
   *
   * Needs to be static so that we can call it externally from InputParameters for
   * errors that do not have context of the MooseBase
   */
  static const hit::Node * getHitNode(const InputParameters & params);
};

template <typename T>
const T &
MooseBase::getParam(const std::string & name) const
{
  return InputParameters::getParamHelper<T>(name, _pars);
}

template <typename T>
const T *
MooseBase::queryParam(const std::string & name) const
{
  return isParamValid(name) ? &getParam<T>(name) : nullptr;
}

template <typename T>
const T &
MooseBase::getRenamedParam(const std::string & old_name, const std::string & new_name) const
{
  // Most important: accept new parameter
  if (isParamSetByUser(new_name) && !isParamValid(old_name))
    return getParam<T>(new_name);
  // Second most: accept old parameter
  if (isParamValid(old_name) && !isParamSetByUser(new_name))
    return getParam<T>(old_name);
  // Third most: accept default for new parameter
  if (isParamValid(new_name) && !isParamValid(old_name))
    return getParam<T>(new_name);
  // Refuse: no default, no value passed
  if (!isParamValid(old_name) && !isParamValid(new_name))
    mooseError("parameter '" + new_name +
               "' is being retrieved without being set.\nDid you misspell it?");
  // Refuse: both old and new parameters set by user
  else
    mooseError("Parameter '" + new_name + "' may not be provided alongside former parameter '" +
               old_name + "'");
}

template <typename T1, typename T2>
std::vector<std::pair<T1, T2>>
MooseBase::getParam(const std::string & param1, const std::string & param2) const
{
  return _pars.get<T1, T2>(param1, param2);
}

template <typename T>
T
MooseBase::getCheckedPointerParam(const std::string & name, const std::string & error_string) const
{
  return _pars.getCheckedPointerParam<T>(name, error_string);
}

template <typename... Args>
[[noreturn]] void
MooseBase::paramError(const std::string & param, Args... args) const
{
  _pars.paramError(param, std::forward<Args>(args)...);
}

template <typename... Args>
void
MooseBase::paramWarning(const std::string & param, Args... args) const
{
  mooseWarning(_pars.paramMessage(param, std::forward<Args>(args)...));
}

template <typename... Args>
void
MooseBase::paramInfo(const std::string & param, Args... args) const
{
  mooseInfo(_pars.paramMessage(param, std::forward<Args>(args)...));
}
