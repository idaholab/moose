//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Control.h"
#include "ControlData.h"
#include "THMProblem.h"

class THMControl : public Control
{
public:
  THMControl(const InputParameters & parameters);

  virtual void init() {}

  /**
   * Return the Controls that must run before this Control
   */
  const std::vector<std::string> & getControlDataDependencies() const
  {
    return _control_data_depends_on;
  }

protected:
  /**
   * Declare control data with name 'component:data_name'
   *
   * @param data_name A unique name for the control data
   */
  template <typename T>
  T & declareComponentControlData(const std::string & data_name);

  /**
   * Declare control data with name 'data_name'
   *
   * @param data_name A unique name for the control data
   */
  template <typename T>
  T & declareControlData(const std::string & data_name);

  /**
   * Get a reference to control data that are specified in the input parameter 'param_name'
   *
   * @param param_name The parameter name that contains the name of the control data
   */
  template <typename T>
  const T & getControlData(const std::string & param_name);

  /**
   * Get a reference to control data value from a previous time step that is specified in the input
   * parameter 'param_name'
   *
   * @param param_name The parameter name that contains the name of the control data
   */
  template <typename T>
  const T & getControlDataOld(const std::string & param_name);

  /**
   * Get a reference to control data that are specified by 'data_name' name
   *
   * @param data_name The name of the control data
   */
  template <typename T>
  const T & getControlDataByName(const std::string & data_name);

  /**
   * Get a reference to control data value from previous time step that is specified by 'data_name'
   * name
   *
   * @param data_name The name of the control data
   */
  template <typename T>
  const T & getControlDataOldByName(const std::string & data_name);

  /**
   * Get a reference to a component control data value from previous time step
   *
   * @param data_name The name of the control data
   */
  template <typename T>
  const T & getComponentControlDataOld(const std::string & data_name);

  THMProblem * _sim;

  /// A list of control data that are required to run before this control may run
  std::vector<std::string> _control_data_depends_on;

public:
  static InputParameters validParams();
};

template <typename T>
T &
THMControl::declareComponentControlData(const std::string & data_name)
{
  std::string full_name = name() + ":" + data_name;
  ControlData<T> * data_ptr = _sim->declareControlData<T>(full_name, this);
  return data_ptr->set();
}

template <typename T>
T &
THMControl::declareControlData(const std::string & data_name)
{
  ControlData<T> * data_ptr = _sim->declareControlData<T>(data_name, this);
  return data_ptr->set();
}

template <typename T>
const T &
THMControl::getControlData(const std::string & param_name)
{
  std::string data_name = getParam<std::string>(param_name);
  return getControlDataByName<T>(data_name);
}

template <typename T>
const T &
THMControl::getControlDataOld(const std::string & param_name)
{
  std::string data_name = getParam<std::string>(param_name);
  return getControlDataOldByName<T>(data_name);
}

template <typename T>
const T &
THMControl::getControlDataByName(const std::string & data_name)
{
  ControlData<T> * data_ptr = _sim->getControlData<T>(data_name);
  if (data_ptr == nullptr)
    mooseError("Trying to get control data '",
               data_name,
               "', but it does not exist in the system. Check your spelling.");

  // set up dependencies for this control object
  auto it = std::find(_control_data_depends_on.begin(), _control_data_depends_on.end(), data_name);
  if (it == _control_data_depends_on.end())
    _control_data_depends_on.push_back(data_name);

  return data_ptr->get();
}

template <typename T>
const T &
THMControl::getControlDataOldByName(const std::string & data_name)
{
  ControlData<T> * data_ptr = _sim->getControlData<T>(data_name);
  if (data_ptr == nullptr)
    mooseError("Trying to get control data '",
               data_name,
               "', but it does not exist in the system. Check your spelling.");

  return data_ptr->getOld();
}

template <typename T>
const T &
THMControl::getComponentControlDataOld(const std::string & data_name)
{
  std::string full_name = name() + ":" + data_name;
  return getControlDataOldByName<T>(full_name);
}
