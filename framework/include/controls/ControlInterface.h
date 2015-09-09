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

// Forward declarations
class ControlInterface;

template<>
InputParameters validParams<ControlInterface>();


/**
 * A storage container for the name of parameters to be controlled
 */
struct ControlParameterName
{
  /// The "system" name (e.g., Kerenels)
  std::string system;

  /// The "object" name (generally a sub block in an input file)
  std::string object;

  /// The name of the parameter to be controlled
  std::string param;
};


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

  /**
   * Obtain a writeable reference to a single parameter given a InputParameter name
   */
  template<typename T>
  T & getControllableParam(const std::string & name);

  /**
   * Obtain a writeable reference to a single parameter given a specific name
   */
  template<typename T>
  T & getControllableParamByName(const std::string & name);

  /**
   * Obtain pointers to all parameters given a InputParameter name
   */
  template<typename T>
  std::vector<T*> getControllableParamVector(const std::string & name);

  /**
   * Obtain pointers to all parameters given a specific name
   */
  template<typename T>
  std::vector<T*> getControllableParamVectorByName(const std::string & name);

  /**
   * Destructor
   */
  virtual ~ControlInterface(){}

private:

  /**
   * A private constructor to limit the use of this interface to Control related objects
   */
  ControlInterface(const InputParameters & parameters);

  /**
   * Helper method for extracting iterators to InputParameter objects containing a property with the given name
   * @param name The complete name of the desired parameter (only used for error messages)
   * @param desired The desired parameter separated into components via tokenizeName method of this class
   *
   * Using the iterator allows for the public get methods to provide better error messages because the
   * full object name can be reported.
   *
   * @see tokenizeName
   */
  template<typename T>
  std::vector<InputParameterIterator> getValidInputParameterIteratorsByName(const std::string & name, const ControlParameterName & desired);

  /**
   * Helper method separating parameter name into system/group/param format
   *
   * [System]
   *   [./object]
   *     param = ...
   *
   */
  ControlParameterName tokenizeName(const std::string & name);

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
  friend class RealParameterReporter;
  friend class Control;
};

template<typename T>
T &
ControlInterface::getControllableParam(const std::string & name)
{
  return getControllableParamByName<T>(_ci_parameters.get<std::string>(name));
}

template<typename T>
T &
ControlInterface::getControllableParamByName(const std::string & name)
{
  // Get the InputParameter objects that contain objects
  ControlParameterName desired = tokenizeName(name);
  std::vector<InputParameterIterator> iters = getValidInputParameterIteratorsByName<T>(name, desired);

  // If more than one parameter is located, report an error message listing the object and parameter names
  if (iters.size() > 1)
  {
    std::ostringstream oss;
    oss << "The controlled parameter, '" << name << "', in " << _ci_name << " was found in multiple objects:\n";
    for (std::vector<InputParameterIterator>::const_iterator it = iters.begin(); it != iters.end(); ++it)
      oss << "  " << (*it)->first << '\n';
    oss << "\nUse the method 'getControlParamVector' to access multiple parameters";
    mooseError(oss.str());
  }

  // Return a writeable reference to the parameter
  return iters[0]->second->set<T>(desired.param);
}

template<typename T>
std::vector<T*>
ControlInterface::getControllableParamVector(const std::string & name)
{
  return getControllableParamVectorByName<T>(_ci_parameters.get<std::string>(name));
}

template<typename T>
std::vector<T*>
ControlInterface::getControllableParamVectorByName(const std::string & name)
{
  // Get the InputParameter objects that contain objects
  ControlParameterName desired = tokenizeName(name);
  std::vector<InputParameterIterator> iters = getValidInputParameterIteratorsByName<T>(name, desired);

  // The vector the return
  std::vector<T*> output;

  // Loop over all InputParameter objects
  for (std::vector<InputParameterIterator>::iterator it = iters.begin(); it != iters.end(); ++it)
    output.push_back(&((*it)->second->set<T>(desired.param)));

  return output;
}

template<typename T>
std::vector<InputParameterIterator>
ControlInterface::getValidInputParameterIteratorsByName(const std::string & name, const ControlParameterName & desired)
{
  // The vector to return
  std::vector<InputParameterIterator> output;

  // Storage for tokenizing the current InputParameters name
  std::vector<std::string> current;

  // Loop over all InputParameter objects
  for (InputParameterIterator it = _input_parameter_warehouse.begin(_tid); it != _input_parameter_warehouse.end(_tid); ++it)
  {
    InputParameters & parameters = *(it->second);

    // Get the current system and group:
    //   current[0] = system; current[1] = group;
    current.clear();
    MooseUtils::tokenize(it->first, current);

    // Continue if the system and group do not agree
    if ( (!desired.system.empty() && current.size() > 0 && desired.system != current[0]) ||
         (!desired.object.empty() && current.size() > 1 && desired.object != current[1]) )
      continue;

    // If the parameter is valid update the output vector with a pointer to the parameter
    if (parameters.isParamValid(desired.param))
    {
      // Do not allow non-controllable types to be controlled
      if (!parameters.isControllable(desired.param))
        mooseError("The controlled parameter, '" << name << "', in " << _ci_name << " is not a controllable parameter.");

      output.push_back(it);
    }
  }

  // Error if nothing was found
  if (output.size() == 0)
    mooseError("The controlled parameter, '" << name << "', in " << _ci_name << " was not found.");

  return output;
}

#endif // CONTROLINTERFACE_H
