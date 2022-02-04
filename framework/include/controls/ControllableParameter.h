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
#include "InputParameters.h"
#include "MooseObjectName.h"
#include "MooseObjectParameterName.h"
#include "ControllableItem.h"

/**
 * The ControllableParameter class is simply a set of ControllableItem objects. This object
 * is what is used from within a Control for setting input parameter values. These objects are
 * made on demand by the Control objects.
 *
 * This class is needed to allow for multiple parameters to be set with a single interface.
 */

class ControllableParameter
{
public:
  ControllableParameter() = default;
  virtual ~ControllableParameter() = default;

  ControllableParameter(ControllableParameter &&) = default;
  ControllableParameter(const ControllableParameter &) = delete;
  ControllableParameter & operator=(const ControllableParameter &) = delete;
  ControllableParameter & operator=(ControllableParameter &&) = delete;

  /**
   * Return true if the container is empty.
   */
  bool empty() { return _items.size() == 0; }

  /**
   * Return a string that lists the parameters stored by this object.
   *
   * This is used by ControlInterface::getControlParamByName for error reporting.
   */
  std::string dump() const;

  /**
   * Set the value(s) of the controlled parameters stored in this class.
   * @param value The value to change the parameters to.
   */
  template <typename T>
  void set(const T & value, bool type_check = true);

  /**
   * Return a copy of the values of the given type.
   */
  template <typename T>
  std::vector<T> get(bool type_check = true, bool warn_when_values_difffer = false) const;

  /**
   * Check size() and the type of the stored items, i.e., there must be items with the given type.
   */
  template <typename T>
  bool check();

  /**
   * Check the execute flags.
   */
  void checkExecuteOnType(const ExecFlagType & current) const;

  /**
   * Adds the supplied item with the other items within this object.
   */
  void add(ControllableItem * item);

  /// Allows this to be used with std:: cout
  friend std::ostream & operator<<(std::ostream & stream, const ControllableParameter & obj);

private:
  /// Storage for the ControllableItems, these are stored as pointers to avoid copies.
  std::vector<ControllableItem *> _items;
};

template <typename T>
void
ControllableParameter::set(const T & value, bool type_check /*=true*/)
{
  for (ControllableItem * item : _items)
    item->set<T>(value, type_check);
}

template <typename T>
std::vector<T>
ControllableParameter::get(bool type_check /*=true*/, bool warn_when_values_differ) const
{
  std::vector<T> output;
  for (const ControllableItem * const item : _items)
  {
    std::vector<T> local = item->get<T>(type_check);
    output.insert(output.end(), local.begin(), local.end());
  }

  // Produce a warning, if the flag is true, when multiple parameters have different values
  if (warn_when_values_differ && _items.size() > 1)
  {
    // The first parameter to test against
    const T value0 = output[0];

    // Loop over all other parameter values
    for (T value : output)
    {
      if (value0 != value)
      {
        std::ostringstream oss;
        oss << "The following controlled parameters are being retrieved, but the values differ:\n";
        oss << dump();
        mooseWarning(oss.str());
      }
    }
  }

  return output;
}

template <typename T>
bool
ControllableParameter::check()
{
  bool type = std::all_of(
      _items.begin(), _items.end(), [](ControllableItem * item) { return item->check<T>(); });
  return type && !empty();
}
