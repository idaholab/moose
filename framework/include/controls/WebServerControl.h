//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Control.h"

#include "WebServerControlTypeRegistry.h"

#include "tinyhttp/http.h"

#include <atomic>
#include <memory>
#include <thread>

/**
 * Starts a webserver that an external process can connect to
 * in order to send JSON messages to control the solve
 */
class WebServerControl : public Control
{
public:
  static InputParameters validParams();

  WebServerControl(const InputParameters & parameters);
  ~WebServerControl();

  virtual void execute() override;

  /**
   * @return A string representation of \p json_type
   */
  static std::string stringifyJSONType(const miniJson::JsonType & json_type);

  /**
   * @return A c++ representation of the scalar value of \p json_value with
   * the given expected json type
   */
  template <typename T, miniJson::JsonType json_type>
  static T getScalarJSONValue(const miniJson::Json & json_value);

  using ValueBase = Moose::WebServerControlTypeRegistry::ValueBase;

  /**
   * Base class for a controllable value with a given type and name
   */
  template <typename T>
  class TypedValueBase : public ValueBase
  {
  public:
    TypedValueBase(const std::string & name, const std::string & type) : ValueBase(name, type) {}
    TypedValueBase(const std::string & name, const std::string & type, const T & value)
      : ValueBase(name, type), _value(value)
    {
    }

    /**
     * @return The underlying value
     */
    const T & value() const { return _value; }

    virtual void setControllableValue(WebServerControl & control) override final
    {
      control.comm().broadcast(_value);
      control.setControllableValueByName<T>(name(), value());
    }

  private:
    /// The underlying value
    T _value;
  };

  /**
   * Class that stores a scalar controllable value to be set
   */
  template <typename T, miniJson::JsonType json_type>
  class ScalarValue : public TypedValueBase<T>
  {
  public:
    ScalarValue(const std::string & name, const std::string & type) : TypedValueBase<T>(name, type)
    {
    }
    ScalarValue(const std::string & name,
                const std::string & type,
                const miniJson::Json & json_value)
      : TypedValueBase<T>(name, type, getScalarJSONValue<T, json_type>(json_value))
    {
    }
  };

  /**
   * Class that stores a vector controllable value to be set
   */
  template <typename T, miniJson::JsonType json_type>
  class VectorValue : public TypedValueBase<std::vector<T>>
  {
  public:
    VectorValue(const std::string & name, const std::string & type)
      : TypedValueBase<std::vector<T>>(name, type)
    {
    }
    VectorValue(const std::string & name,
                const std::string & type,
                const miniJson::Json & json_value)
      : TypedValueBase<std::vector<T>>(name, type, getVectorJSONValue(json_value))
    {
    }

    static std::vector<T> getVectorJSONValue(const miniJson::Json & json_value)
    {
      const auto from_json_type = json_value.getType();
      if (from_json_type != miniJson::JsonType::kArray)
        throw ValueBase::Exception("The value '" + json_value.serialize() + "' of type " +
                                   stringifyJSONType(from_json_type) + " is not an array");

      const auto & array_value = json_value.toArray();
      std::vector<T> value(array_value.size());
      for (const auto i : index_range(array_value))
        value[i] = getScalarJSONValue<T, json_type>(array_value[i]);
      return value;
    }
  };

  /**
   * Registers a scalar parameter type to be controlled
   */
  template <typename T, miniJson::JsonType json_type>
  static char registerScalarType(const std::string type_name)
  {
    return Moose::WebServerControlTypeRegistry().add<ScalarValue<T, json_type>>(type_name);
  }
  /**
   * Registers a vector parameter type to be controlled
   */
  template <typename T, miniJson::JsonType json_type>
  static char registerVectorType(const std::string type_name)
  {
    return Moose::WebServerControlTypeRegistry().add<VectorValue<T, json_type>>("std::vector<" +
                                                                                type_name + ">");
  }

private:
  /**
   * Internal method for starting the server
   */
  void startServer();

  /**
   * @return Whether or not the server is currently waiting
   */
  bool currentlyWaiting() const { return _currently_waiting.load(); }

  /// Whether or not the Control is currently waiting
  std::atomic<bool> _currently_waiting;

  /// The server
  std::unique_ptr<HttpServer> _server;
  /// The server thread
  std::unique_ptr<std::thread> _server_thread;

  /// The values received to control; filled on rank 0 from the server and then broadcast
  std::vector<std::unique_ptr<ValueBase>> _controlled_values;
  /// Mutex to prevent threaded writes to _controlled_values
  std::mutex _controlled_values_mutex;
};

template <typename T, miniJson::JsonType json_type>
T
WebServerControl::getScalarJSONValue(const miniJson::Json & json_value)
{
  const auto from_json_type = json_value.getType();
  if (from_json_type != json_type)
    throw ValueBase::Exception("The value " + json_value.serialize() + " of JSON type " +
                               stringifyJSONType(from_json_type) +
                               " is not of the expected JSON type " + stringifyJSONType(json_type));

  if constexpr (json_type == miniJson::JsonType::kBool)
    return json_value.toBool();
  else if constexpr (json_type == miniJson::JsonType::kNumber)
    return json_value.toDouble();
  else if constexpr (json_type == miniJson::JsonType::kString)
    return json_value.toString();
  ::mooseError("WebServerControl::getScalarJSONValue(): Not configured for parsing type ",
               stringifyJSONType(from_json_type));
}
