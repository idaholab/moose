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
#include <vector>

#include "nlohmann/json.h"

#include "WebServerControlTypeRegistry.h"
#include "WebServerControl.h"

#define registerWebServerControlCombine1(X, Y) X##Y
#define registerWebServerControlCombine(X, Y) registerWebServerControlCombine1(X, Y)
// Register a generic type
#define registerWebServerControlType(T, parse_function)                                            \
  static char registerWebServerControlCombine(wsc_type, __COUNTER__) =                             \
      Moose::WebServerControlTypeRegistration::registerType<T>(#T, parse_function)
// Register a scalar type
#define registerWebServerControlScalar(T)                                                          \
  static char registerWebServerControlCombine(wsc_scalar, __COUNTER__) =                           \
      Moose::WebServerControlTypeRegistration::registerScalarType<T>(#T)
// Register a vector type
#define registerWebServerControlVector(T)                                                          \
  static char registerWebServerControlCombine(wsc_vector, __COUNTER__) =                           \
      Moose::WebServerControlTypeRegistration::registerVectorType<T>(#T)

/**
 * Defines classes for registering values that can be parsed,
 * communicated, and stored in the WebServerControl.
 */
namespace Moose::WebServerControlTypeRegistration
{

/**
 * Register a generic type to be controlled by the WebServerControl.
 *
 * This should be used through the registerWebServerControlType macro.
 *
 * @param type_name Name of the type; this is the name that the client
 * will associate with the value when it is communicated
 * @param parse_function Function that parses the value from a JSON value
 */
template <class T>
char
registerType(const std::string type_name,
             std::function<T(const nlohmann::json &)> && parse_function)
{
  return Moose::WebServerControlTypeRegistry().add<WebServerControl::ControlledValue<T>, T>(
      type_name, std::move(parse_function));
}

/**
 * Register a scalar type to be controlled by the WebServerControl.
 *
 * This should be used through the registerWebServerControlScalar macro.
 *
 * Uses nlohmann::json to perform the type conversion.
 *
 * @param type_name Name of the type; this is the name that the client
 * will associate with the value when it is communicated
 */
template <class T>
char
registerScalarType(const std::string & type_name)
{
  const auto parse_function = [](const nlohmann::json & json_value) -> T
  { return json_value.get<T>(); };
  return registerType<T>(type_name, std::move(parse_function));
}

/**
 * Register a vector type to be controlled by the WebServerControl.
 *
 * This should be used through the registerWebServerControlVector macro.
 *
 * Uses nlohmann::json to perform the type conversion.
 *
 * @param type_name Name of the type in the vector; this is the name that
 * the client will associate with the value when it is communicated
 */
template <class T>
char
registerVectorType(const std::string & type_name)
{
  using vector_T = std::vector<T>;
  const auto parse_function = [](const nlohmann::json & json_value) -> vector_T
  { return json_value.get<vector_T>(); };
  return registerType<vector_T>("std::vector<" + type_name + ">", std::move(parse_function));
}

}
