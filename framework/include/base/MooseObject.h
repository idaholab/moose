//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "InputParameters.h"
#include "ConsoleStreamInterface.h"
#include "Registry.h"
#include "MooseUtils.h"

#include "libmesh/parallel_object.h"

#define usingMooseObjectMembers                                                                    \
  using MooseObject::isParamValid;                                                                 \
  using MooseObject::paramError

class MooseApp;
class MooseObject;

template <>
InputParameters validParams<MooseObject>();

// needed to avoid #include cycle with MooseApp and MooseObject
[[noreturn]] void callMooseErrorRaw(std::string & msg, MooseApp * app);

/**
 * Generates a canonical paramError prefix for param-related error/warning/info messages.
 *
 * Use this for building custom messages when the default paramError isn't
 * quite what you need.
 */
std::string paramErrorPrefix(const InputParameters & params, const std::string & param);

// helper macro to explicitly instantiate AD classes
#define adBaseClass(X)                                                                             \
  template class X<RESIDUAL>;                                                                      \
  template class X<JACOBIAN>

/**
 * Every object that can be built by the factory should be derived from this class.
 */
class MooseObject : public ConsoleStreamInterface, public libMesh::ParallelObject
{
public:
  static InputParameters validParams();

  MooseObject(const InputParameters & parameters);

  virtual ~MooseObject() = default;

  /**
   * Get the type of this object.
   * @return the name of the type of this object
   */
  const std::string & type() const { return _type; }

  /**
   * Get the name of the object
   * @return The name of the object
   */
  virtual const std::string & name() const { return _name; }

  /**
   * Get the object's combined type and name; useful in error handling.
   * @return The type and name of this object in the form '<type()> "<name()>"'.
   */
  std::string typeAndName() const;

  /**
   * Get the parameters of the object
   * @return The parameters of the object
   */
  const InputParameters & parameters() const { return _pars; }

  /**
   * Retrieve a parameter for the object
   * @param name The name of the parameter
   * @return The value of the parameter
   */
  template <typename T>
  const T & getParam(const std::string & name) const;

  /**
   * Verifies that the requested parameter exists and is not NULL and returns it to the caller.
   * The template parameter must be a pointer or an error will be thrown.
   */
  template <typename T>
  T getCheckedPointerParam(const std::string & name, const std::string & error_string = "") const
  {
    return parameters().getCheckedPointerParam<T>(name, error_string);
  }

  /**
   * Test if the supplied parameter is valid
   * @param name The name of the parameter to test
   */
  inline bool isParamValid(const std::string & name) const { return _pars.isParamValid(name); }

  /**
   * Get the MooseApp this object is associated with.
   */
  MooseApp & getMooseApp() const { return _app; }

  /**
   * Return the enabled status of the object.
   */
  virtual bool enabled() const { return _enabled; }

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
   * Emits an error prefixed with object name and type.
   */
  template <typename... Args>
  [[noreturn]] void mooseError(Args &&... args) const
  {
    std::ostringstream oss;
    moose::internal::mooseStreamAll(oss, errorPrefix("error"), std::forward<Args>(args)...);
    std::string msg = oss.str();
    callMooseErrorRaw(msg, &_app);
  }

  /**
   * Emits an error without the prefixing included in mooseError().
   */
  template <typename... Args>
  [[noreturn]] void mooseErrorNonPrefixed(Args &&... args) const
  {
    std::ostringstream oss;
    moose::internal::mooseStreamAll(oss, std::forward<Args>(args)...);
    std::string msg = oss.str();
    callMooseErrorRaw(msg, &_app);
  }

  /**
   * Emits a warning prefixed with object name and type.
   */
  template <typename... Args>
  void mooseWarning(Args &&... args) const
  {
    moose::internal::mooseWarningStream(
        _console, errorPrefix("warning"), std::forward<Args>(args)...);
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
    moose::internal::mooseDeprecatedStream(_console, false, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void mooseInfo(Args &&... args) const
  {
    moose::internal::mooseInfoStream(_console, std::forward<Args>(args)...);
  }

  /**
   * A descriptive prefix for errors for this object:
   *
   * The following <error_type> occurred in the object "<name>", of type "<type>".
   */
  std::string errorPrefix(const std::string & error_type) const;

protected:
  /// Parameters of this object, references the InputParameters stored in the InputParametersWarehouse
  const InputParameters & _pars;

  /// The MooseApp this object is associated with
  MooseApp & _app;

  /// The type of this object (the Class name)
  const std::string & _type;

  /// The name of this object, reference to value stored in InputParameters
  const std::string & _name;

  /// Reference to the "enable" InputParaemters, used by Controls for toggling on/off MooseObjects
  const bool & _enabled;

private:
  template <typename... Args>
  std::string paramErrorMsg(const std::string & param, Args... args) const
  {
    auto param_prefix = paramErrorPrefix(_pars, param);

    // With no input location information, append object info (name + type)
    const std::string object_prefix =
        _pars.inputLocation(param).empty() ? errorPrefix("parameter error") : "";

    std::ostringstream oss;
    moose::internal::mooseStreamAll(oss, std::forward<Args>(args)...);
    std::string msg = oss.str();

    // Wrap error message to a separate line from param_prefix if it is about to
    // blow past 100 chars.  But only wrap if the param_prefix is long enough (12
    // chars) for the wrap to buy us much extra length. We don't check object_prefix
    // here because it will go before the parameter error
    if ((param_prefix.size() > 12 && msg.size() + param_prefix.size() > 99) ||
        msg.find("\n") != std::string::npos)
    {
      if (param_prefix.size() > 0 && param_prefix[param_prefix.size() - 1] != ':')
        param_prefix += ":";
      return object_prefix + param_prefix + "\n    " + MooseUtils::replaceAll(msg, "\n", "\n    ");
    }
    return object_prefix + param_prefix + " " + msg;
  }
};

template <typename T>
const T &
MooseObject::getParam(const std::string & name) const
{
  return InputParameters::getParamHelper(name, _pars, static_cast<T *>(0));
}

template <typename... Args>
[[noreturn]] void
MooseObject::paramError(const std::string & param, Args... args) const
{
  Moose::show_trace = false;
  std::string msg = paramErrorMsg(param, std::forward<Args>(args)...);
  callMooseErrorRaw(msg, &_app);
  Moose::show_trace = true;
}

template <typename... Args>
void
MooseObject::paramWarning(const std::string & param, Args... args) const
{
  mooseWarning(paramErrorMsg(param, std::forward<Args>(args)...));
}

template <typename... Args>
void
MooseObject::paramInfo(const std::string & param, Args... args) const
{
  mooseInfo(paramErrorMsg(param, std::forward<Args>(args)...));
}
