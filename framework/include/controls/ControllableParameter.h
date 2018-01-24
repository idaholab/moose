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

#ifndef CONTROLLABLEPARAMETER_H
#define CONTROLLABLEPARAMETER_H

// MOOSE includes
#include "InputParameters.h"
#include "MooseObjectName.h"
#include "MooseObjectParameterName.h"

/**
 * A class for accessing controllable InputParameters.
 *
 * This class is used within Control objects for returning sets
 * of InputParaemeters.
 */
template <typename T>
class ControllableParameter
{
public:
  /**
   * Constructor.
   *
   * The construct does nothing, items must be added via
   * the insert method.
   *
   * @see ControlInterface
   */
  ControllableParameter();

  /**
   * The number of parameters contained in the class.
   */
  std::size_t size() const { return _parameter_values.size(); }

  /**
   * Set the value(s) of the controlled parameters stored in this class.
   * @param value The value to change the parameters to.
   */
  void set(const T & value);

  /**
   * Retrieve a copy of the parameter values controlled by this class.
   */
  const std::vector<T *> & get() const;

  /**
   * Return a string that lists the parameters stored by this object.
   *
   * This is used by ControlInterface::getControlParamByName for error
   * reporting.
   */
  std::string dump();

  /**
   * Adds a parameter to the container.
   *
   * @see ControlInterface
   */
  void insert(const MooseObjectParameterName & name, std::shared_ptr<InputParameters> param_object);

private:
  /// Vector of pointers to the parameters stored in this object for control.
  std::vector<T *> _parameter_values;

  /// Map of the possible names associated with the parameters stored for control.
  std::multimap<T *, MooseObjectParameterName> _parameter_names;
};

template <typename T>
ControllableParameter<T>::ControllableParameter()
{
}

template <typename T>
void
ControllableParameter<T>::insert(const MooseObjectParameterName & name,
                                 std::shared_ptr<InputParameters> param_object)
{
  // Get a pointer to the Parameter
  T * param_ptr = &param_object->template set<T>(name.parameter());

  // Search for the pointer in the existing multimap
  typename std::multimap<T *, MooseObjectParameterName>::iterator iter =
      _parameter_names.find(param_ptr);

  // If the pointer does not exist, add it
  if (iter == _parameter_names.end())
    _parameter_values.push_back(param_ptr);

  // Update the object names
  _parameter_names.insert(std::pair<T *, MooseObjectParameterName>(param_ptr, name));
}

template <typename T>
void
ControllableParameter<T>::set(const T & value)
{
  typename std::vector<T *>::iterator iter;
  for (unsigned int i = 0; i < _parameter_values.size(); i++)
    *_parameter_values[i] = value;
}

template <typename T>
const std::vector<T *> &
ControllableParameter<T>::get() const
{
  return _parameter_values;
}

template <typename T>
std::string
ControllableParameter<T>::dump()
{
  // Count of objects, for printing a number with the parameter
  unsigned int index = 0;

  // The output stream
  std::ostringstream oss;

  // Loop through each pointer
  typename std::vector<T *>::iterator iter;
  for (iter = _parameter_values.begin(); iter != _parameter_values.end(); iter++)
  {
    // Get the list of names associated with the parameter
    std::pair<typename std::multimap<T *, MooseObjectParameterName>::const_iterator,
              typename std::multimap<T *, MooseObjectParameterName>::const_iterator>
        range = _parameter_names.equal_range(*iter);
    typename std::multimap<T *, MooseObjectParameterName>::const_iterator jt;

    // Print each parameter as a list of possible names
    oss << "(" << index << ") ";
    for (jt = range.first; jt != range.second; ++jt)
      oss << jt->second << " ";
    oss << "\n";
    index++;
  }

  return oss.str();
}

#endif // CONTROLLABLEPARAMETER_H
