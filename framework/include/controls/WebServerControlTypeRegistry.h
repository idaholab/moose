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

#include "MooseError.h"
#include "MooseUtils.h"

#include "nlohmann/json.h"

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
    ControlledValueBase(const std::string & name, const std::string & type)
      : _name(name), _type(type)
    {
    }

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
     * Will broadcast the value for setting it.
     */
    virtual void setControllableValue(WebServerControl & control) = 0;

  private:
    /// The name that the value is for
    const std::string _name;
    /// The string representation of the type
    const std::string _type;
  };

  /**
   * Registers a type with string name \p type_name and the given derived type.
   */
  template <class DerivedValueBase, class ValueType>
  static char add(const std::string & type_name,
                  std::function<ValueType(const nlohmann::json &)> && parse_function);

  /**
   * @return Whether or not the type \p type is registered.
   */
  static bool isRegistered(const std::string & type) { return getRegistry()._name_map.count(type); }

  /**
   * Builds a value with the type \p type, name \p name, and a default value.
   */
  static std::unique_ptr<ControlledValueBase> build(const std::string & type,
                                                    const std::string & name)
  {
    return get(type).build(name);
  }
  /**
   * Builds a value with the type \p type, name \p name, and a value parsed from \p json_value.
   */
  static std::unique_ptr<ControlledValueBase>
  build(const std::string & type, const std::string & name, const nlohmann::json & json_value)
  {
    return get(type).build(name, json_value);
  }

private:
  /**
   * Base registry class for a type that is used to build values.
   */
  class RegisteredTypeBase
  {
  public:
    RegisteredTypeBase(const std::string & type) : _type(type) {}
    virtual ~RegisteredTypeBase() {}

    /**
     * @return The string representation of the type
     */
    const std::string & type() const { return _type; }
    /**
     * Builds a value with the given type, name \p name, and JSON value \p json_value.
     *
     * This will parse the JSON value into the underlying type and will be called
     * on only rank 0 where server listens.
     */
    virtual std::unique_ptr<ControlledValueBase> build(const std::string & name,
                                                       const nlohmann::json & json_value) const = 0;
    /**
     * Builds a value with the given type, name \p name, and a default value.
     *
     * This will be called on processors that are not rank 0 for cloning.
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
   */
  template <class ControlledValue, class ValueType>
  struct RegisteredType : public RegisteredTypeBase
  {
    RegisteredType(const std::string & type,
                   std::function<ValueType(const nlohmann::json &)> && parse_function)
      : RegisteredTypeBase(type), _parse_function(std::move(parse_function))
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
    std::function<ValueType(const nlohmann::json &)> _parse_function;
  };

  /**
   * Internal getter for the registration object for type \p type.
   */
  static const RegisteredTypeBase & get(const std::string & type);

  /// The registration data
  std::map<std::string, std::unique_ptr<RegisteredTypeBase>> _name_map;
  /// The registered value types, to avoid registering the same underlying
  /// value type multiple times
  std::set<std::type_index> _value_types;
};

template <class DerivedValueBase, class ValueType>
char
WebServerControlTypeRegistry::add(
    const std::string & type_name,
    std::function<ValueType(const nlohmann::json &)> && parse_function)
{
  static_assert(std::is_base_of_v<ControlledValueBase, DerivedValueBase>,
                "Is not derived from ControlledValueBase");
  static_assert(std::is_same_v<typename DerivedValueBase::value_type, ValueType>,
                "Is not the same");

  auto entry = std::make_unique<RegisteredType<DerivedValueBase, ValueType>>(
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
