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

// A storage structure for the name
struct ControlParameterNameContainer
{
  std::string system;
  std::string object;
  std::string param;
  std::string syntax;
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
  T & getControlParam(const std::string & name);

  /**
   * Obtain a writeable reference to a single parameter given a specific name
   */
  template<typename T>
  T & getControlParamByName(const std::string & name);

  /**
   * Obtain pointers to all parameters given a InputParameter name
   */
  template<typename T>
  std::vector<T*> getControlParamVector(const std::string & name);

  /**
   * Obtain pointers to all parameters given a specific name
   */
  template<typename T>
  std::vector<T*> getControlParamVectorByName(const std::string & name);

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
  std::vector<InputParameterIterator> getValidInputParameterIteratorsByName(const std::string & name, const ControlParameterNameContainer & desired);

  /**
   * Helper method separating parameter name into desired format
   */
  static ControlParameterNameContainer tokenizeName(std::string name);

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
ControlInterface::getControlParam(const std::string & name)
{
  return getControlParamByName<T>(_ci_parameters.get<std::string>(name));
}

template<typename T>
T &
ControlInterface::getControlParamByName(const std::string & name)
{
  // Get the InputParameter objects that contain objects
  ControlParameterNameContainer desired = tokenizeName(name);
  std::vector<InputParameterIterator> iters = getValidInputParameterIteratorsByName<T>(name, desired);

  // If more than one parameter is located, report an eror message listing the object and parameter names
  if (iters.size() > 1)
  {
    std::ostringstream oss;
    oss << "The controlled parameter, '" << name << "', in " << _ci_name << " was found in multiple objects:\n";
    for (std::vector<InputParameterIterator>::const_iterator it = iters.begin(); it != iters.end(); ++it)
      oss << "  " << (*it)->system << "::" << (*it)->object << '\n';
    oss << "\nUse the method 'getControlParamVector' to access multiple parameters";
    mooseError(oss.str());
  }

  // Return a writeable reference to the parameter
  return iters[0]->parameters->set<T>(desired.param);
}

template<typename T>
std::vector<T*>
ControlInterface::getControlParamVector(const std::string & name)
{
  return getControlParamVectorByName<T>(_ci_parameters.get<std::string>(name));
}

template<typename T>
std::vector<T*>
ControlInterface::getControlParamVectorByName(const std::string & name)
{
  // Get the InputParameter objects that contain objects
  ControlParameterNameContainer desired = tokenizeName(name);
  std::vector<InputParameterIterator> iters = getValidInputParameterIteratorsByName<T>(name, desired);

  // The vector the return
  std::vector<T*> output;

  // Loop over all InputParameter objects
  for (std::vector<InputParameterIterator>::iterator it = iters.begin(); it != iters.end(); ++it)
    output.push_back(&((*it)->parameters->set<T>(desired.param)));

  return output;
}

template<typename T>
std::vector<InputParameterIterator>
ControlInterface::getValidInputParameterIteratorsByName(const std::string & name, const ControlParameterNameContainer & desired)
{
  // The vector to return
  std::vector<InputParameterIterator> output;

  // Loop over all InputParameter objects
  for (InputParameterIterator it = _input_parameter_warehouse.begin(_tid); it != _input_parameter_warehouse.end(_tid); ++it)
  {
    // Continue if the system, syntax, and object names do not agree
    if ( (!desired.system.empty() && !it->system.empty() && desired.system != it->system)
      || (!desired.syntax.empty() && !it->syntax.empty() && desired.syntax != it->syntax)
      || (!desired.object.empty() && !it->object.empty() && desired.object != it->object))
      continue;

    // If the parameter is valid and controllable update the output vector with a pointer to the parameter
    if (it->parameters->isParamValid(desired.param))
    {
      // Do not allow non-controllable types to be controlled
      if (!it->parameters->isControllable(desired.param))
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
