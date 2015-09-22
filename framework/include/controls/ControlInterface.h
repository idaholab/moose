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
   * Obtain a ControllableParameter class for accessing parameter values given input file syntax.
   */
  template<typename T>
  ControllableParameter<T> getControlParam(const std::string & name);
  template<typename T>
  ControllableParameter<T> getControlParam(const std::string & name, unsigned int count);
  ///@}

  ///@{
  /**
   * Obtain a ControllableParameter class for accessing parameter values given the actual name.
   */
  template<typename T>
  ControllableParameter<T> getControlParamByName(const std::string & name);
  template<typename T>
  ControllableParameter<T> getControlParamByName(const std::string & name, unsigned int count);
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

  /// The object thread id, needed for access the warehouse
  THREAD_ID _tid;

  /// The name of the object, for better error message
  const std::string & _ci_name;

  // Only these objects are allowed to construct this interface
  friend class RealControlParameterReporter;
  friend class Control;
};

template<typename T>
ControllableParameter<T>
ControlInterface::getControlParam(const std::string & name)
{
  return getControlParamByName<T>(_ci_parameters.get<std::string>(name),
                                  std::numeric_limits<unsigned int>::max());
}

template<typename T>
ControllableParameter<T>
ControlInterface::getControlParam(const std::string & name, unsigned int count)
{
  return getControlParamByName<T>(_ci_parameters.get<std::string>(name), count);
}

template<typename T>
ControllableParameter<T>
ControlInterface::getControlParamByName(const std::string & name)
{
  return getControlParamByName<T>(_ci_parameters.get<std::string>(name),
                                  std::numeric_limits<unsigned int>::max());
}

template<typename T>
ControllableParameter<T>
ControlInterface::getControlParamByName(const std::string & name, unsigned int count)
{

  // The desired parameter name
  MooseObjectParameterName desired(name);

  // The ControlllableParametr object to return
  ControllableParameter<T> output;

  // Loop over all InputParameter objects
  for (InputParameterWarehouse::InputParameterIterator it = _input_parameter_warehouse.begin(_tid); it != _input_parameter_warehouse.end(_tid); ++it)
  {
    // If the desired object name does not match the current object name, move on
    if (desired != it->first)
      continue;

    // If the parameter is valid and controllable update the output vector with a pointer to the parameter
    if (it->second->have_parameter<T>(desired.parameter()))
    {
      // Do not allow non-controllable types to be controlled
      if (!it->second->isControllable(desired.parameter()))
        mooseError("The controlled parameter, '" << name << "', in " << _ci_name << " is not a controllable parameter.");
      // Store pointer to the writable parameter
      output.insert(MooseObjectParameterName(it->first, desired.parameter()), &(it->second->set<T>(desired.parameter())));
    }
  }

  // Error if nothing was found
  if (output.size() == 0)
    mooseError("The controlled parameter, '" << name << "', in " << _ci_name << " was not found.");

  // Error if the size limit was reached
  if (count != std::numeric_limits<unsigned int>::max() && output.size() != count)
  {
    std::ostringstream oss;
    oss << "The controlled parameter, '" << name << "', in " << _ci_name << " was found,\n";
    oss << "but the number of parameters matching (" << output.size() << ") the name differs\n";
    oss << "from the amount requested (" << count << ").\n\n";
    oss << "The following parameters were found:\n";
    oss << output.dump();
    mooseError(oss.str());
  }

  return output;
}

#endif // CONTROLINTERFACE_H
