#pragma once

#include "Control.h"
#include "ControlData.h"
#include "Simulation.h"

class THMControl;

template <>
InputParameters validParams<THMControl>();

class THMControl : public Control
{
public:
  THMControl(const InputParameters & parameters);

  virtual void init() {}

  /**
   * Return the Controls that must run before this Control
   */
  std::vector<std::string> & getControlDataDependencies() { return _control_data_depends_on; }

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
   * Get a reference to control data that are specified by 'data_name' name
   *
   * @param data_name The name of the control data
   */
  template <typename T>
  const T & getControlDataByName(const std::string & data_name);

  Simulation & _sim;

  /// A list of control data that are required to run before this control may run
  std::vector<std::string> _control_data_depends_on;
};

template <typename T>
T &
THMControl::declareComponentControlData(const std::string & data_name)
{
  std::string full_name = name() + ":" + data_name;
  ControlData<T> * data_ptr = _sim.declareControlData<T>(full_name, this);
  return data_ptr->set();
}

template <typename T>
T &
THMControl::declareControlData(const std::string & data_name)
{
  ControlData<T> * data_ptr = _sim.declareControlData<T>(data_name, this);
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
THMControl::getControlDataByName(const std::string & data_name)
{
  ControlData<T> * data_ptr = _sim.getControlData<T>(data_name);
  if (data_ptr == nullptr)
    mooseError("Trying to get control data '",
               data_name,
               "', but it does not exist in the system. Check your spelling.");

  // set up dependencies for this control object
  auto & deps = getControlDataDependencies();
  auto it = std::find(deps.begin(), deps.end(), data_name);
  if (it == deps.end())
    deps.push_back(data_name);

  return data_ptr->get();
}
