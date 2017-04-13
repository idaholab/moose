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

#ifndef MOOSEOBJECT_H
#define MOOSEOBJECT_H

// MOOSE includes
#include "InputParameters.h"
#include "ConsoleStreamInterface.h"

// libMesh includes
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
   * Test if the supplied parameter is valid
   * @param name The name of the parameter to test
   */
  inline bool isParamValid(const std::string & name) const { return _pars.isParamValid(name); }

  /**
   * Get the MooseApp this object is associated with.
   */
  MooseApp & getMooseApp() { return _app; }

  /**
   * Return the enabled status of the object.
   */
  virtual bool enabled() { return _enabled; }

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
  /// The MooseApp this object is associated with
  MooseApp & _app;

  /// Parameters of this object, references the InputParameters stored in the InputParametersWarehouse
  const InputParameters & _pars;

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
