//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <functional>
#include <string>
#include <map>
#include <type_traits>
#include <typeindex>

#include "nlohmann/json.h"

#include "MooseError.h"
#include "MooseUtils.h"

class WebServerControl;

namespace Moose
{
/**
 * A static registry used to register and build values of different types for the WebServerControl
 *
 * Needed due to the complexities of parsing parameter types from generic JSON received
 * by the web server.
 */
class WebServerControlTypeRegistry
{
public:
  /**
   * @return The WebServerControlTypeRegistry singleton
   */
  static WebServerControlTypeRegistry & getRegistry();

  /**
   * The base class for a value that is produced by this registry.
   */
  class ControlledValueBase
  {
  public:
    /**
     * Constructor.
     * @param name The name that the value is for (typically a controllable path)
     * @param type The string representationg of the type
     */
    ControlledValueBase(const std::string & name, const std::string & type);

    virtual ~ControlledValueBase() {}

    /**
     * @return The name that the value is for
     */
    const std::string & name() const { return _name; }
    /**
     * @return The string representation of the type
     */
    const std::string & type() const { return _type; }

    /**
     * Sets the controllable value given the name and type via the controllable
     * interface in \p control.
     *
     * Will broadcast the value and set it in the derived, type-aware class.
     */
    virtual void setControllableValue(WebServerControl & control) = 0;

  private:
    /// The name that the value is for
    const std::string _name;
    /// The string representation of the type
    const std::string _type;
  };

  /**
   * Base registry class for a type that is used to build values.
   */
  class RegisteredTypeBase
  {
  public:
    RegisteredTypeBase(const std::string & type);

    virtual ~RegisteredTypeBase() {}

    /**
     * @return The string representation of the type
     */
    const std::string & type() const { return _type; }
    /**
     * Builds a value with the given type, name \p name, and JSON value \p json_value.
     *
     * This will be called on receipt of a controllable value on rank 0 in the
     * WebServerControl. It will at that point parse the value from JSON and store it.
     * Later on during sync, it will be used to broadcast the value and then locally
     * set it.
     */
    virtual std::unique_ptr<ControlledValueBase> build(const std::string & name,
                                                       const nlohmann::json & json_value) const = 0;
    /**
     * Builds a value with the given type, name \p name, and a default value.
     *
     * This will be called by the WebServerControl on ranks that are not rank 0.
     * It will be used during the sync step, where the value is broadcasted to the
     * rest of the ranks and then set locally.
     */
    virtual std::unique_ptr<ControlledValueBase> build(const std::string & name) const = 0;

  private:
    /// The string representation of the underlying type
    const std::string _type;
  };

  /**
   * Derived registry item.
   *
   * Stores how to build the value and how to parse the value from JSON.
   *
   * @tparam ControlledValue The derived ControlledValueBase class that contains
   * the implementation for setting the controllable value
   * @tparam ValueType The underlying type of the value to be controlled
   */
  template <class ControlledValue, class ValueType>
  struct RegisteredType : public RegisteredTypeBase
  {
    RegisteredType(const std::string & type,
                   std::function<ValueType(const nlohmann::json &)> && parse_function)
      : RegisteredTypeBase(type), _parse_function(parse_function)
    {
    }

    virtual std::unique_ptr<ControlledValueBase>
    build(const std::string & name) const override final
    {
      return std::make_unique<ControlledValue>(name, type());
    }
    virtual std::unique_ptr<ControlledValueBase>
    build(const std::string & name, const nlohmann::json & json_value) const override final
    {
      return std::make_unique<ControlledValue>(name, type(), _parse_function(json_value));
    }

  private:
    /// Function that converts from json -> the value for the ValueType
    const std::function<ValueType(const nlohmann::json &)> _parse_function;
  };

  /**
   * Register a type in the registry
   *
   * @tparam ControlledValue The derived ControlledValueBase class that contains
   * the implementation for setting the controllable value
   * @tparam ValueType The underlying type of the value to be controlled
   * @param type_name Human readable name for the type of the value to be controlled
   * @param parse_function Function used to parse the value from JSON
   */
  template <class ControlledValue, class ValueType>
  static char add(const std::string & type_name,
                  std::function<ValueType(const nlohmann::json &)> && parse_function);

  /**
   * Query the registration for the given type.
   */
  static const RegisteredTypeBase * query(const std::string & type);

  /**
   * Get the registration for the given type, erroring if it isn't registered.
   */
  static const RegisteredTypeBase & get(const std::string & type);

private:
  /// The registration data
  std::map<std::string, std::unique_ptr<RegisteredTypeBase>> _name_map;
  /// The registered value types, to avoid registering the same underlying
  /// value type multiple times
  std::set<std::type_index> _value_types;
};

template <class ControlledValue, class ValueType>
char
WebServerControlTypeRegistry::add(
    const std::string & type_name,
    std::function<ValueType(const nlohmann::json &)> && parse_function)
{
  static_assert(std::is_base_of_v<ControlledValueBase, ControlledValue>,
                "Is not derived from ControlledValueBase");
  static_assert(std::is_same_v<typename ControlledValue::value_type, ValueType>, "Is not the same");

  auto entry = std::make_unique<RegisteredType<ControlledValue, ValueType>>(
      type_name, std::move(parse_function));
  if (!getRegistry()._name_map.emplace(type_name, std::move(entry)).second)
    ::mooseError(
        "WebServerControlTypeRegistry: The string type \"", type_name, "\" is already registered.");

  static const std::type_index index = typeid(ValueType);
  if (!getRegistry()._value_types.insert(index).second)
    ::mooseError("WebServerControlRegistry: The type \"",
                 MooseUtils::prettyCppType<ValueType>(),
                 "\" is already registered");
  return 0;
}
}
