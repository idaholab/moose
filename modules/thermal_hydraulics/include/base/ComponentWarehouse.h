//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseError.h"

class Component;
class InputParameters;
class Factory;

/**
 * Warehouse for storing components
 */
class ComponentWarehouse
{
public:
  /**
   * Constructor
   *
   * @param factory   The factory
   */
  ComponentWarehouse(Factory & factory);

  /**
   * Returns the list of components
   */
  const std::vector<std::shared_ptr<Component>> & getComponents() const { return _components; }

  /**
   * Returns true if a component exists with the given name
   *
   * @param[in] name   The name of the component
   */
  bool hasComponent(const std::string & name) const;

  /**
   * Adds a component into the simulation
   *
   * @param[in] type   Type (the registered class name) of the component
   * @param[in] name   Name of the component
   * @param[in] params   Input parameters
   */
  void addComponent(const std::string & type, const std::string & name, InputParameters & params);

  /**
   * Returns true if a component exists with the given name and type
   *
   * @tparam T   Type of the component we are requesting
   * @param[in] name   The name of the component
   */
  template <typename T>
  bool hasComponentOfType(const std::string & name) const;

  /**
   * Gets a component by its name
   *
   * @tparam T   Type of the component we are requesting
   * @param[in] name   The name of the component
   * @return Pointer to the component if found (otherwise throws error)
   */
  template <typename T>
  const T & getComponentByName(const std::string & name) const;

  /**
   * Sorts the components using the dependency resolver
   */
  void sortComponents();

private:
  /// The factory
  Factory & _factory;
  /// List of components in this simulation
  std::vector<std::shared_ptr<Component>> _components;
  /// Map of components by their names
  std::map<std::string, std::shared_ptr<Component>> _comp_by_name;
};

template <typename T>
bool
ComponentWarehouse::hasComponentOfType(const std::string & name) const
{
  auto it = _comp_by_name.find(name);
  if (it != _comp_by_name.end())
    return dynamic_cast<T *>((it->second).get()) != nullptr;
  else
    return false;
}

template <typename T>
const T &
ComponentWarehouse::getComponentByName(const std::string & name) const
{
  auto it = _comp_by_name.find(name);
  if (it != _comp_by_name.end())
    return *dynamic_cast<T *>((it->second).get());
  else
    mooseError("Component '",
               name,
               "' does not exist in the simulation. Use hasComponent() or "
               "checkComponentByName() before calling getComponent().");
}
