//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <string>
#include <map>
#include <type_traits>

#include "MooseError.h"

#include "minijson/minijson.h"

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
  class ValueBase
  {
  public:
    /**
     * Constructor.
     * @param name The name that the value is for (typically a controllable path)
     * @param type The string representationg of the type
     */
    ValueBase(const std::string & name, const std::string & type) : _name(name), _type(type) {}
    virtual ~ValueBase() {}

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

    /**
     * Common exception for parsing related errors in converting JSON to a value.
     */
    struct Exception : public std::exception
    {
    public:
      Exception(const std::string & message) : _message(message) {}
      virtual const char * what() const noexcept override final { return _message.c_str(); }

    private:
      const std::string _message;
    };

  private:
    /// The name that the value is for
    const std::string _name;
    /// The string representation of the type
    const std::string _type;
  };

  /**
   * Registers a type with string name \p type_name and the given derived type.
   */
  template <typename DerivedValueType>
  static char add(const std::string & type_name)
  {
    static_assert(std::is_base_of_v<ValueBase, DerivedValueType>, "Is not derived from ValueBase");
    getRegistry()._types.emplace(type_name, std::make_unique<Type<DerivedValueType>>(type_name));
    return 0;
  }

  /**
   * @return Whether or not the type \p type is registered.
   */
  static bool isRegistered(const std::string & type) { return getRegistry()._types.count(type); }

  /**
   * Builds a value with the type \p type, name \p name, and a default value.
   */
  static std::unique_ptr<ValueBase> build(const std::string & type, const std::string & name)
  {
    return get(type).build(name);
  }
  /**
   * Builds a value with the type \p type, name \p name, and a value parsed from \p json_value.
   *
   * Will throw ValueBase::Exception on a parsing error.
   */
  static std::unique_ptr<ValueBase>
  build(const std::string & type, const std::string & name, const miniJson::Json & json_value)
  {
    return get(type).build(name, json_value);
  }

private:
  /**
   * Base registry class for a type that is used to build values.
   */
  class TypeBase
  {
  public:
    TypeBase(const std::string & type) : _type(type) {}
    virtual ~TypeBase() {}

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
    virtual std::unique_ptr<ValueBase> build(const std::string & name,
                                             const miniJson::Json & json_value) const = 0;
    /**
     * Builds a value with the given type, name \p name, and a default value.
     *
     * This will be called on processors that are not rank 0 for cloning.
     */
    virtual std::unique_ptr<ValueBase> build(const std::string & name) const = 0;

  private:
    /// The string representation of the underlying type
    const std::string _type;
  };

  template <class DerivedValueType>
  struct Type : public TypeBase
  {
    Type(const std::string & type) : TypeBase(type) {}

    virtual std::unique_ptr<ValueBase> build(const std::string & name) const override final
    {
      return std::make_unique<DerivedValueType>(name, type());
    }
    virtual std::unique_ptr<ValueBase> build(const std::string & name,
                                             const miniJson::Json & json_value) const override final
    {
      return std::make_unique<DerivedValueType>(name, type(), json_value);
    }
  };

  /**
   * Internal getter for the registration object for type \p type.
   */
  static const TypeBase & get(const std::string & type)
  {
    auto & registry = getRegistry();
    const auto it = registry._types.find(type);
    if (it == registry._types.end())
      mooseError("WebServerControlTypeRegistry: The type '", type, "' is not registered");
    return *it->second;
  }

  /// The registration data
  std::map<std::string, std::unique_ptr<TypeBase>> _types;
};
}
