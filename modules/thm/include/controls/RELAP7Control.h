#ifndef RELAP7CONTROL_H
#define RELAP7CONTROL_H

#include "Control.h"
#include "ControlData.h"
#include "Restartable.h"
#include "Simulation.h"

class RELAP7Control;

template <>
InputParameters validParams<RELAP7Control>();

class RELAP7Control : public Control, public Restartable
{
public:
  RELAP7Control(const InputParameters & parameters);

protected:
  /**
   * Declare control data with name 'data_name'
   *
   * @param data_name An unique name for the control data
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

  Simulation & _sim;
};

template <typename T>
T &
RELAP7Control::declareControlData(const std::string & data_name)
{
  std::string full_name = "RELAP7/Controls/" + name() + ":" + data_name;
  ControlData<T> * data_ptr = _sim.getControlData<T>(full_name);
  return data_ptr->set();
}

template <typename T>
const T &
RELAP7Control::getControlData(const std::string & param_name)
{
  std::string data_name = getParam<std::string>(param_name);
  std::string full_name = "RELAP7/Controls/" + data_name;
  ControlData<T> * data_ptr = _sim.getControlData<T>(full_name);
  if (data_ptr == nullptr)
    mooseError("Trying to get control data '",
               data_name,
               "', but it does not exist in the system. Check your spelling.");
  return data_ptr->get();
}

#endif // RELAP7CONTROL_H
