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

#ifndef CONTROLINTERFACE_H
#define CONTROLINTERFACE_H

// MOOSE includes
#include "InputParameters.h"
#include "InputParameterWarehouse.h"
#include "MooseUtils.h"
#include "ControllableParameter.h"

// Forward declarations
class ControlInterface;

template<>
InputParameters validParams<ControlInterface>();

/**
 * An interface class for accessing writable references to InputParameters
 *
 * This class exists to provided parameter access to the Control base class
 * as well as the RealParameterReporter postprocessor, no other objects
 * should use this class. Hence, the private constructor
 */
class ControlInterface
{
protected:

  ///@{
  /**
   * Obtain the value of a controllable parameter given input file syntax or actual name.
   * @param name The name of the parameter to retrieve a value.
   * @param warn_when_values_diff When true, produce a warning if multiple controllable values share the given
   *                              name but have varying values.
   * @return A constant reference to the first parameter that matches the given name.
   */
  template<typename T>
  const T & getControllableValue(const std::string & name, bool warn_when_values_differ = true);

  template<typename T>
  const T & getControllableValueByName(const std::string & name, bool warn_when_values_differ = true);

  template<typename T>
  const T & getControllableValueByName(const std::string & tag, const std::string & param_name, bool warn_when_values_differ = true);
  ///@}

  ///@{
  /**
   * Set the value(s) of a controllable parameter of class given input file syntax or actual name.
   * @param name The name of the parameter to retrieve a value.
   * @param value The value to set all matching parameters to.
   * @param warn_when_values_diff When true, produce a warning if multiple controllable values share the given
   *                              name but have varying values.
   */
  template<typename T>
  void setControllableValue(const std::string & name, const T & value, bool warn_when_values_differ = false);

  template<typename T>
  void setControllableValueByName(const std::string & name, const T & value, bool warn_when_values_differ = false);

  template<typename T>
  void setControllableValueByName(const std::string & tag, const std::string & param_name, const T & value, bool warn_when_values_differ = false);

  ///@}

  /**
   * Destructor
   */
  virtual ~ControlInterface(){}

private:

  /**
   * A private constructor to limit the use of this interface to Control related objects
   */
  ControlInterface(const InputParameters & parameters);

  /// Reference to the Control objects parameters
  const InputParameters & _ci_parameters;

  /// A Reference to MooseApp (needed to gain access to InputParameterWarehouse)
  MooseApp & _ci_app;

  /// A reference to the InputParameterWarehouse which is used for access the parameter objects
  InputParameterWarehouse & _input_parameter_warehouse;

  /// Helper method for retrieving controllable parameters
  template<typename T>
  ControllableParameter<T> getControllableParameterHelper(const MooseObjectParameterName & desired, bool warn_when_values_differ);

  /// The name of the object, for better error message
  const std::string & _ci_name;

  // Only these objects are allowed to construct this interface
  friend class RealControlParameterReporter;
  friend class Control;
};

template<typename T>
const T &
ControlInterface::getControllableValue(const std::string & name, bool warn_when_values_differ)
{
  return getControllableValueByName<T>(_ci_parameters.get<std::string>(name), warn_when_values_differ);
}

template<typename T>
const T &
ControlInterface::getControllableValueByName(const std::string & name, bool warn_when_values_differ)
{
  MooseObjectParameterName desired(name);
  ControllableParameter<T> helper = getControllableParameterHelper<T>(desired, warn_when_values_differ);
  return  *(helper.get()[0]);
}

template<typename T>
const T &
ControlInterface::getControllableValueByName(const std::string & tag, const std::string & param_name, bool warn_when_values_differ)
{
  MooseObjectParameterName desired(tag, param_name);
  ControllableParameter<T> helper = getControllableParameterHelper<T>(desired, warn_when_values_differ);
  return  *(helper.get()[0]);
}

template<typename T>
void
ControlInterface::setControllableValue(const std::string & name, const T & value, bool warn_when_values_differ)
{
  setControllableValueByName<T>(_ci_parameters.get<std::string>(name), value, warn_when_values_differ);
}

template<typename T>
void
ControlInterface::setControllableValueByName(const std::string & name, const T & value, bool warn_when_values_differ)
{
  MooseObjectParameterName desired(name);
  ControllableParameter<T> helper = getControllableParameterHelper<T>(desired, warn_when_values_differ);
  helper.set(value);
}

template<typename T>
void
ControlInterface::setControllableValueByName(const std::string & tag, const std::string & param_name, const T & value, bool warn_when_values_differ)
{
  MooseObjectParameterName desired(tag, param_name);
  ControllableParameter<T> helper = getControllableParameterHelper<T>(desired, warn_when_values_differ);
  helper.set(value);
}

template<typename T>
ControllableParameter<T>
ControlInterface::getControllableParameterHelper(const MooseObjectParameterName & desired, bool warn_when_values_differ)
{
  // The ControllableParameter object to return
  ControllableParameter<T> output;

  // Loop over all threads
  for (THREAD_ID tid = 0; tid < libMesh::n_threads(); ++tid)
  {
    // Loop over all InputParameter objects
    for (InputParameterWarehouse::InputParameterIterator it = _input_parameter_warehouse.begin(tid); it != _input_parameter_warehouse.end(tid); ++it)
    {
      // If the desired object name does not match the current object name, move on
      if (desired != it->first)
        continue;

      // If the parameter is valid and controllable update the output vector with a pointer to the parameter
      if (it->second->have_parameter<T>(desired.parameter()))
      {
        // Do not allow non-controllable types to be controlled
        if (!it->second->isControllable(desired.parameter()))
          mooseError("The desired parameter is not controllable: " << desired);

        // Store pointer to the writable parameter
        output.insert(MooseObjectParameterName(it->first, desired.parameter()), &(it->second->set<T>(desired.parameter())));
      }
    }
  }

  // Error if nothing was found
  if (output.size() == 0)
    mooseError("The controlled parameter was not found: " << desired);

  // Produce a warning, if the flag is true, when multiple parameters have differing values
  if (warn_when_values_differ)
  {
    // Inspect the values for differing values
    const std::vector<T*> & values = output.get();

    // The first parameter to test against
    const T & value0 = *values[0];

    // Loop over all other parameter values
    for (typename std::vector<T*>::const_iterator it = values.begin() + 1; it != values.end(); ++it)
    {
      if (value0 != **it)
      {
        std::ostringstream oss;
        oss << "The controlled parameter with tag, '" << desired.tag() << "', and name, '" << desired.name() << "', in " << _ci_name << " was found,\n";
        oss << "but the number of parameters have differing values.\n\n";
        oss << output.dump();
        mooseWarning(oss.str());
      }
    }
  }

  return output;
}

#endif // CONTROLINTERFACE_H
