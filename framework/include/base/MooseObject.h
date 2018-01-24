//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MOOSEOBJECT_H
#define MOOSEOBJECT_H

// MOOSE includes
#include "InputParameters.h"
#include "ConsoleStreamInterface.h"

#include "libmesh/parallel_object.h"

class MooseApp;
class MooseObject;

template <>
InputParameters validParams<MooseObject>();

// needed to avoid #include cycle with MooseApp and MooseObject
[[noreturn]] void callMooseErrorRaw(std::string & msg, MooseApp * app);

/**
 * Every object that can be built by the factory should be derived from this class.
 */
class MooseObject : public ConsoleStreamInterface, public libMesh::ParallelObject
{
public:
  MooseObject(const InputParameters & parameters);

  virtual ~MooseObject() = default;

  /**
   * Get the name of the object
   * @return The name of the object
   */
  const std::string & name() const { return _name; }

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
  [[noreturn]] void paramError(const std::string & param, Args... args)
  {
    auto prefix = param + ": ";
    if (!_pars.inputLocation(param).empty())
      prefix = _pars.inputLocation(param) + ": (" + _pars.paramFullpath(param) + ") ";
    mooseError(prefix, args...);
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
    moose::internal::mooseDeprecatedStream(_console, std::forward<Args>(args)...);
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

  /// The name of this object, reference to value stored in InputParameters
  const std::string & _name;

  /// Reference to the "enable" InputParaemters, used by Controls for toggling on/off MooseObjects
  const bool & _enabled;
};

template <typename T>
const T &
MooseObject::getParam(const std::string & name) const
{
  return InputParameters::getParamHelper(name, _pars, static_cast<T *>(0));
}

#endif /* MOOSEOBJECT_H*/
