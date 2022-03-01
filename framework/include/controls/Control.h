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
#include "MooseObject.h"
#include "ControllableParameter.h"
#include "TransientInterface.h"
#include "SetupInterface.h"
#include "FunctionInterface.h"
#include "UserObjectInterface.h"
#include "PostprocessorInterface.h"
#include "VectorPostprocessorInterface.h"

class FEProblemBase;
class InputParameterWarehouse;

/**
 * Base class for Control objects.
 *
 * These objects are create by the [Controls] block in the input file after
 * all other MooseObjects are created, so they have access to parameters
 * in all other MooseObjects.
 */
class Control : public MooseObject,
                public TransientInterface,
                public SetupInterface,
                public FunctionInterface,
                public UserObjectInterface,
                public Restartable,
                protected PostprocessorInterface,
                protected VectorPostprocessorInterface
{
public:
  /**
   * Class constructor
   * @param parameters The input parameters for this control object
   */
  static InputParameters validParams();

  Control(const InputParameters & parameters);

  /**
   * Class destructor
   */
  virtual ~Control() {}

  /**
   * Execute the control. This must be overridden.
   */
  virtual void execute() = 0;

  /**
   * (DEPRECATED) Return the valid "execute_on" options for Control objects
   */
  static MultiMooseEnum getExecuteOptions();

  /**
   * Return the Controls that must run before this Control
   */
  std::vector<std::string> & getDependencies() { return _depends_on; }

protected:
  /// Reference to the FEProblemBase for this object
  FEProblemBase & _fe_problem;

  /// A list of controls that are required to run before this control may run
  std::vector<std::string> _depends_on;

  ///@{
  /**
   * Direct access to the ControllableParameter object.
   */
  ControllableParameter getControllableParameter(const std::string & param_name);
  ControllableParameter getControllableParameterByName(const std::string & param_name);
  ControllableParameter getControllableParameterByName(const std::string & tag,
                                                       const std::string & object_name,
                                                       const std::string & param_name);
  ControllableParameter getControllableParameterByName(const MooseObjectName & object_name,
                                                       const std::string & param_name);
  ControllableParameter getControllableParameterByName(const MooseObjectParameterName & param_name);
  ///@}

  ///@{
  /**
   * Obtain the value of a controllable parameter given input file syntax or actual name.
   * @param name The name of the parameter to retrieve a value.
   * @param warn_when_values_diff When true, produce a warning if multiple controllable values share
   * the given name but have varying values.
   *
   * @return A copy of the first parameter that matches the given name.
   */
  template <typename T>
  T getControllableValue(const std::string & name, bool warn_when_values_differ = true);

  template <typename T>
  T getControllableValueByName(const std::string & name, bool warn_when_values_differ = true);

  template <typename T>
  T getControllableValueByName(const std::string & object_name,
                               const std::string & param_name,
                               bool warn_when_values_differ = true);

  template <typename T>
  T getControllableValueByName(const MooseObjectName & object_name,
                               const std::string & param_name,
                               bool warn_when_values_differ = true);

  template <typename T>
  T getControllableValueByName(const std::string & tag,
                               const std::string & object_name,
                               const std::string & param_name,
                               bool warn_when_values_differ = true);

  template <typename T>
  T getControllableValueByName(const MooseObjectParameterName & desired,
                               bool warn_when_values_differ = true);
  ///@}

  ///@{
  /**
   * Set the value(s) of a controllable parameter of class given input file syntax or actual name.
   * @param name The name of the parameter to retrieve a value.
   * @param value The value to set all matching parameters to.
   * @param warn_when_values_diff When true, produce a warning if multiple controllable values share
   *                              the given name but have varying values.
   */
  template <typename T>
  void setControllableValue(const std::string & name, const T & value);

  template <typename T>
  void setControllableValueByName(const std::string & name, const T & value);

  template <typename T>
  void setControllableValueByName(const std::string & object_name,
                                  const std::string & param_name,
                                  const T & value);

  template <typename T>
  void setControllableValueByName(const MooseObjectName & object_name,
                                  const std::string & param_name,
                                  const T & value);

  template <typename T>
  void setControllableValueByName(const std::string & tag,
                                  const std::string & object_name,
                                  const std::string & param_name,
                                  const T & value);

  template <typename T>
  void setControllableValueByName(const MooseObjectParameterName & name, const T & value);
  ///@}

private:
  /// A reference to the InputParameterWarehouse which is used for access the parameter objects
  InputParameterWarehouse & _input_parameter_warehouse;
};

template <typename T>
T
Control::getControllableValue(const std::string & name, bool warn_when_values_differ)
{
  return getControllableValueByName<T>(getParam<std::string>(name), warn_when_values_differ);
}

template <typename T>
T
Control::getControllableValueByName(const std::string & name, bool warn_when_values_differ)
{
  MooseObjectParameterName desired(name);
  ControllableParameter helper = getControllableParameterByName(desired);
  return helper.get<T>(true, warn_when_values_differ)[0];
}

template <typename T>
T
Control::getControllableValueByName(const std::string & object_name,
                                    const std::string & param_name,
                                    bool warn_when_values_differ)
{
  MooseObjectParameterName desired(MooseObjectName(object_name), param_name);
  ControllableParameter helper = getControllableParameterByName(desired);
  return helper.get<T>(true, warn_when_values_differ)[0];
}

template <typename T>
T
Control::getControllableValueByName(const MooseObjectName & object_name,
                                    const std::string & param_name,
                                    bool warn_when_values_differ)
{
  MooseObjectParameterName desired(object_name, param_name);
  ControllableParameter helper = getControllableParameterByName(desired);
  return helper.get<T>(true, warn_when_values_differ)[0];
}

template <typename T>
T
Control::getControllableValueByName(const std::string & tag,
                                    const std::string & object_name,
                                    const std::string & param_name,
                                    bool warn_when_values_differ)
{
  MooseObjectParameterName desired(tag, object_name, param_name);
  ControllableParameter helper = getControllableParameterByName(desired);
  return helper.get<T>(true, warn_when_values_differ)[0];
}

template <typename T>
T
Control::getControllableValueByName(const MooseObjectParameterName & desired,
                                    bool warn_when_values_differ)
{
  ControllableParameter helper = getControllableParameterByName(desired);
  return helper.get<T>(true, warn_when_values_differ)[0];
}

template <typename T>
void
Control::setControllableValueByName(const MooseObjectParameterName & desired, const T & value)
{
  ControllableParameter helper = getControllableParameterByName(desired);
  helper.set<T>(value);
}

template <typename T>
void
Control::setControllableValue(const std::string & name, const T & value)
{
  setControllableValueByName<T>(getParam<std::string>(name), value);
}

template <typename T>
void
Control::setControllableValueByName(const std::string & name, const T & value)
{
  MooseObjectParameterName desired(name);
  ControllableParameter helper = getControllableParameterByName(desired);
  helper.set<T>(value);
}

template <typename T>
void
Control::setControllableValueByName(const std::string & object_name,
                                    const std::string & param_name,
                                    const T & value)
{
  MooseObjectParameterName desired(MooseObjectName(object_name), param_name);
  ControllableParameter helper = getControllableParameterByName(desired);
  helper.set<T>(value);
}

template <typename T>
void
Control::setControllableValueByName(const MooseObjectName & object_name,
                                    const std::string & param_name,
                                    const T & value)
{
  MooseObjectParameterName desired(object_name, param_name);
  ControllableParameter helper = getControllableParameterByName(desired);
  helper.set<T>(value);
}

template <typename T>
void
Control::setControllableValueByName(const std::string & tag,
                                    const std::string & object_name,
                                    const std::string & param_name,
                                    const T & value)
{
  MooseObjectParameterName desired(tag, object_name, param_name);
  ControllableParameter helper = getControllableParameterByName(desired);
  helper.set<T>(value);
}
