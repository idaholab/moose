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
#include "MemberTemplateMacros.h"

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
   * TODO:MooseVariableToMooseObject (see #10601)
   */
  virtual const std::string & name() const { return _name; }

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
  const T & getParamTempl(const std::string & name) const;

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
  [[noreturn]] void paramError(const std::string & param, Args... args) const
  {
    auto prefix = param + ": ";
    if (!_pars.inputLocation(param).empty())
      prefix = _pars.inputLocation(param) + ": (" + _pars.paramFullpath(param) + "):\n";
    mooseError(prefix, args...);
  }

  /**
   * Emits a warning prefixed with the file and line number of the given param (from the input
   * file) along with the full parameter path+name followed by the given args as the message.
   * If this object's parameters were not created directly by the Parser, then this function falls
   * back to the normal behavior of mooseWarning - only printing a message using the given args.
   */
  template <typename... Args>
  void paramWarning(const std::string & param, Args... args) const
  {
    auto prefix = param + ": ";
    if (!_pars.inputLocation(param).empty())
      prefix = _pars.inputLocation(param) + ": (" + _pars.paramFullpath(param) + "):\n";
    mooseWarning(prefix, args...);
  }

  /**
   * Emits an informational message prefixed with the file and line number of the given param
   * (from the input file) along with the full parameter path+name followed by the given args as
   * the message.  If this object's parameters were not created directly by the Parser, then this
   * function falls back to the normal behavior of mooseInfo - only printing a message using
   * the given args.
   */
  template <typename... Args>
  void paramInfo(const std::string & param, Args... args) const
  {
    auto prefix = param + ": ";
    if (!_pars.inputLocation(param).empty())
      prefix = _pars.inputLocation(param) + ": (" + _pars.paramFullpath(param) + "):\n";
    mooseInfo(prefix, args...);
  }

  template <typename... Args>
  [[noreturn]] void mooseError(Args &&... args) const
  {
    std::ostringstream oss;
    moose::internal::mooseStreamAll(oss, std::forward<Args>(args)...);
    std::string msg = oss.str();
    callMooseErrorRaw(msg, &_app);
  }

  template <typename... Args>
  void mooseWarning(Args &&... args) const
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
};

template <typename T>
const T &
MooseObject::getParamTempl(const std::string & name) const
{
  return InputParameters::getParamHelper(name, _pars, static_cast<T *>(0));
}
